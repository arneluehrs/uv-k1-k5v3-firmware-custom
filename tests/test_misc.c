/*
 * Unit tests for pure-logic helpers in App/misc.c:
 *   - NUMBER_AddWithWraparound
 *   - StrToUL
 *   - Cache helpers (MR_FindInCache, MR_FindOldestCacheEntry, etc.)
 */

#include "test_framework.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ------------------------------------------------------------------ *
 * Provide just enough definitions so the functions under test compile *
 * ------------------------------------------------------------------ */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#define MR_CHANNELS_MAX       1024
#define MR_CHANNELS_LIST      24
#define MR_CHANNELS_CACHE_SIZE 10

typedef union {
    struct {
        uint16_t
            band :      3,
            compander : 2,
            unused_1 :  1,
            unused_2 :  1,
            exclude :   1,
            scanlist :  8;
    };
    uint16_t __val;
} ChannelAttributes_t;

typedef struct {
    uint16_t channel_id;
    ChannelAttributes_t attributes;
    uint32_t access_time;
} MR_ChannelCache_t;

MR_ChannelCache_t gMR_ChannelAttributes_Cache[MR_CHANNELS_CACHE_SIZE] = {0};
ChannelAttributes_t gMR_ChannelAttributes_Current = {0};

/* Stub for GetCurrentTime – deterministic counter for testing */
uint32_t gBlinkCounter;

/* Stubs for Flash access – used by MR_LoadChannelAttributesFromFlash / Save */
static uint8_t _fake_flash[4096] = {0};

void PY25Q16_ReadBuffer(uint32_t addr, void *buf, uint16_t size) {
    memcpy(buf, _fake_flash + addr, size);
}

void PY25Q16_WriteBuffer(uint16_t addr, const void *buf, uint16_t size, bool flag) {
    (void)flag;
    memcpy(_fake_flash + addr, buf, size);
}

/* ------------------------------------------------------------------
 * Now pull in just the functions from misc.c we want to test.
 * We can't #include the whole file because of heavy deps, so we
 * replicate the standalone functions here (they are short and stable).
 * ------------------------------------------------------------------ */

int32_t NUMBER_AddWithWraparound(int32_t Base, int32_t Add, int32_t LowerLimit, int32_t UpperLimit)
{
    Base += Add;
    if (Base == 0x7fffffff || Base < LowerLimit)
        return UpperLimit;
    if (Base > UpperLimit)
        return LowerLimit;
    return Base;
}

unsigned long StrToUL(const char *str)
{
    unsigned long ul = 0;
    for (uint8_t i = 0; i < strlen(str); i++) {
        char c = str[i];
        if (c < '0' || c > '9')
            break;
        ul = ul * 10 + (uint8_t)(c - '0');
    }
    return ul;
}

/* Cache helpers (copied verbatim from misc.c to avoid pulling full deps) */

static int MR_FindInCache(uint16_t channel_id)
{
    for (int i = 0; i < MR_CHANNELS_CACHE_SIZE; i++)
        if (gMR_ChannelAttributes_Cache[i].channel_id == channel_id)
            return i;
    return -1;
}

static int MR_FindOldestCacheEntry(void)
{
    int oldest_index = 0;
    uint32_t oldest_time = gMR_ChannelAttributes_Cache[0].access_time;
    for (int i = 1; i < MR_CHANNELS_CACHE_SIZE; i++) {
        if (gMR_ChannelAttributes_Cache[i].access_time < oldest_time) {
            oldest_time = gMR_ChannelAttributes_Cache[i].access_time;
            oldest_index = i;
        }
    }
    return oldest_index;
}

static int MR_FindEmptyCacheSlot(void)
{
    for (int i = 0; i < MR_CHANNELS_CACHE_SIZE; i++)
        if (gMR_ChannelAttributes_Cache[i].channel_id == 0xFFFF)
            return i;
    return -1;
}

void MR_InvalidateChannelAttributesCache(void)
{
    for (int i = 0; i < MR_CHANNELS_CACHE_SIZE; i++) {
        gMR_ChannelAttributes_Cache[i].channel_id = 0xFFFF;
        gMR_ChannelAttributes_Cache[i].access_time = 0;
    }
}

/* ================================================================== */
/*                      NUMBER_AddWithWraparound                      */
/* ================================================================== */

TEST(test_wraparound_no_wrap)
{
    ASSERT_EQ(NUMBER_AddWithWraparound(5, 1, 0, 10), 6);
}

TEST(test_wraparound_at_upper)
{
    /* 10 + 1 > 10 → wraps to lower (0) */
    ASSERT_EQ(NUMBER_AddWithWraparound(10, 1, 0, 10), 0);
}

TEST(test_wraparound_at_lower)
{
    /* 0 + (-1) = -1 < 0 → wraps to upper (10) */
    ASSERT_EQ(NUMBER_AddWithWraparound(0, -1, 0, 10), 10);
}

