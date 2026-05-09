#ifndef VISMUT_CORE_PRINTER_INSTRUCTIONS_PRINTER
#define VISMUT_CORE_PRINTER_INSTRUCTIONS_PRINTER
#include "../compiler/op_codes.h"
#include "../memory/constant_pool.h"

typedef struct {
    VismutInstruction *ip;
    u64 instructions_count;
    ConstantPool *constant_pool;
} InstructionsPrinter;

InstructionsPrinter InstructionsPrinter_Create(VismutInstruction *ip, u64 count,
                                               ConstantPool *constant_pool);

void InstructionsPrinter_Print(InstructionsPrinter *ctx);

#endif
