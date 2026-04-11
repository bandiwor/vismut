#ifndef VISMUT_CORE_ERRORS_ERRORS_H
#define VISMUT_CORE_ERRORS_ERRORS_H
#include "../tokenizer/token.h"
#include "../types.h"

#define X_ERRORS(X)                                                                                \
    X(VISMUT_ERROR_OK, "No error.")                                                                \
    X(VISMUT_ERROR_UNKNOWN, "Unknown error code.")                                                 \
    X(VISMUT_ERROR_UNREACHABLE, "This code is unreachable.")                                       \
    X(VISMUT_ERROR_OUT_OF_MEMORY, "Out of memory.")                                                \
    X(VISMUT_ERROR_IO, "IO error.")                                                                \
    X(VISMUT_ERROR_IO_FILE_NOT_FOUND, "IO: File not found.")                                       \
    X(VISMUT_ERROR_IO_LOCKED, "IO: Locked.")                                                       \
    X(VISMUT_ERROR_IO_RESOURCE_LIMIT, "IO: Resource limit.")                                       \
    X(VISMUT_ERROR_IO_FATAL, "IO: Fatal")                                                          \
    X(VISMUT_ERROR_IO_UNEXPECTED_EOF, "IO: Unexpected EOF.")                                       \
    X(VISMUT_ERROR_IO_ACCESS_DENIED, "IO: Access denied.")                                         \
    X(VISMUT_ERROR_IO_DISK_FULL, "IO: Disk full.")                                                 \
    X(VISMUT_ERROR_TOO_LONG_NUMBER, "Number is too long.")                                         \
    X(VISMUT_ERROR_PARSING_NUMBER, "Number parsing failed.")                                       \
    X(VISMUT_ERROR_UNTERMINATED_STRING, "Unterminated string.")                                    \
    X(VISMUT_ERROR_INVALID_STRING_ESCAPE, "Invalid string escape.")                                \
    X(VISMUT_ERROR_NUMBER_OVERFLOW, "Number overflow.")                                            \
    X(VISMUT_ERROR_NUMBER_INVALID_SUFFIX, "Number has invalid suffix.")                            \
    X(VISMUT_ERROR_UNEXPECTED_CHAR, "Unexpected char.")                                            \
    X(VISMUT_ERROR_UNEXPECTED_TOKEN, "Unexpected token.")                                          \
    X(VISMUT_ERROR_INVALID_SYNTAX, "Invalid syntax.")                                              \
    X(VISMUT_ERROR_STRING_TOO_LONG, "String too long.")                                            \
    X(VISMUT_ERROR_PARSING_ARGS, "Invalid args.")                                                  \
    X(VISMUT_ERROR_RETURN_OUTSIDE_FUNCTION, "Return outside a function not allowed")               \
    X(VISMUT_ERROR_TOO_MANY_FUNCTION_ARGUMENTS, "Too many function arguments.")

typedef enum {
#define X(name, text) name,
    X_ERRORS(X)
#undef X
        VISMUT_ERROR_COUNT
} VismutErrorType;

typedef union {
    struct {
        uint8_t caught;
    } unexpected_symbol;

    struct {
        uint8_t caught;
    } unknown_symbol;

    struct {
        VismutToken caught;
    } unexpected_token;
} VismutErrorDetails;

typedef struct {
    VismutErrorType type;
    VismutErrorDetails details;
    const u8 *module;
    StringView source;
    const u8 *location;
    int line, column, length;
} VismutErrorInfo;

const u8 *VismutErrorType_String(VismutErrorType);

VismutErrorType VismutErrorType_FromErrno(const int err, const VismutErrorType defaultType);

#endif
