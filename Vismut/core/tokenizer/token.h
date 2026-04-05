#ifndef VISMUT_CORE_TOKENIZER_TOKEN_H
#define VISMUT_CORE_TOKENIZER_TOKEN_H
#include "../defines.h"
#include "../types.h"

#define X_VISMUT_TOKENS(X)                                                                         \
    X(VISMUT_TOKEN_EOF, "<eof>")                                                                   \
    X(VISMUT_TOKEN_I1_TYPE, "i1")                                                                  \
    X(VISMUT_TOKEN_I8_TYPE, "i8")                                                                  \
    X(VISMUT_TOKEN_I16_TYPE, "i16")                                                                \
    X(VISMUT_TOKEN_I32_TYPE, "i32")                                                                \
    X(VISMUT_TOKEN_I64_TYPE, "i64")                                                                \
    X(VISMUT_TOKEN_U8_TYPE, "u8")                                                                  \
    X(VISMUT_TOKEN_U16_TYPE, "u16")                                                                \
    X(VISMUT_TOKEN_U32_TYPE, "u32")                                                                \
    X(VISMUT_TOKEN_U64_TYPE, "u64")                                                                \
    X(VISMUT_TOKEN_F32_TYPE, "f32")                                                                \
    X(VISMUT_TOKEN_F64_TYPE, "f64")                                                                \
    X(VISMUT_TOKEN_STR_TYPE, "str")                                                                \
    X(VISMUT_TOKEN_VOID_TYPE, "void")                                                              \
    X(VISMUT_TOKEN_IDENTIFIER, "<identifier>")                                                     \
    X(VISMUT_TOKEN_NAME_DECLARATION, "$")                                                          \
    X(VISMUT_TOKEN_CONDITION_STATEMENT, "#")                                                       \
    X(VISMUT_TOKEN_CONDITION_ELSE_IF, "!#")                                                        \
    X(VISMUT_TOKEN_EXCLAMATION_MARK, "!")                                                          \
    X(VISMUT_TOKEN_QUESTION_MARK, "?")                                                             \
    X(VISMUT_TOKEN_WHILE_STATEMENT, "@")                                                           \
    X(VISMUT_TOKEN_PERCENT, "%")                                                                   \
    X(VISMUT_TOKEN_FOR_STATEMENT, "%%")                                                            \
    X(VISMUT_TOKEN_NAMESPACE_DECLARATION, "<>")                                                    \
    X(VISMUT_TOKEN_STRUCTURE_DECLARATION, "$>")                                                    \
    X(VISMUT_TOKEN_RETURN_STATEMENT, "'")                                                          \
    X(VISMUT_TOKEN_PRINT_STATEMENT, "::")                                                          \
    X(VISMUT_TOKEN_INPUT_STATEMENT, "$:")                                                          \
    X(VISMUT_TOKEN_IMPORT_STATEMENT, "+>")                                                         \
    X(VISMUT_TOKEN_EXPORT_STATEMENT, "<+")                                                         \
    X(VISMUT_TOKEN_PLUS, "+")                                                                      \
    X(VISMUT_TOKEN_PLUS_PLUS, "++")                                                                \
    X(VISMUT_TOKEN_MINUS, "-")                                                                     \
    X(VISMUT_TOKEN_MINUS_MINUS, "--")                                                              \
    X(VISMUT_TOKEN_STAR, "*")                                                                      \
    X(VISMUT_TOKEN_STAR_STAR, "**")                                                                \
    X(VISMUT_TOKEN_SLASH, "/")                                                                     \
    X(VISMUT_TOKEN_ARROW_RIGHT, "->")                                                              \
    X(VISMUT_TOKEN_FAT_ARROW_RIGHT, "=>")                                                          \
    X(VISMUT_TOKEN_DOT, ".")                                                                       \
    X(VISMUT_TOKEN_COMMA, ",")                                                                     \
    X(VISMUT_TOKEN_SEMICOLON, ";")                                                                 \
    X(VISMUT_TOKEN_COLON, ":")                                                                     \
    X(VISMUT_TOKEN_LBRACE, "{")                                                                    \
    X(VISMUT_TOKEN_RBRACE, "}")                                                                    \
    X(VISMUT_TOKEN_LBRACKET, "[")                                                                  \
    X(VISMUT_TOKEN_RBRACKET, "]")                                                                  \
    X(VISMUT_TOKEN_LPAREN, "(")                                                                    \
    X(VISMUT_TOKEN_RPAREN, ")")                                                                    \
    X(VISMUT_TOKEN_LANGLE, "<")                                                                    \
    X(VISMUT_TOKEN_LESS_THAN_OR_EQUALS, "<=")                                                      \
    X(VISMUT_TOKEN_RANGLE, ">")                                                                    \
    X(VISMUT_TOKEN_GREATER_THAN_OR_EQUALS, ">=")                                                   \
    X(VISMUT_TOKEN_EQUAL, "=")                                                                     \
    X(VISMUT_TOKEN_EQUAL_EQUAL, "==")                                                              \
    X(VISMUT_TOKEN_BITWISE_OR, "|")                                                                \
    X(VISMUT_TOKEN_LOGICAL_OR, "||")                                                               \
    X(VISMUT_TOKEN_AMPERSAND, "&")                                                                 \
    X(VISMUT_TOKEN_AMPERSAND_AMPERSAND, "&&")                                                      \
    X(VISMUT_TOKEN_CIRCUMFLEX, "^")                                                                \
    X(VISMUT_TOKEN_TILDA, "~")                                                                     \
    X(VISMUT_TOKEN_SHIFT_LEFT, "<<")                                                               \
    X(VISMUT_TOKEN_SHIFT_RIGHT, ">>")                                                              \
    X(VISMUT_TOKEN_NOT_EQUALS, "!=")                                                               \
    X(VISMUT_TOKEN_INT_LITERAL, "<int literal>")                                                   \
    X(VISMUT_TOKEN_FLOAT_LITERAL, "<f64 literal>")                                                 \
    X(VISMUT_TOKEN_STR_LITERAL, "<const u8* literal>")                                             \
    X(VISMUT_TOKEN_SINGLE_LINE_COMMENT, "<single line comment>")                                   \
    X(VISMUT_TOKEN_DOC_COMMENT, "<doc comment>")                                                   \
    X(VISMUT_TOKEN_MULTILINE_COMMENT, "<multiline comment>")                                       \
    X(VISMUT_TOKEN_UNKNOWN, "<unknown>")

