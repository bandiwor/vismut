#ifndef VISMUT_CORE_CONVERT_STRING_TO_DOUBLE_H
#define VISMUT_CORE_CONVERT_STRING_TO_DOUBLE_H
#include "../errors/errors.h"

VismutErrorType VismutConvert_StrToDouble(StringView str, double *out_double);

#endif