TEST(test_wraparound_stay_at_boundary)
{
    ASSERT_EQ(NUMBER_AddWithWraparound(0, 0, 0, 10), 0);
    ASSERT_EQ(NUMBER_AddWithWraparound(10, 0, 0, 10), 10);
}

TEST(test_wraparound_int_max)
{
    /* If Base reaches 0x7fffffff → upper */
    ASSERT_EQ(NUMBER_AddWithWraparound(0x7ffffffe, 1, 0, 100), 100);
}

TEST(test_wraparound_negative_range)
{
    ASSERT_EQ(NUMBER_AddWithWraparound(-5, 1, -10, -1), -4);
    ASSERT_EQ(NUMBER_AddWithWraparound(-1, 1, -10, -1), -10);
    ASSERT_EQ(NUMBER_AddWithWraparound(-10, -1, -10, -1), -1);
}

/* ================================================================== */
/*                            StrToUL                                 */
/* ================================================================== */

TEST(test_strtoul_basic)
{
    ASSERT_EQ(StrToUL("12345"), 12345UL);
}

TEST(test_strtoul_zero)
{
    ASSERT_EQ(StrToUL("0"), 0UL);
}

TEST(test_strtoul_leading_zeros)
{
    ASSERT_EQ(StrToUL("007"), 7UL);
}

TEST(test_strtoul_trailing_non_digit)
{
    ASSERT_EQ(StrToUL("42abc"), 42UL);
}

TEST(test_strtoul_empty)
{
    ASSERT_EQ(StrToUL(""), 0UL);
}

TEST(test_strtoul_non_digit_start)
{
    ASSERT_EQ(StrToUL("abc"), 0UL);
}

TEST(test_strtoul_large_number)
{
    ASSERT_EQ(StrToUL("4294967295"), 4294967295UL);
}

/* ================================================================== */
/*                      Cache helpers                                 */
/* ================================================================== */

TEST(test_cache_invalidate)
{
    MR_InvalidateChannelAttributesCache();
    for (int i = 0; i < MR_CHANNELS_CACHE_SIZE; i++) {
        ASSERT_EQ(gMR_ChannelAttributes_Cache[i].channel_id, 0xFFFF);
        ASSERT_EQ(gMR_ChannelAttributes_Cache[i].access_time, 0);
    }
}

TEST(test_cache_find_empty_slot)
{
    MR_InvalidateChannelAttributesCache();
    /* All slots should be empty, first empty is 0 */
    ASSERT_EQ(MR_FindEmptyCacheSlot(), 0);
}

TEST(test_cache_find_in_cache_miss)
{
    MR_InvalidateChannelAttributesCache();
    ASSERT_EQ(MR_FindInCache(42), -1);
}

TEST(test_cache_find_in_cache_hit)
{
    MR_InvalidateChannelAttributesCache();
    gMR_ChannelAttributes_Cache[3].channel_id = 42;
    ASSERT_EQ(MR_FindInCache(42), 3);
}

TEST(test_cache_find_oldest)
{
    MR_InvalidateChannelAttributesCache();
    for (int i = 0; i < MR_CHANNELS_CACHE_SIZE; i++) {
        gMR_ChannelAttributes_Cache[i].channel_id = i;
        gMR_ChannelAttributes_Cache[i].access_time = 100 + i;
    }
    /* Slot 0 has the oldest time (100) */
    ASSERT_EQ(MR_FindOldestCacheEntry(), 0);

    /* Make slot 5 the oldest */
    gMR_ChannelAttributes_Cache[5].access_time = 1;
    ASSERT_EQ(MR_FindOldestCacheEntry(), 5);
}

TEST(test_cache_no_empty_when_full)
{
    MR_InvalidateChannelAttributesCache();
    for (int i = 0; i < MR_CHANNELS_CACHE_SIZE; i++)
        gMR_ChannelAttributes_Cache[i].channel_id = i;
    ASSERT_EQ(MR_FindEmptyCacheSlot(), -1);
}

int main(void)
{
    printf("=== misc tests ===\n");
    RUN(test_wraparound_no_wrap);
    RUN(test_wraparound_at_upper);
    RUN(test_wraparound_at_lower);
    RUN(test_wraparound_stay_at_boundary);
    RUN(test_wraparound_int_max);
    RUN(test_wraparound_negative_range);
    RUN(test_strtoul_basic);
    RUN(test_strtoul_zero);
    RUN(test_strtoul_leading_zeros);
    RUN(test_strtoul_trailing_non_digit);
    RUN(test_strtoul_empty);
    RUN(test_strtoul_non_digit_start);
    RUN(test_strtoul_large_number);
    RUN(test_cache_invalidate);
    RUN(test_cache_find_empty_slot);
    RUN(test_cache_find_in_cache_miss);
    RUN(test_cache_find_in_cache_hit);
    RUN(test_cache_find_oldest);
    RUN(test_cache_no_empty_when_full);
    REPORT();
}
