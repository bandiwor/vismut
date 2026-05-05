#ifndef VISMUT_CORE_ERRORS_ERRORS_H
#define VISMUT_CORE_ERRORS_ERRORS_H
#include "../defines.h"
#include "../types.h"

#define X_ERRORS(X)                                                                                \
    X(VISMUT_OK, "No error.")                                                                      \
    X(VISMUT_ERR_OOM, "Out of memory.")                                                            \
                                                                                                   \
    X(VISMUT_ERR_IO, "IO error.")                                                                  \
    X(VISMUT_ERR_IO_NOT_FOUND, "File not found.")                                                  \
    X(VISMUT_ERR_IO_LOCKED, "File locked.")                                                        \
    X(VISMUT_ERR_IO_LIMIT, "IO resource limit reached.")                                           \
    X(VISMUT_ERR_IO_FATAL, "Fatal IO error.")                                                      \
    X(VISMUT_ERR_IO_EOF, "Unexpected EOF.")                                                        \
    X(VISMUT_ERR_IO_DENIED, "Access denied.")                                                      \
    X(VISMUT_ERR_IO_DISK_FULL, "Disk full.")                                                       \
                                                                                                   \
    X(VISMUT_ERR_NUM_TOO_LONG, "Number literal too long.")                                         \
    X(VISMUT_ERR_NUM_PARSE, "Invalid number format.")                                              \
    X(VISMUT_ERR_NUM_OVERFLOW, "Numeric overflow.")                                                \
    X(VISMUT_ERR_NUM_BAD_SUFFIX, "Invalid numeric suffix.")                                        \
    X(VISMUT_ERR_ARRAY_LENGTH, "Invalid array length.")                                            \
    X(VISMUT_ERR_UNTERMINATED_STRING, "Unterminated string.")                                      \
    X(VISMUT_ERR_BAD_ESCAPE, "Invalid escape sequence.")                                           \
    X(VISMUT_ERR_INVALID_UNICODE, "Invalid unicode sequence.")                                     \
    X(VISMUT_ERR_STRING_TOO_LONG, "String literal too long.")                                      \
    X(VISMUT_ERR_UNEXPECTED_CHAR, "Unexpected character.")                                         \
    X(VISMUT_ERR_UNEXPECTED_TOKEN, "Unexpected token.")                                            \
    X(VISMUT_ERR_SYNTAX, "Syntax error.")                                                          \
    X(VISMUT_ERR_UNTERMINATED_COMMENT, "Unterminated comment.")                                    \
    X(VISMUT_ERR_TYPE_SYNTAX, "Type syntax error.")                                                \
    X(VISMUT_ERR_MODULE_ALREADY_LOADED, "Module already loaded.")                                  \
    X(VISMUT_ERR_CIRCULAR_DEPENDENCY, "Circular dependency.")                                      \
    X(VISMUT_ERR_DOT_ON_NON_MODULE, "Dot on non module.")                                          \
    X(VISMUT_ERR_PRIVATE_MEMBER, "Access to the private member.")                                  \
                                                                                                   \
    X(VISMUT_ERR_BAD_ARGS, "Invalid arguments.")                                                   \
    X(VISMUT_ERR_RETURN_OUTSIDE_FN, "Return outside function.")                                    \
    X(VISMUT_ERR_TYPE, "Type mismatch.")                                                           \
    X(VISMUT_ERR_CALL_ARITY, "Call arity.")                                                        \
    X(VISMUT_ERR_UNEXPECTED_TYPE, "Unexpected type.")                                              \
    X(VISMUT_ERR_CALL_NOT_A_FN, "Called not a function.")                                          \
    X(VISMUT_ERR_UNSUPPORTED_BINARY, "Unsupported binary operation.")                              \
    X(VISMUT_ERR_UNSUPPORTED_UNARY, "Unsupported unary operation.")                                \
    X(VISMUT_ERR_NON_UNIT_TYPE_AT_TOP_LEVEL, "Not unit expression type at top-level.")             \
    X(VISMUT_ERR_NAME_CONFLICT, "Name already defined.")                                           \
    X(VISMUT_ERR_NAME_NOT_FOUND, "Undefined name.")                                                \
    X(VISMUT_ERR_CONST_MUTATION, "Cannot mutate constant.")                                        \
    X(VISMUT_ERR_ASSIGN_RVALUE, "Cannot assign to rvalue.")                                        \
    X(VISMUT_ERR_REF_RVALUE, "Cannot take address of rvalue.")                                     \
                                                                                                   \
    X(VISMUT_ERR_TOO_MANY_ARGS, "Too many arguments.")                                             \
    X(VISMUT_ERR_TOO_MANY_PARAMS, "Too many parameters.")                                          \
    X(VISMUT_ERR_TOO_MANY_FIELDS, "Too many fields.")                                              \
    X(VISMUT_ERR_NESTED_FN, "Nested functions not allowed.")                                       \
                                                                                                   \
    X(VISMUT_ERR_UNKNOWN, "Unknown error.")                                                        \
    X(VISMUT_ERR_UNREACHABLE, "Unreachable code executed.")

#define INTERNAL_ERROR_LOCATION_TEMPLATE(fn) ("<core>::" STRINGIFY(fn))

typedef enum {
#define X(name, text) name,
    X_ERRORS(X)
#undef X
        VISMUT_ERROR_COUNT
} VismutErrorType;

const u8 *VismutErrorType_String(VismutErrorType);

VismutErrorType VismutErrorType_FromErrno(const int err, const VismutErrorType defaultType);

#endif
