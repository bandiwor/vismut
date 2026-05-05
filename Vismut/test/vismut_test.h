#ifndef VISMUT_TEST_VISMUT_TEST_H
#define VISMUT_TEST_VISMUT_TEST_H

#define VISMUT_FLOAT_EPSILON 1e-6

typedef void (*VismutTestFunc)(void);

typedef struct VismutTestNode {
    const char *suite_name;
    const char *test_name;
    VismutTestFunc func;
    struct VismutTestNode *next;
} VismutTestNode;

extern VismutTestNode *g_vismut_test_list;
extern int g_vismut_tests_run;
extern int g_vismut_tests_failed;
extern const char *g_vismut_current_test;

#define TEST(suite, name)                                                                          \
    static void test_##suite##_##name(void);                                                       \
    __attribute__((constructor)) static void register_##suite##_##name(void) {                     \
        static VismutTestNode node = {#suite, #name, test_##suite##_##name, NULL};                 \
        node.next = g_vismut_test_list;                                                            \
        g_vismut_test_list = &node;                                                                \
    }                                                                                              \
    static void test_##suite##_##name(void)

#define VISMUT_FAIL()                                                                              \
    do {                                                                                           \
        fprintf(stderr, "[  FAILED  ] %s:%d\n", __FILE__, __LINE__);                               \
        g_vismut_tests_failed++;                                                                   \
        return;                                                                                    \
    } while (0)

#define EXPECT_TRUE(cond)                                                                          \
    do {                                                                                           \
        if (unlikely(!(cond))) {                                                                   \
            fprintf(stderr, "Expected true, got false: %s\n", #cond);                              \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_EQ_INT(expected, actual)                                                            \
    do {                                                                                           \
        const i64 e = (expected);                                                                  \
        const i64 a = (actual);                                                                    \
        if (unlikely(e != a)) {                                                                    \
            fprintf(stderr, "Expected %ld, got %ld\n", e, a);                                      \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_EQ_FLOAT(expected, actual)                                                          \
    do {                                                                                           \
        const f64 e = (f64)(expected);                                                             \
        const f64 a = (f64)(actual);                                                               \
        const f64 diff = __builtin_fabs(e - a);                                                    \
        if (unlikely(diff > VISMUT_FLOAT_EPSILON)) {                                               \
            fprintf(stderr, "Float mismatch. Expected: %f, Got: %f (diff: %f)\n", e, a, diff);     \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_SV_EQ(expected_cstr, actual_sv)                                                     \
    do {                                                                                           \
        const char *e = (expected_cstr);                                                           \
        const StringView a = (actual_sv);                                                          \
        const size_t e_len = __builtin_strlen(e);                                                  \
        if (unlikely(e_len != a.length || __builtin_memcmp(e, a.data, e_len) != 0)) {              \
            fprintf(stderr, "StringView mismatch. Expected: '%s', Got: '%.*s'\n", e,               \
                    (int)a.length, a.data);                                                        \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_FAIL_CODE(expected, status)                                                         \
    do {                                                                                           \
        const VismutErrorType s = (status);                                                        \
        const VismutErrorType e = (expected);                                                      \
        if (unlikely(s != e)) {                                                                    \
            fprintf(stderr, "Expected '%s', got '%s'\n", VismutErrorType_String(e),                \
                    VismutErrorType_String(s));                                                    \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_SUCCESS(status)                                                                     \
    do {                                                                                           \
        const long s = (status);                                                                   \
        if (unlikely(s != VISMUT_OK)) {                                                            \
            fprintf(stderr, "Expected success, got '%s'\n", VismutErrorType_String(s));            \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_TOKEN_TYPE(expected_type, actual_token)                                             \
    do {                                                                                           \
        const VismutTokenType e = (expected_type);                                                 \
        const VismutTokenType a = (actual_token).type;                                             \
        if (unlikely(e != a)) {                                                                    \
            fprintf(stderr, "Token mismatch. Expected: %s, Got: %s\n", VismutTokenType_String(e),  \
                    VismutTokenType_String(a));                                                    \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

#define EXPECT_TOKEN_POS(expected_pos, actual_token)                                               \
    do {                                                                                           \
        const Position e = expected_pos;                                                           \
        const VismutToken a_token = actual_token;                                                  \
        const Position a = a_token.position;                                                       \
        if (unlikely(e.length != a.length || e.offset != a.offset)) {                              \
            fprintf(                                                                               \
                stderr,                                                                            \
                "Token %s position mismatch. Expected: (off=%u,len=%u), Got: (off=%u,len=%u)\n",   \
                VismutTokenType_String(a_token.type), e.offset, e.length, a.offset, a.length);     \
            VISMUT_FAIL();                                                                         \
        }                                                                                          \
    } while (0)

int VismutTest_RunAll(void);

#endif
