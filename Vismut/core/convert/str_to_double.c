#include "str_to_double.h"
#include "../utils/issomechar.h"
#include "math.h"

static const double PowersOf10[] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
                                    1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
                                    1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

static const double NegPowersOf10[] = {1e-0,  1e-1,  1e-2,  1e-3,  1e-4,  1e-5,  1e-6,  1e-7,
                                       1e-8,  1e-9,  1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15,
                                       1e-16, 1e-17, 1e-18, 1e-19, 1e-20, 1e-21, 1e-22};

VismutErrorType VismutConvert_StrToDouble(const StringView str, double *out_double) {
    const u8 *start = str.data;
    const u8 *end = str.data + str.length;

    const u8 *p = start;
    double sign = 1.0;

    if (p < end) {
        if (*p == '-') {
            sign = -1.0;
            p++;
        } else if (*p == '+') {
            p++;
        }
    }

    const u8 *const first_digit_pos = p;
    u64 mantissa = 0;
    int exp_adj = 0;
    int has_dot = 0;

    while (p < end) {
        const u8 c = *p;
        if (IsDecDigit(c)) {
            if (unlikely(mantissa == 0 && c == '0')) {
                if (has_dot)
                    exp_adj--;
                p++;
                continue;
            }
            if (likely(mantissa < 1844674407370955161ULL)) {
                mantissa = (mantissa * 10) + (c - '0');
                if (has_dot)
                    exp_adj--;
            } else {
                if (!has_dot)
                    exp_adj++;
            }
            p++;
        } else if (c == '.' && !has_dot) {
            has_dot = 1;
            p++;
        } else {
            break;
        }
    }

    if (unlikely(p == first_digit_pos || (has_dot && (p - first_digit_pos == 1)))) {
        return VISMUT_ERROR_PARSING_NUMBER;
    }

    if (p < end && (*p == 'e' || *p == 'E')) {
        p++;

        int e_sign = 1;
        if (p < end) {
            if (*p == '-') {
                e_sign = -1;
                p++;
            } else if (*p == '+')
                p++;
        }

        const uint8_t *const e_digits_start = p;
        int e_val = 0;
        while (p < end && IsDecDigit(*p)) {
            if (e_val < 1024)
                e_val = (e_val * 10) + (*p - '0');
            p++;
        }

        if (p != e_digits_start) {
            exp_adj += (e_sign * e_val);
        }
    }

    if (unlikely(mantissa == 0)) {
        *out_double = 0.0 * sign;
        return VISMUT_ERROR_OK;
    }

    double res = (double)mantissa;

    if (exp_adj != 0) {
        const int abs_exp = exp_adj < 0 ? -exp_adj : exp_adj;
        if (abs_exp < (int)(sizeof(PowersOf10) / sizeof(double))) {
            if (exp_adj > 0)
                res *= PowersOf10[abs_exp];
            else
                res *= NegPowersOf10[abs_exp];
        } else {
            res *= pow(10.0, exp_adj);
        }
    }

    if (unlikely(isinf(res))) {
        return VISMUT_ERROR_NUMBER_OVERFLOW;
    }

    *out_double = res * sign;
    return VISMUT_ERROR_OK;
}
