/*
 * Unit tests for pure-logic helpers in App/frequencies.c:
 *   - FREQUENCY_GetBand
 *   - FREQUENCY_CalculateOutputPower
 *   - FREQUENCY_RoundToStep
 *   - FREQUENCY_GetStepIdxFromSortedIdx / FREQUENCY_GetSortedIdxFromStepIdx
 *   - RX_freq_check
 */

#include "test_framework.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* ------------------------------------------------------------------ *
 * Provide definitions that frequencies.c normally gets from headers   *
 * ------------------------------------------------------------------ */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

/* We enable ENABLE_WIDE_RX so we get the extended ranges (matches Fusion preset) */
#ifndef ENABLE_WIDE_RX
#define ENABLE_WIDE_RX
#endif

/* Re-create the frequency band enum and step enum from frequencies.h */
typedef enum {
    BAND_NONE = -1,
    BAND1_50MHz = 0,
    BAND2_108MHz,
    BAND3_137MHz,
    BAND4_174MHz,
    BAND5_350MHz,
    BAND6_400MHz,
    BAND7_470MHz,
    BAND_N_ELEM
} FREQUENCY_Band_t;

typedef enum {
    STEP_2_5kHz,
    STEP_5kHz,
    STEP_6_25kHz,
    STEP_10kHz,
    STEP_12_5kHz,
    STEP_25kHz,
    STEP_8_33kHz,
    STEP_0_01kHz,
    STEP_0_05kHz,
    STEP_0_1kHz,
    STEP_0_25kHz,
    STEP_0_5kHz,
    STEP_1kHz,
    STEP_1_25kHz,
    STEP_9kHz,
    STEP_15kHz,
    STEP_20kHz,
    STEP_30kHz,
    STEP_50kHz,
    STEP_100kHz,
    STEP_125kHz,
    STEP_200kHz,
    STEP_250kHz,
    STEP_500kHz,
    STEP_N_ELEM
} STEP_Setting_t;

typedef struct {
    const uint32_t lower;
    const uint32_t upper;
} freq_band_table_t;

/* Guard out the includes that frequencies.c normally pulls */
#define FREQUENCIES_H
#define MISC_H
#define SETTINGS_H

/* F_LOCK enum values from settings.h (TX_freq_check uses these) */
enum TxLockModes_t {
    F_LOCK_DEF,
    F_LOCK_FCC,
    F_LOCK_CE,
    F_LOCK_GB,
    F_LOCK_430,
    F_LOCK_438,
    F_LOCK_ALL,
    F_LOCK_NONE,
    F_LOCK_LEN
};

/* Stubs for globals that TX_freq_check reads */
uint8_t gSetting_F_LOCK  = 0; /* F_LOCK_DEF */
bool    gSetting_350EN   = true;
bool    gSetting_200TX   = true;
bool    gSetting_350TX   = true;
bool    gSetting_500TX   = true;

/* Forward-declare RX_freq_check so TX_freq_check compiles
   (RX_freq_check is defined later in the same file). */
int32_t RX_freq_check(const uint32_t Frequency);

/* Pull in the implementation */
#include "../App/frequencies.c"

/* ================================================================== */
/*                        FREQUENCY_GetBand                           */
/* ================================================================== */

TEST(test_getband_50mhz)
{
    /* 50 MHz → BAND1 (lower for wide-rx is 1.8 MHz) */
    ASSERT_EQ(FREQUENCY_GetBand(5000000), BAND1_50MHz);
}

TEST(test_getband_108mhz)
{
    ASSERT_EQ(FREQUENCY_GetBand(10800000), BAND2_108MHz);
}

TEST(test_getband_137mhz)
{
    ASSERT_EQ(FREQUENCY_GetBand(13700000), BAND3_137MHz);
}

TEST(test_getband_174mhz)
{
    ASSERT_EQ(FREQUENCY_GetBand(17400000), BAND4_174MHz);
}

TEST(test_getband_400mhz)
{
    ASSERT_EQ(FREQUENCY_GetBand(40000000), BAND6_400MHz);
}

TEST(test_getband_470mhz)
{
    ASSERT_EQ(FREQUENCY_GetBand(47000000), BAND7_470MHz);
}

TEST(test_getband_below_lowest)
{
    /* Below the lowest band → should still return BAND1_50MHz */
    ASSERT_EQ(FREQUENCY_GetBand(100000), BAND1_50MHz);
}

/* ================================================================== */
/*                  FREQUENCY_CalculateOutputPower                    */
/* ================================================================== */

TEST(test_output_power_below_lower)
{
    uint8_t pwr = FREQUENCY_CalculateOutputPower(10, 50, 90, 100, 200, 300, 50);
    ASSERT_EQ(pwr, 10);
}

TEST(test_output_power_above_upper)
{
    uint8_t pwr = FREQUENCY_CalculateOutputPower(10, 50, 90, 100, 200, 300, 350);
    ASSERT_EQ(pwr, 90);
}

TEST(test_output_power_at_lower)
{
    uint8_t pwr = FREQUENCY_CalculateOutputPower(10, 50, 90, 100, 200, 300, 100);
    ASSERT_EQ(pwr, 10);
}

TEST(test_output_power_at_upper)
{
    uint8_t pwr = FREQUENCY_CalculateOutputPower(10, 50, 90, 100, 200, 300, 300);
    ASSERT_EQ(pwr, 90);
}

