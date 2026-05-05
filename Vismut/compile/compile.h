#ifndef VISMUT_COMPILE_COMPILE_H
#define VISMUT_COMPILE_COMPILE_H
#include "../args/parse_args.h"
#include "../core/errors/error_details.h"
#include "../core/errors/errors.h"

VismutErrorType compile_program(const VismutCompileArgs *args, VismutErrorInfo *out_info);

#endif
