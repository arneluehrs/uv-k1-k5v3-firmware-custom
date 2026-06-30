/*
 * Unit tests for App/dcs.c  (DCS/CTCSS encoding and lookup)
 */

#include "test_framework.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Provide the ARRAY_SIZE macro that dcs.c expects from misc.h */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

/* dcs.c includes "dcs.h" and "misc.h"; supply dcs.h via the App dir
   and skip the rest of misc.h since we already defined ARRAY_SIZE. */
#define MISC_H  /* guard out misc.h */

/* Pull in the implementation */
#include "../App/dcs.c"

/* ------------------------------------------------------------------ */

TEST(test_ctcss_table_size)
{
    ASSERT_EQ(ARRAY_SIZE(CTCSS_Options), 50);
}

TEST(test_dcs_table_size)
{
    ASSERT_EQ(ARRAY_SIZE(DCS_Options), 104);
}

TEST(test_ctcss_options_sorted)
{
    for (unsigned i = 1; i < ARRAY_SIZE(CTCSS_Options); i++)
        ASSERT(CTCSS_Options[i] > CTCSS_Options[i - 1]);
}

TEST(test_golay_codeword_digital)
{
    uint32_t code = DCS_GetGolayCodeWord(CODE_TYPE_DIGITAL, 0);
    /* The Golay code for DCS_Options[0]=0x0013 + 0x800:
       DCS_CalculateGolay(0x0813).  Just verify it is a valid 23-bit word. */
    ASSERT(code != 0);
    ASSERT((code & ~0x7FFFFF) == 0);
}

TEST(test_golay_codeword_reverse_digital)
{
    uint32_t fwd = DCS_GetGolayCodeWord(CODE_TYPE_DIGITAL, 0);
    uint32_t rev = DCS_GetGolayCodeWord(CODE_TYPE_REVERSE_DIGITAL, 0);
    /* Reverse is the bitwise complement within 23 bits */
    ASSERT_EQ(fwd ^ rev, 0x7FFFFF);
}

TEST(test_golay_roundtrip)
{
    /* Encode option 5, then decode back via DCS_GetCdcssCode */
    uint32_t code = DCS_GetGolayCodeWord(CODE_TYPE_DIGITAL, 5);
    uint8_t decoded = DCS_GetCdcssCode(code);
    ASSERT_EQ(decoded, 5);
}

TEST(test_golay_roundtrip_all)
{
    for (uint8_t opt = 0; opt < ARRAY_SIZE(DCS_Options); opt++) {
        uint32_t code = DCS_GetGolayCodeWord(CODE_TYPE_DIGITAL, opt);
        uint8_t decoded = DCS_GetCdcssCode(code);
        ASSERT_EQ(decoded, opt);
    }
}

TEST(test_cdcss_code_invalid)
{
    /* A random non-Golay value should return 0xFF */
    uint8_t result = DCS_GetCdcssCode(0x000001);
    ASSERT_EQ(result, 0xFF);
}

TEST(test_ctcss_exact_match)
{
    /* CTCSS_Options[0] = 670 → index 0 */
    uint8_t idx = DCS_GetCtcssCode(670);
    ASSERT_EQ(idx, 0);
}

TEST(test_ctcss_exact_match_last)
{
    /* CTCSS_Options[49] = 2541 → index 49 */
    uint8_t idx = DCS_GetCtcssCode(2541);
    ASSERT_EQ(idx, 49);
}

TEST(test_ctcss_closest_match)
{
    /* 672 is closest to CTCSS_Options[0]=670 (delta 2) vs [1]=693 (delta 21) */
    uint8_t idx = DCS_GetCtcssCode(672);
    ASSERT_EQ(idx, 0);
}

TEST(test_ctcss_approved_index_first)
{
    /* Option 0 is approved (not in CTCSS_ExtraIdx).
       CTCSS_ExtraIdx[0]=1, so option 0 is approved_index 0. */
    uint8_t ai = DCS_GetCtcssApprovedIndex(0);
    ASSERT_EQ(ai, 0);
}

TEST(test_ctcss_approved_index_extra)
{
    /* Option 1 is in CTCSS_ExtraIdx → should return 0xFF */
    uint8_t ai = DCS_GetCtcssApprovedIndex(1);
    ASSERT_EQ(ai, 0xFF);
}

TEST(test_ctcss_approved_index_out_of_range)
{
    uint8_t ai = DCS_GetCtcssApprovedIndex(50);
    ASSERT_EQ(ai, 0xFF);
}

TEST(test_dcs_approved_index_extra)
{
    /* DCS_ExtraIdx[0]=5, so option 5 should return 0xFF */
    uint8_t ai = DCS_GetDcsApprovedIndex(5);
    ASSERT_EQ(ai, 0xFF);
}

TEST(test_dcs_approved_index_valid)
{
    /* Option 0 is not extra → should return 0 */
    uint8_t ai = DCS_GetDcsApprovedIndex(0);
    ASSERT_EQ(ai, 0);
}

TEST(test_dcs_approved_index_out_of_range)
{
    uint8_t ai = DCS_GetDcsApprovedIndex(104);
    ASSERT_EQ(ai, 0xFF);
}

int main(void)
{
    printf("=== DCS/CTCSS tests ===\n");
    RUN(test_ctcss_table_size);
    RUN(test_dcs_table_size);
    RUN(test_ctcss_options_sorted);
    RUN(test_golay_codeword_digital);
    RUN(test_golay_codeword_reverse_digital);
    RUN(test_golay_roundtrip);
    RUN(test_golay_roundtrip_all);
    RUN(test_cdcss_code_invalid);
    RUN(test_ctcss_exact_match);
    RUN(test_ctcss_exact_match_last);
    RUN(test_ctcss_closest_match);
    RUN(test_ctcss_approved_index_first);
    RUN(test_ctcss_approved_index_extra);
    RUN(test_ctcss_approved_index_out_of_range);
    RUN(test_dcs_approved_index_extra);
    RUN(test_dcs_approved_index_valid);
    RUN(test_dcs_approved_index_out_of_range);
    REPORT();
}
