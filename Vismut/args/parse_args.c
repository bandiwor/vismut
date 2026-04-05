#include "parse_args.h"
#include "../core/defines.h"
#include <stdio.h>
#include <string.h>

void VismutArgs_PrintHelp(const char *const self_name) {
    printf("Usage: %s [OPTIONS] [INPUT_FILE]\n\n", self_name);
    printf("Options:\n");

#define X(type, name, str, field, def, desc) printf("\t%-15s %s\n", str, desc);
    X_ARGS(X)
#undef X

    printf("\n");
}

attribute_nonnull(1, 3) int VismutArgs_Parse(const char *restrict const *restrict const argv,
                                             const int argc, VismutArgs *restrict out_args) {
    memset(out_args, 0, sizeof(VismutArgs));

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (arg[0] == '-') {
#define X(type, name, str, field, def, desc)                                                       \
    if (__builtin_strcmp(arg, str) == 0) {                                                         \
        if (type == ARG_FLAG) {                                                                    \
            out_args->field##_int = 1;                                                             \
        } else if (type == ARG_VALUE) {                                                            \
            if (likely(i + 1 < argc)) {                                                            \
                out_args->field##_str = argv[++i];                                                 \
            } else {                                                                               \
                fprintf(stderr, "Error: Argument '%s' requires a value.\n", str);                  \
                return 0;                                                                          \
            }                                                                                      \
        }                                                                                          \
    } else
            X_ARGS(X)
#undef X
            /* else */ {
                fprintf(stderr, "Error: Unknown argument '%s'.\n", arg);
                return 0;
            }
        } else {
            if (out_args->input_file == NULL) {
                out_args->input_file = (const u8 *)arg;
            } else {
                fprintf(stderr, "Error: Multiple input files are not supported ('%s').\n", arg);
                return 0;
            }
        }
    }

    return 1;
}
