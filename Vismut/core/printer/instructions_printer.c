#include "instructions_printer.h"
#include <stdio.h>

InstructionsPrinter InstructionsPrinter_Create(VismutInstruction *ip, u64 count,
                                               ConstantPool *constant_pool) {
    return (InstructionsPrinter){
        .ip = ip,
        .instructions_count = count,
        .constant_pool = constant_pool,
    };
}

static void print_cp_value(const ConstantPoolValue *value) {
    switch (value->type) {
    case CONSTANT_NODE_INT:
        printf("%ld", value->int_);
        break;
    case CONSTANT_NODE_UINT:
        printf("%lu", value->uint_);
        break;
    case CONSTANT_NODE_FLOAT:
        printf("%f", value->float_);
        break;
    case CONSTANT_NODE_STRING:
        printf("%.*s", value->string.length, value->string.data);
        break;
    case CONSTANT_NODE_ARRAY: {
        const u32 max_idx = value->array.length;
        u32 idx = 0;
        printf("(");
        while (idx < max_idx) {
            print_cp_value(&value->array.data[idx]);
            ++idx;
            if (idx < max_idx) {
                printf(", ");
            }
        }
        printf(")");
        break;
    }
    default:
        break;
    }
}

void InstructionsPrinter_Print(InstructionsPrinter *ctx) {
    const VismutInstruction *const start = ctx->ip;
    const VismutInstruction *const limit = (ctx->ip + ctx->instructions_count);

    while (ctx->ip < limit) {
        const u32 offset = (u32)(ctx->ip - start);
        u32 raw_inst = *ctx->ip;
        ctx->ip += 1;

        const VismutOpcode type = (VismutOpcode)VismutInstruction_Opcode(raw_inst);
        const OpcodeFormat fmt = VismutOpcode_GetFormat(type);

        printf("[%04u] %-15s ", offset, VismutOpcode_String(type));

        switch (fmt) {
        case OP_FMT_NONE: {
            break;
        }

        case OP_FMT_A: {
            printf("R%u", VismutInstruction_A(raw_inst));
            break;
        }

        case OP_FMT_AB: {
            printf("R%u, R%u", VismutInstruction_A(raw_inst), VismutInstruction_B(raw_inst));
            break;
        }

        case OP_FMT_ABC: {
            printf("R%u, R%u, R%u", VismutInstruction_A(raw_inst), VismutInstruction_B(raw_inst),
                   VismutInstruction_C(raw_inst));
            break;
        }

        case OP_FMT_ABx: {
            if (type == OP_LOAD_CONST) {
                const ConstantPoolIdx pool_idx = VismutInstruction_Bx(raw_inst);
                ConstantPoolValue *cp_value = &RawVector_At(&ctx->constant_pool->elements_vector,
                                                            ConstantPoolValue, pool_idx);
                printf("R%u, %u [", VismutInstruction_A(raw_inst), pool_idx);
                print_cp_value(cp_value);
                printf("]");
                break;
            }
            printf("R%u, %u", VismutInstruction_A(raw_inst), VismutInstruction_Bx(raw_inst));
            break;
        }

        case OP_FMT_AsBx: {
            const i32 sbx = VismutInstruction_sBx(raw_inst);
            printf("R%u, %+d (-> [%04u])", VismutInstruction_A(raw_inst), sbx, offset + 1 + sbx);
            break;
        }

        case OP_FMT_sAx: {
            const i32 sax = VismutInstruction_sAx(raw_inst);
            printf("%+d (-> [%04u])", sax, offset + 1 + sax);
            break;
        }

        case OP_FMT_ABC_WIDE: {
            if (ctx->ip < limit) {
                const u32 extra_payload = *ctx->ip;

                printf("R%u, params=%u, frame-size=%u [extra: %u]", VismutInstruction_A(raw_inst),
                       VismutInstruction_B(raw_inst), VismutInstruction_C(raw_inst), extra_payload);

                ++ctx->ip;
            } else {
                printf("<CORRUPTED WIDE INST>");
            }
            break;
        }

        default:
            printf("<UNKNOWN FORMAT>");
            break;
        }

        printf("\n");
    }
}
