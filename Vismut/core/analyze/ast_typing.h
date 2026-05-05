#ifndef VISMUT_CORE_ANALYZE_AST_TYPING_H
#define VISMUT_CORE_ANALYZE_AST_TYPING_H
#include "../memory/type_context.h"

attribute_noinline VismutErrorType GetBinaryOpResult(VismutTypeContext *restrict ctx,
                                                     const VismutType *restrict left_t,
                                                     const VismutType *restrict right_t,
                                                     const VismutType **restrict out_type,
                                                     ASTBinaryNodeType operation,
                                                     VismutErrorDetails *restrict out_details);

VismutErrorType GetUnaryOpResult(VismutTypeContext *restrict ctx,
                                 const VismutType *restrict right_t, ASTUnaryNodeType operation,
                                 const VismutType **restrict out_type,
                                 VismutErrorDetails *restrict out_details);

VismutErrorType CheckAssignment(VismutTypeContext *restrict ctx,
                                const VismutType *restrict target_t,
                                const VismutType *restrict value_t, ASTAssignNodeType operation,
                                VismutErrorDetails *restrict out_details);

#endif
