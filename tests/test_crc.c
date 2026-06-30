/*
 * Unit tests for App/driver/crc.c  (CRC-CCITT calculation)
 */

#include "test_framework.h"
#include <stdint.h>

/* Pull in the implementation directly (no external deps). */
#include "../App/driver/crc.c"

/* ------------------------------------------------------------------ */

TEST(test_crc_empty_buffer)
{
    uint16_t crc = CRC_Calculate("", 0);
    ASSERT_EQ(crc, 0x0000);
}

TEST(test_crc_single_byte_zero)
{
    /* CRC-CCITT of a single 0x00 byte with init=0 is still 0x0000 */
    uint8_t data = 0x00;
    uint16_t crc = CRC_Calculate(&data, 1);
    ASSERT_EQ(crc, 0x0000);
}

TEST(test_crc_single_byte_nonzero)
{
    uint8_t data = 0x01;
    uint16_t crc = CRC_Calculate(&data, 1);
    ASSERT_NEQ(crc, 0x0000);
}

TEST(test_crc_known_vector_123456789)
{
    /* CRC-CCITT (init=0, poly=0x1021) of "123456789" should be 0x31C3 */
    const char *data = "123456789";
    uint16_t crc = CRC_Calculate(data, 9);
    ASSERT_EQ(crc, 0x31C3);
}

TEST(test_crc_different_data_different_crc)
{
    uint16_t crc_a = CRC_Calculate("ABCD", 4);
    uint16_t crc_b = CRC_Calculate("ABCE", 4);
    ASSERT(crc_a != crc_b);
}

TEST(test_crc_same_data_same_crc)
{
    uint16_t crc1 = CRC_Calculate("hello", 5);
    uint16_t crc2 = CRC_Calculate("hello", 5);
    ASSERT_EQ(crc1, crc2);
}

TEST(test_crc_all_zeros)
{
    uint8_t data[4] = {0, 0, 0, 0};
    uint16_t crc = CRC_Calculate(data, 4);
    ASSERT_EQ(crc, 0x0000);
}

TEST(test_crc_all_0xff)
{
    uint8_t data[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint16_t crc = CRC_Calculate(data, 4);
    ASSERT_NEQ(crc, 0x0000);
}

TEST(test_crc_single_0xff)
{
    uint8_t data = 0xFF;
    uint16_t crc = CRC_Calculate(&data, 1);
    /* CRC-CCITT of a single 0xFF byte with init=0: 0x1EF0 */
    ASSERT_EQ(crc, 0x1EF0);
}

TEST(test_crc_incremental_length)
{
    /* Verify CRC changes as buffer grows */
    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint16_t prev = 0;
    for (uint16_t len = 1; len <= 8; len++) {
        uint16_t crc = CRC_Calculate(data, len);
        ASSERT(crc != prev);
        prev = crc;
    }
}

int main(void)
{
    printf("=== CRC tests ===\n");
    RUN(test_crc_empty_buffer);
    RUN(test_crc_single_byte_zero);
    RUN(test_crc_single_byte_nonzero);
    RUN(test_crc_known_vector_123456789);
    RUN(test_crc_different_data_different_crc);
    RUN(test_crc_same_data_same_crc);
    RUN(test_crc_all_zeros);
    RUN(test_crc_all_0xff);
    RUN(test_crc_single_0xff);
    RUN(test_crc_incremental_length);
    REPORT();
}
