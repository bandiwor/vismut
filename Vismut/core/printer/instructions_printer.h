#ifndef VISMUT_CORE_PRINTER_INSTRUCTIONS_PRINTER
#define VISMUT_CORE_PRINTER_INSTRUCTIONS_PRINTER
#include "../compiler/op_codes.h"

typedef struct {
    VismutInstruction *ip;
    u64 instructions_count;
} InstructionsPrinter;

InstructionsPrinter InstructionsPrinter_Create(VismutInstruction *ip, u64 count);

void InstructionsPrinter_Print(InstructionsPrinter *ctx);

#endif