TEST(test_output_power_at_middle)
{
    /* At the middle point, should return TxpMid + interpolation from lower half.
       lower_half = (50 - 10) * (200 - 100) / (200 - 100) = 40
       result = 50 + 40 = 90   ...wait, that's the formula from the code:
       TxpMid += ((TxpMid - TxpLow) * (Freq - Lower)) / (Mid - Lower)
       = 50 + (50 - 10)*(200-100)/(200-100) = 50 + 40 = 90 */
    uint8_t pwr = FREQUENCY_CalculateOutputPower(10, 50, 90, 100, 200, 300, 200);
    ASSERT_EQ(pwr, 90);
}

TEST(test_output_power_midway_lower_half)
{
    /* Freq=150, halfway between Lower(100) and Mid(200)
       TxpMid += (50-10)*(150-100)/(200-100) = 50 + 20 = 70 */
    uint8_t pwr = FREQUENCY_CalculateOutputPower(10, 50, 90, 100, 200, 300, 150);
    ASSERT_EQ(pwr, 70);
}

/* ================================================================== */
/*                      FREQUENCY_RoundToStep                         */
/* ================================================================== */

TEST(test_round_step_1)
{
    /* step=1 (0.01 kHz) → no rounding */
    ASSERT_EQ(FREQUENCY_RoundToStep(12345, 1), 12345u);
}

TEST(test_round_step_500)
{
    /* step=500 (<1000): rounds to nearest 500 */
    ASSERT_EQ(FREQUENCY_RoundToStep(14400000, 500), 14400000u);
    ASSERT_EQ(FREQUENCY_RoundToStep(14400250, 500), 14400500u); /* 250 rounds up */
    ASSERT_EQ(FREQUENCY_RoundToStep(14400100, 500), 14400000u); /* 100 rounds down */
}

TEST(test_round_step_1250)
{
    /* step=1250 → step/2 = 625 */
    uint32_t r = FREQUENCY_RoundToStep(14400001, 1250);
    ASSERT_EQ(r % 625, 0u);
}

TEST(test_round_step_25)
{
    /* step=25 (< 1000) → rounds to nearest multiple of 25 */
    ASSERT_EQ(FREQUENCY_RoundToStep(100, 25), 100u);
    ASSERT_EQ(FREQUENCY_RoundToStep(112, 25), 125u); /* (112+13)/25*25 = 125 */
    ASSERT_EQ(FREQUENCY_RoundToStep(87, 25), 100u);  /* (87+13)/25*25  = 100 */
}

TEST(test_round_step_833_aviation)
{
    /* Aviation 8.33 kHz spacing has special logic */
    uint32_t base = 11800000; /* 118.000 MHz */
    uint32_t r = FREQUENCY_RoundToStep(base, 833);
    ASSERT_EQ(r, base);
}

/* ================================================================== */
/*                Step index ↔ sorted index                           */
/* ================================================================== */

TEST(test_step_sorted_roundtrip)
{
    for (uint8_t si = 0; si < STEP_N_ELEM; si++) {
        STEP_Setting_t step = FREQUENCY_GetStepIdxFromSortedIdx(si);
        uint32_t back = FREQUENCY_GetSortedIdxFromStepIdx(step);
        ASSERT_EQ(back, si);
    }
}

TEST(test_step_table_values)
{
    ASSERT_EQ(gStepFrequencyTable[STEP_2_5kHz], 250);
    ASSERT_EQ(gStepFrequencyTable[STEP_5kHz],   500);
    ASSERT_EQ(gStepFrequencyTable[STEP_25kHz],  2500);
    ASSERT_EQ(gStepFrequencyTable[STEP_8_33kHz], 833);
}

/* ================================================================== */
/*                          RX_freq_check                             */
/* ================================================================== */

TEST(test_rx_freq_check_in_band)
{
    ASSERT_EQ(RX_freq_check(14500000), 0);  /* 145 MHz – OK */
}

TEST(test_rx_freq_check_below_range)
{
    ASSERT_EQ(RX_freq_check(100000), -1);   /* below lowest band */
}

TEST(test_rx_freq_check_gap)
{
    /* 63..84 MHz gap (between BX4819 bands) */
    ASSERT_EQ(RX_freq_check(70000000), -1);
}

int main(void)
{
    printf("=== Frequency tests ===\n");
    RUN(test_getband_50mhz);
    RUN(test_getband_108mhz);
    RUN(test_getband_137mhz);
    RUN(test_getband_174mhz);
    RUN(test_getband_400mhz);
    RUN(test_getband_470mhz);
    RUN(test_getband_below_lowest);
    RUN(test_output_power_below_lower);
    RUN(test_output_power_above_upper);
    RUN(test_output_power_at_lower);
    RUN(test_output_power_at_upper);
    RUN(test_output_power_at_middle);
    RUN(test_output_power_midway_lower_half);
    RUN(test_round_step_1);
    RUN(test_round_step_500);
    RUN(test_round_step_1250);
    RUN(test_round_step_25);
    RUN(test_round_step_833_aviation);
    RUN(test_step_sorted_roundtrip);
    RUN(test_step_table_values);
    RUN(test_rx_freq_check_in_band);
    RUN(test_rx_freq_check_below_range);
    RUN(test_rx_freq_check_gap);
    REPORT();
}
