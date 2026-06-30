/*
 * Minimal unit-test framework for host-side testing of embedded C modules.
 *
 * Usage:
 *   TEST(test_name) { ASSERT(...); ASSERT_EQ(a, b); }
 *   int main(void) { RUN(test_name); REPORT(); }
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int _tf_pass;
static int _tf_fail;
static int _tf_cur_fail;

#define TEST(name) static void name(void)

#define RUN(name) do {                                          \
        _tf_cur_fail = 0;                                       \
        name();                                                 \
        if (_tf_cur_fail) {                                     \
            _tf_fail++;                                         \
            printf("  FAIL  %s\n", #name);                     \
        } else {                                                \
            _tf_pass++;                                         \
            printf("  PASS  %s\n", #name);                     \
        }                                                       \
    } while (0)

#define ASSERT(cond) do {                                       \
        if (!(cond)) {                                          \
            printf("    ASSERT failed: %s  (%s:%d)\n",          \
                   #cond, __FILE__, __LINE__);                  \
            _tf_cur_fail = 1;                                   \
        }                                                       \
    } while (0)

#define ASSERT_EQ(a, b) do {                                    \
        long long _a = (long long)(a);                          \
        long long _b = (long long)(b);                          \
        if (_a != _b) {                                         \
            printf("    ASSERT_EQ failed: %s == %lld, "         \
                   "expected %s == %lld  (%s:%d)\n",            \
                   #a, _a, #b, _b, __FILE__, __LINE__);         \
            _tf_cur_fail = 1;                                   \
        }                                                       \
    } while (0)

#define ASSERT_NEQ(a, b) do {                                   \
        long long _a = (long long)(a);                          \
        long long _b = (long long)(b);                          \
        if (_a == _b) {                                         \
            printf("    ASSERT_NEQ failed: %s == %s == %lld"    \
                   "  (%s:%d)\n",                               \
                   #a, #b, _a, __FILE__, __LINE__);             \
            _tf_cur_fail = 1;                                   \
        }                                                       \
    } while (0)

#define ASSERT_STR_EQ(a, b) do {                                \
        if (strcmp((a), (b)) != 0) {                             \
            printf("    ASSERT_STR_EQ failed: \"%s\" != \"%s\"" \
                   "  (%s:%d)\n",                               \
                   (a), (b), __FILE__, __LINE__);               \
            _tf_cur_fail = 1;                                   \
        }                                                       \
    } while (0)

#define REPORT() do {                                           \
        printf("\n%d passed, %d failed\n",                      \
               _tf_pass, _tf_fail);                             \
        return _tf_fail ? 1 : 0;                                \
    } while (0)

#endif /* TEST_FRAMEWORK_H */
