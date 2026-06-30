/*
 * Unit tests for pure-logic helpers in App/app/dtmf.c:
 *   - DTMF_ValidateCodes
 *   - DTMF_GetCharacter
 *   - DTMF_Append / DTMF_clear_input_box
 */

#include "test_framework.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ------------------------------------------------------------------ *
 * Provide just enough types so the functions under test compile       *
 * ------------------------------------------------------------------ */

/* KEY_Code_t values used by DTMF_GetCharacter */
enum KEY_Code_e {
    KEY_0 = 0,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_MENU, KEY_UP, KEY_DOWN, KEY_EXIT, KEY_STAR, KEY_F,
    KEY_PTT, KEY_SIDE2, KEY_SIDE1, KEY_INVALID
};
typedef enum KEY_Code_e KEY_Code_t;

/* DTMF globals used by Append / clear */
char     gDTMF_InputBox[15];
uint8_t  gDTMF_InputBox_Index = 0;
bool     gDTMF_InputMode      = false;
char     gDTMF_RX_live[20];

/* ------------------------------------------------------------------ *
 * Functions under test (copied from dtmf.c to avoid pulling all deps)*
 * ------------------------------------------------------------------ */

bool DTMF_ValidateCodes(char *pCode, const unsigned int size)
{
    unsigned int i;
    if (pCode[0] == (char)0xFF || pCode[0] == 0)
        return false;
    for (i = 0; i < size; i++) {
        if (pCode[i] == (char)0xFF || pCode[i] == 0) {
            pCode[i] = 0;
            break;
        }
        if ((pCode[i] < '0' || pCode[i] > '9') &&
            (pCode[i] < 'A' || pCode[i] > 'D') &&
            pCode[i] != '*' && pCode[i] != '#')
            return false;
    }
    return true;
}

char DTMF_GetCharacter(const unsigned int code)
{
    if (code <= KEY_9)
        return '0' + code;
    switch (code) {
        case KEY_MENU: return 'A';
        case KEY_UP:   return 'B';
        case KEY_DOWN: return 'C';
        case KEY_EXIT: return 'D';
        case KEY_STAR: return '*';
        case KEY_F:    return '#';
        default:       return (char)0xff;
    }
}

void DTMF_clear_input_box(void)
{
    memset(gDTMF_InputBox, 0, sizeof(gDTMF_InputBox));
    gDTMF_InputBox_Index = 0;
    gDTMF_InputMode      = false;
}

void DTMF_Append(const char code)
{
    if (gDTMF_InputBox_Index == 0) {
        memset(gDTMF_InputBox, '-', sizeof(gDTMF_InputBox) - 1);
        gDTMF_InputBox[sizeof(gDTMF_InputBox) - 1] = 0;
    }
    if (gDTMF_InputBox_Index < (sizeof(gDTMF_InputBox) - 1))
        gDTMF_InputBox[gDTMF_InputBox_Index++] = code;
}

void DTMF_clear_input_box_memory(void)
{
    memset(gDTMF_RX_live, 0, sizeof(gDTMF_RX_live));
}

/* ================================================================== */
/*                      DTMF_ValidateCodes                            */
/* ================================================================== */

TEST(test_validate_empty)
{
    char code[4] = {0, 0, 0, 0};
    ASSERT(!DTMF_ValidateCodes(code, 4));
}

TEST(test_validate_0xff_first)
{
    char code[4] = {(char)0xFF, '1', '2', '3'};
    ASSERT(!DTMF_ValidateCodes(code, 4));
}

TEST(test_validate_digits)
{
    char code[4] = {'1', '2', '3', 0};
    ASSERT(DTMF_ValidateCodes(code, 4));
}

TEST(test_validate_letters)
{
    char code[4] = {'A', 'B', 'C', 'D'};
    ASSERT(DTMF_ValidateCodes(code, 4));
}

TEST(test_validate_star_hash)
{
    char code[3] = {'*', '#', 0};
    ASSERT(DTMF_ValidateCodes(code, 3));
}

TEST(test_validate_invalid_char)
{
    char code[4] = {'1', 'E', '3', 0};
    ASSERT(!DTMF_ValidateCodes(code, 4));
}

TEST(test_validate_lowercase_invalid)
{
    char code[4] = {'a', 'b', 'c', 0};
    ASSERT(!DTMF_ValidateCodes(code, 4));
}