#define X_INT_SUFFIX(X)                                                                            \
    X(VISMUT_INT_SUFFIX_NONE, "i/u")                                                               \
    X(VISMUT_INT_SUFFIX_I1, "i1")                                                                  \
    X(VISMUT_INT_SUFFIX_I8, "i8")                                                                  \
    X(VISMUT_INT_SUFFIX_I16, "i16")                                                                \
    X(VISMUT_INT_SUFFIX_I32, "i32")                                                                \
    X(VISMUT_INT_SUFFIX_I64, "i64")                                                                \
    X(VISMUT_INT_SUFFIX_U8, "u8")                                                                  \
    X(VISMUT_INT_SUFFIX_U16, "u16")                                                                \
    X(VISMUT_INT_SUFFIX_U32, "u32")                                                                \
    X(VISMUT_INT_SUFFIX_U64, "u64")                                                                \
    X(VISMUT_INT_SUFFIX_UNKNOWN, "<unknown>")

#define X_FLOAT_SUFFIX(X)                                                                          \
    X(VISMUT_FLOAT_SUFFIX_NONE, "f32/f64")                                                         \
    X(VISMUT_FLOAT_SUFFIX_F32, "f32")                                                              \
    X(VISMUT_FLOAT_SUFFIX_F64, "f64")                                                              \
    X(VISMUT_FLOAT_SUFFIX_UNKNOWN, "<unknown>")

typedef enum {
#define X(name, text) name,
    X_VISMUT_TOKENS(X)
#undef X
        VISMUT_TOKEN_COUNT
} VismutTokenType;

typedef enum {
#define X(name, text) name,
    X_INT_SUFFIX(X)
#undef X
        VISMUT_INT_SUFFIX_COUNT
} VismutIntSuffix;

typedef enum {
#define X(name, text) name,
    X_FLOAT_SUFFIX(X)
#undef X
        VISMUT_FLOAT_SUFFIX_COUNT
} VismutFloatSuffix;

typedef struct {
    VismutTokenType type;
    Position position;

    union {
        struct {
            u64 value;
            VismutIntSuffix suffix;
        } i;
        struct {
            double value;
            VismutFloatSuffix suffix;
        } f;
        StringView str;
    } data;
} VismutToken;

attribute_const const u8 *VismutTokenType_String(VismutTokenType);
attribute_const const u8 *VismutIntSuffix_String(VismutIntSuffix);
attribute_const const u8 *VismutFloatSuffix_String(VismutFloatSuffix);

#endif
