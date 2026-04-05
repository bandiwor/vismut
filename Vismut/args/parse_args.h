#ifndef VISMUT_ARGS_PARSE_ARGS_H
#define VISMUT_ARGS_PARSE_ARGS_H
#include "../core/types.h"

typedef enum { ARG_FLAG, ARG_VALUE } VismutArgType;

#define X_ARGS(X)                                                                                  \
    X(ARG_FLAG, HELP, "--help", is_help, 0, "Show help information")                               \
    X(ARG_VALUE, OUTPUT, "-o", output_path, "vismut.output.c", "Output .c file")

typedef struct {
    const u8 *input_file;
#define X(type, name, str, field, def, desc)                                                       \
    union {                                                                                        \
        const char *field##_str;                                                                   \
        int field##_int;                                                                           \
    };
    X_ARGS(X)
#undef X
} VismutArgs;

/* Returns 1 if successed
 * Returns 0 if failed
 */
int VismutArgs_Parse(const char *restrict const *restrict const argv, const int argc,
                     VismutArgs *restrict out_args);

void VismutArgs_PrintHelp(const char *const self_name);

#endif