TEST(test_validate_mixed_valid)
{
    char code[8] = {'1', 'A', '*', '9', '#', 'D', '0', 0};
    ASSERT(DTMF_ValidateCodes(code, 8));
}

TEST(test_validate_truncates_at_null)
{
    char code[8] = {'1', '2', 0, 'E', 'E', 'E', 'E', 'E'};
    /* Should stop at the null and return true (first two chars valid) */
    ASSERT(DTMF_ValidateCodes(code, 8));
}

TEST(test_validate_truncates_at_0xff)
{
    char code[8] = {'1', '2', (char)0xFF, 'E', 'E', 'E', 'E', 'E'};
    ASSERT(DTMF_ValidateCodes(code, 8));
    /* The 0xFF byte should be replaced with 0 */
    ASSERT_EQ(code[2], 0);
}

/* ================================================================== */
/*                      DTMF_GetCharacter                             */
/* ================================================================== */

TEST(test_getchar_digits)
{
    for (unsigned i = KEY_0; i <= KEY_9; i++)
        ASSERT_EQ(DTMF_GetCharacter(i), '0' + i);
}

TEST(test_getchar_menu)
{
    ASSERT_EQ(DTMF_GetCharacter(KEY_MENU), 'A');
}

TEST(test_getchar_arrows)
{
    ASSERT_EQ(DTMF_GetCharacter(KEY_UP),   'B');
    ASSERT_EQ(DTMF_GetCharacter(KEY_DOWN), 'C');
}

TEST(test_getchar_exit)
{
    ASSERT_EQ(DTMF_GetCharacter(KEY_EXIT), 'D');
}

TEST(test_getchar_star)
{
    ASSERT_EQ(DTMF_GetCharacter(KEY_STAR), '*');
}

TEST(test_getchar_f_key)
{
    ASSERT_EQ(DTMF_GetCharacter(KEY_F), '#');
}

TEST(test_getchar_invalid)
{
    ASSERT_EQ((uint8_t)DTMF_GetCharacter(KEY_PTT), 0xFF);
}

/* ================================================================== */
/*                  DTMF_Append / clear_input_box                     */
/* ================================================================== */

TEST(test_append_basic)
{
    DTMF_clear_input_box();
    DTMF_Append('1');
    ASSERT_EQ(gDTMF_InputBox[0], '1');
    ASSERT_EQ(gDTMF_InputBox_Index, 1);
    /* Rest should be dashes */
    ASSERT_EQ(gDTMF_InputBox[1], '-');
}

TEST(test_append_fill)
{
    DTMF_clear_input_box();
    for (int i = 0; i < 14; i++)
        DTMF_Append('A' + (i % 4));
    ASSERT_EQ(gDTMF_InputBox_Index, 14);
    /* Should not overflow */
    DTMF_Append('X');
    ASSERT_EQ(gDTMF_InputBox_Index, 14);
}

TEST(test_clear_input_box)
{
    DTMF_Append('1');
    DTMF_clear_input_box();
    ASSERT_EQ(gDTMF_InputBox_Index, 0);
    ASSERT_EQ(gDTMF_InputMode, false);
    for (int i = 0; i < 15; i++)
        ASSERT_EQ(gDTMF_InputBox[i], 0);
}

TEST(test_clear_input_box_memory)
{
    memset(gDTMF_RX_live, 'X', sizeof(gDTMF_RX_live));
    DTMF_clear_input_box_memory();
    for (int i = 0; i < (int)sizeof(gDTMF_RX_live); i++)
        ASSERT_EQ(gDTMF_RX_live[i], 0);
}

int main(void)
{
    printf("=== DTMF tests ===\n");
    RUN(test_validate_empty);
    RUN(test_validate_0xff_first);
    RUN(test_validate_digits);
    RUN(test_validate_letters);
    RUN(test_validate_star_hash);
    RUN(test_validate_invalid_char);
    RUN(test_validate_lowercase_invalid);
    RUN(test_validate_mixed_valid);
    RUN(test_validate_truncates_at_null);
    RUN(test_validate_truncates_at_0xff);
    RUN(test_getchar_digits);
    RUN(test_getchar_menu);
    RUN(test_getchar_arrows);
    RUN(test_getchar_exit);
    RUN(test_getchar_star);
    RUN(test_getchar_f_key);
    RUN(test_getchar_invalid);
    RUN(test_append_basic);
    RUN(test_append_fill);
    RUN(test_clear_input_box);
    RUN(test_clear_input_box_memory);
    REPORT();
}
