#ifndef VISMUT_CORE_ERRORS_ERROR_DETAILS_H
#define VISMUT_CORE_ERRORS_ERROR_DETAILS_H
#include "../ast/ast_type.h"
#include "../ast/type.h"
#include "../tokenizer/token.h"
#include "errors.h"

typedef union {
    u8 char_;
    VismutTokenType token_type;
    VismutTypeKind type_kind;
    StringView filename;
    StringView symbol_name;
    StringView module_name;
    const VismutType *type;
    struct {
        StringView location;
        u64 bytes_required;
    } oom;
    struct {
        VismutTypeKind left;
        VismutTypeKind right;
        ASTBinaryNodeType op;
    } binary;
    struct {
        VismutTypeKind right;
        ASTUnaryNodeType op;
    } unary;
} VismutErrorDetails;

typedef struct {
    StringView source;
    VismutErrorType type;
    VismutErrorDetails details;
    Position pos;
} VismutErrorInfo;

#endif
