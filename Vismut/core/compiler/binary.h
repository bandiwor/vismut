#ifndef VISMUT_CORE_COMPILER_BINARY_H
#define VISMUT_CORE_COMPILER_BINARY_H
#include "../ast/ast_type.h"
#include "../ast/type.h"
#include "../errors/error_details.h"
#include "op_codes.h"

VismutErrorType VismutCompiler_GetBinaryInstruction(ASTBinaryNodeType op,
                                                    const VismutType *restrict left_t,
                                                    const VismutType *restrict right_t,
                                                    VismutOpcode *restrict out_opcode,
                                                    VismutErrorDetails *restrict out_details);

#endif
