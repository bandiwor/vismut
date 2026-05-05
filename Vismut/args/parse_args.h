#ifndef VISMUT_ARGS_PARSE_ARGS_H
#define VISMUT_ARGS_PARSE_ARGS_H
#include "../core/errors/errors.h"
#include "../core/types.h"
#include <stdbool.h>

typedef enum {
    VISMUT_CMD_NONE = 0,
    VISMUT_CMD_COMPILE,
    VISMUT_CMD_RUN,
    VISMUT_CMD_INTERACTIVE,
    VISMUT_CMD_HELP,
    VISMUT_CMD_VERSION,
} VismutCommand;

typedef struct {
    StringView input_file;
    StringView output_file;
    bool no_print_ast;
    bool no_print_inputs;
} VismutCompileArgs;

typedef struct {
    StringView input_file;
} VismutRunArgs;

typedef struct {
    VismutCommand command;
    union {
        VismutCompileArgs compile;
        VismutRunArgs run;
    };
} VismutArgs;

VismutErrorType VismutArgs_Parse(int argc, const char *const restrict *const restrict argv,
                                 VismutArgs *restrict out_args);

void VismutArgs_PrintHelp(StringView);

void VismutArgs_PrintVersion(void);

#endif
