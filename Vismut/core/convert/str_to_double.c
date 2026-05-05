#include "str_to_double.h"
#include "../utils/issomechar.h"
#include <stdint.h>

static const f64 PowersOf10[] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
                                 1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
                                 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

static const f64 NegPowersOf10[] = {1e-0,  1e-1,  1e-2,  1e-3,  1e-4,  1e-5,  1e-6,  1e-7,
                                    1e-8,  1e-9,  1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15,
                                    1e-16, 1e-17, 1e-18, 1e-19, 1e-20, 1e-21, 1e-22};

#define MAX_MANTISSA_DIV_10 1844674407370955161ULL

VismutErrorType VismutConvert_StrToDouble(const StringView str, double *out_double) {
    if (unlikely(!str.data || str.length == 0)) {
        return VISMUT_ERR_NUM_PARSE;
    }

    const u8 *p = str.data;
    const u8 *const end = str.data + str.length;

    f64 sign = 1.0;
    if (*p == '-' || *p == '+') {
        sign = (*p == '-') ? -1.0 : 1.0;
        p++;
    }

    u64 mantissa = 0;
    int exp_adj = 0;
    i1 has_digits = 0;

    while (p < end && *p == '0') {
        has_digits = 1;
        p++;
    }

    while (p < end && IsDecDigit(*p)) {
        has_digits = 1;
        const u8 c = *p++;

        if (likely(mantissa < MAX_MANTISSA_DIV_10)) {
            mantissa = (mantissa * 10) + (c - '0');
        } else {
            exp_adj++;
        }
    }

    if (p < end && *p == '.') {
        p++;
        while (p < end && IsDecDigit(*p)) {
            has_digits = 1;
            const u8 c = *p++;

            if (mantissa == 0 && c == '0') {
                exp_adj--;
            } else if (likely(mantissa < MAX_MANTISSA_DIV_10)) {
                mantissa = (mantissa * 10) + (c - '0');
                exp_adj--;
            }
        }
    }

    if (unlikely(!has_digits)) {
        return VISMUT_ERR_NUM_PARSE;
    }

    if (p < end && (*p == 'e' || *p == 'E')) {
        p++;
        int e_sign = 1;
        if (p < end && (*p == '-' || *p == '+')) {
            e_sign = (*p == '-') ? -1 : 1;
            p++;
        }

        int e_val = 0;
        while (p < end && IsDecDigit(*p)) {
            if (e_val < 10000) {
                e_val = (e_val * 10) + (*p - '0');
            }
            p++;
        }
        exp_adj += (e_sign * e_val);
    }

    if (unlikely(mantissa == 0)) {
        *out_double = 0.0 * sign;
        return VISMUT_OK;
    }

    if (exp_adj > 310) {
        *out_double = sign * ((f64)1e300 * 1e300);
        return VISMUT_ERR_NUM_OVERFLOW;
    } else if (exp_adj < -340) {
        *out_double = 0.0 * sign;
        return VISMUT_OK;
    }

    f64 res = (f64)mantissa;

    if (exp_adj != 0) {
        if (exp_adj > 0) {
            while (exp_adj >= 22) {
                res *= 1e22;
                exp_adj -= 22;
            }
            if (exp_adj > 0) {
                res *= PowersOf10[exp_adj];
            }
        } else {
            int abs_exp = -exp_adj;
            while (abs_exp >= 22) {
                res *= 1e-22;
                abs_exp -= 22;
            }
            if (abs_exp > 0) {
                res *= NegPowersOf10[abs_exp];
            }
        }
    }

    union {
        f64 f;
        i64 i;
    } u = {.f = res};

    if (unlikely((u.i & 0x7FFFFFFFFFFFFFFF) == 0x7FF0000000000000)) {
        return VISMUT_ERR_NUM_OVERFLOW;
    }

    *out_double = res * sign;
    return VISMUT_OK;
}
