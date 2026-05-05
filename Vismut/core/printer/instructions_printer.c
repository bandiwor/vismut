#include "instructions_printer.h"

InstructionsPrinter InstructionsPrinter_Create(VismutInstruction *ip, u64 count) {
    return (InstructionsPrinter){
        .ip = ip,
        .instructions_count = count,
    };
}

void InstructionsPrinter_Print(InstructionsPrinter *ctx) {
    const VismutInstruction *const start = ctx->ip;
    const VismutInstruction *const limit = ctx->ip + ctx->instructions_count;

    while (ctx->ip < limit) {
        const u32 offset = (u32)(ctx->ip - start);

        u32 raw_inst = *ctx->ip;
        VismutOpcode type = VismutInstruction_DecodeType(raw_inst);

        printf("[%04u] %-15s ", offset, VismutOpcode_String(type));

        OpcodeFormat fmt = VismutOpcode_GetFormat(type);

        switch (fmt) {
        case OP_FMT_NONE: {
            ctx->ip += 1;
            break;
        }

        case OP_FMT_A: {
            VismutDecodedABC d = VismutInstruction_DecodeABC(raw_inst);
            printf("R%u", d.a);
            ctx->ip += 1;
            break;
        }

        case OP_FMT_AB: {
            VismutDecodedABC d = VismutInstruction_DecodeABC(raw_inst);
            printf("R%u, R%u", d.a, d.b);
            ctx->ip += 1;
            break;
        }

        case OP_FMT_ABC: {
            VismutDecodedABC d = VismutInstruction_DecodeABC(raw_inst);
            printf("R%u, R%u, R%u", d.a, d.b, d.c);
            ctx->ip += 1;
            break;
        }

        case OP_FMT_ABx: {
            VismutDecodedABX d = VismutInstruction_DecodeABX(raw_inst);
            printf("R%u, %u", d.a, d.bx);
            ctx->ip += 1;
            break;
        }

        case OP_FMT_ABC_WIDE: {
            if (ctx->ip + 1 < limit) {
                VismutWideInstruction wide = VismutOpCode_MakeWide(raw_inst, ctx->ip[1]);
                VismutDecodedABCWide d = VismutInstruction_DecodeABCWide(wide);
                printf("R%u, R%u, R%u [extra: %u]", d.a, d.b, d.c, d.extra);

                ctx->ip += 2;
            } else {
                printf("<CORRUPTED WIDE INST>");
                ctx->ip += 1;
            }
            break;
        }

        default:
            printf("<UNKNOWN FORMAT>");
            ctx->ip += 1;
            break;
        }

        printf("\n");
    }
}
