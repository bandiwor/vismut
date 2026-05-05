#include "Vismut/args/parse_args.h"
#include "Vismut/compile/compile.h"
#include "Vismut/core/defines.h"
#include "Vismut/core/errors/error_details.h"
#include "Vismut/core/errors/errors.h"
#include "Vismut/core/printer/error_printer.h"
#include "Vismut/core/types.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

static VismutErrorType vismut_start(const int argc, const char *restrict const *restrict const argv,
                                    VismutErrorInfo *error_info) {
    VismutErrorType err = 0;
    VismutArgs args;
    SAFE_RISKY_EXPRESSION(VismutArgs_Parse(argc, argv, &args), err);

    switch (args.command) {
    case VISMUT_CMD_COMPILE:
        return compile_program(&args.compile, error_info);
    case VISMUT_CMD_RUN:
        printf("Running doesn't support now.\n");
        return VISMUT_OK;
    case VISMUT_CMD_INTERACTIVE:
        printf("Interactive mode doesn't support now.\n");
        return VISMUT_OK;
    case VISMUT_CMD_HELP:
    case VISMUT_CMD_NONE:
        VismutArgs_PrintHelp(StringView_FromCStr(argv[0]));
        return VISMUT_OK;
    case VISMUT_CMD_VERSION:
        VismutArgs_PrintVersion();
        return VISMUT_OK;
    default:
        assert(0 && "Unreachable!\n");
        __builtin_unreachable();
        return VISMUT_ERR_UNREACHABLE;
    }
}

int main(int argc, const char *const *const argv) {
    VismutErrorInfo error_info = {0};
    const VismutErrorType code = vismut_start(argc, argv, &error_info);
    if (unlikely(code != VISMUT_OK)) {
        if (code != VISMUT_ERR_BAD_ARGS) {
            ErrorPrinter_Print(StringView_FromCStr("main"), &error_info, stdout);
            printf(":: Vismut exit with status %d (%s)\n", code, VismutErrorType_String(code));
        }
    }

    return code;
}
