#include "parse_args.h"
#include "../Vismut.h"
#include <memory.h>
#include <stdio.h>

VismutErrorType VismutArgs_Parse(int argc, const char *const restrict *const restrict argv,
                                 VismutArgs *restrict out_args) {
    __builtin_memset(out_args, 0, sizeof(VismutArgs));
    if (argc < 2) {
        fprintf(stderr, "Error: No command provided.\n");
        VismutArgs_PrintHelp(StringView_FromCStr(argv[0]));
        return VISMUT_ERR_BAD_ARGS;
    }

    const char *cmd = argv[1];

    if (__builtin_strcmp(cmd, "--help") == 0) {
        out_args->command = VISMUT_CMD_HELP;
        return VISMUT_OK;
    }
    if (__builtin_strcmp(cmd, "--version") == 0) {
        out_args->command = VISMUT_CMD_VERSION;
        return VISMUT_OK;
    }

    if (__builtin_strcmp(cmd, "compile") == 0) {
        out_args->command = VISMUT_CMD_COMPILE;
        for (int i = 2; i < argc; i++) {
            const char *arg = argv[i];

            if (__builtin_strcmp(arg, "-i") == 0) {
                if (++i < argc) {
                    out_args->compile.input_file = StringView_FromCStr(argv[i]);
                } else {
                    fprintf(stderr, "Error: '-i' requires a file path.\n");
                    return VISMUT_ERR_BAD_ARGS;
                }
            } else if (__builtin_strcmp(arg, "-o") == 0) {
                if (++i < argc) {
                    out_args->compile.output_file = StringView_FromCStr(argv[i]);
                } else {
                    fprintf(stderr, "Error: '-o' requires a file path.\n");
                    return VISMUT_ERR_BAD_ARGS;
                }
            } else if (__builtin_strcmp(arg, "--no-print-ast") == 0) {
                out_args->compile.no_print_ast = true;
            } else if (__builtin_strcmp(arg, "--no-print-inputs") == 0) {
                out_args->compile.no_print_inputs = true;
            } else {
                fprintf(stderr, "Error: Unknown argument '%s' for command 'compile'.\n", arg);
                return VISMUT_ERR_BAD_ARGS;
            }
        }

        if (out_args->compile.input_file.length == 0) {
            fprintf(stderr, "Error: 'compile' requires an input file (-i).\n");
            return VISMUT_ERR_BAD_ARGS;
        }
        return VISMUT_OK;
    }

    if (__builtin_strcmp(cmd, "run") == 0) {
        out_args->command = VISMUT_CMD_RUN;
        for (int i = 2; i < argc; i++) {
            const char *arg = argv[i];

            if (__builtin_strcmp(arg, "-i") == 0) {
                if (++i < argc)
                    out_args->run.input_file = StringView_FromCStr(argv[i]);
                else {
                    fprintf(stderr, "Error: '-i' requires a file path.\n");
                    return VISMUT_ERR_BAD_ARGS;
                }
            } else {
                fprintf(stderr, "Error: Unknown argument '%s' for command 'run'.\n", arg);
                return VISMUT_ERR_BAD_ARGS;
            }
        }

        if (out_args->run.input_file.length == 0) {
            fprintf(stderr, "Error: 'run' requires an input file (-i).\n");
            return VISMUT_ERR_BAD_ARGS;
        }
        return VISMUT_OK;
    }

    if (__builtin_strcmp(cmd, "interactive") == 0) {
        out_args->command = VISMUT_CMD_INTERACTIVE;
        if (argc > 2) {
            fprintf(stderr, "Error: 'interactive' does not take any arguments.\n");
            return VISMUT_ERR_BAD_ARGS;
        }
        return VISMUT_OK;
    }

    fprintf(stderr, "Error: Unknown command '%s'.\n", cmd);
    return VISMUT_ERR_BAD_ARGS;
}

void VismutArgs_PrintHelp(const StringView self_name) {
    printf("Usage: %.*s <command> [options]\n\n", (int)self_name.length, self_name.data);
    printf("Commands:\n");
    printf("  compile      Compile a .bi file to .bic bytecode\n");
    printf("  run          Execute a .bi file directly\n");
    printf("  interactive  Start the Vismut REPL\n\n");

    printf("Global Options:\n");
    printf("  --help       Show this help message\n");
    printf("  --version    Show compiler version\n\n");

    printf("Compile Options (vismut compile):\n");
    printf("  -i <file>         Specify input .bi file (required)\n");
    printf("  -o <file>         Specify output .bic file (defaults to <input>.bic)\n");
    printf("  --no-print-ast    Disable AST dumping\n");
    printf("  --no-print-inputs Disable printing input file details\n\n");

    printf("Run Options (vismut run):\n");
    printf("  -i <file>         Specify input .bi file to run (required)\n\n");
}

void VismutArgs_PrintVersion(void) {
    printf("Vismut compiler v%d.%d.%d\n", VISMUT_VERSION_MAJOR, VISMUT_VERSION_MINOR,
           VISMUT_VERSION_PATCH);
}
