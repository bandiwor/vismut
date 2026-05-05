#ifndef VISMUT_CORE_COMPILER_UNARY_H
#define VISMUT_CORE_COMPILER_UNARY_H
#include "../ast/ast_type.h"
#include "../ast/type.h"
#include "../errors/error_details.h"
#include "op_codes.h"

VismutErrorType VismutCompiler_GetUnaryInstruction(ASTUnaryNodeType op,
                                                   const VismutType *restrict right_t,
                                                   VismutOpcode *restrict out_opcode,
                                                   VismutErrorDetails *restrict out_details);

#endif
