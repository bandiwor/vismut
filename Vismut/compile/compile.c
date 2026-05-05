#include "compile.h"
#include "compile_context.h"

VismutErrorType compile_program(const VismutCompileArgs *args, VismutErrorInfo *out_info) {
    VismutCompileContext ctx = {0};
    VismutErrorType err;
    VismutErrorDetails error_details;

    err = VismutCompileContext_Init(&ctx, args, out_info, &error_details);
    if (err != VISMUT_OK) {
        *out_info = (VismutErrorInfo){
            .type = err,
            .source = StringView_Empty(),
            .pos = Position_Create(0, 0),
            .details = error_details,
        };
        return err;
    }

    SAFE_RISKY_EXPRESSION(VismutCompileContext_Compile(&ctx), err);

    VismutCompileContext_Free(&ctx);
    return VISMUT_OK;
}
