#include "tokenizer.h"
#include "../convert/str_to_double.h"
#include "../convert/str_to_int.h"
#include "../functions.h"
#include "../utils/issomechar.h"
#include "../utils/suffix.h"
#include "token.h"
#include "tokenizer_charmap.h"
#include <assert.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

attribute_nonnull(2, 3) attribute_nodiscard VismutTokenizer
    VismutTokenizer_Create(const StringView source, const u8 *restrict const source_filename,
                           Arena *restrict arena, VismutErrorInfo *restrict error_info) {
    if (error_info != NULL) {
        *error_info = (VismutErrorInfo){
            .source = source,
            .type = VISMUT_ERROR_OK,
        };
    }

    return (VismutTokenizer){
        .arena = arena,
        .start = source.data,
        .token_start = source.data,
        .cursor = source.data,
        .limit = source.data + source.length,
        .source_filename = source_filename,
        .error_info = error_info,
    };
}

attribute_nonnull(1) void VismutTokenizer_Reset(VismutTokenizer *tokenizer) {
    if (tokenizer->error_info != NULL) {
        *tokenizer->error_info = (VismutErrorInfo){
            .type = VISMUT_ERROR_OK,
        };
    }

    tokenizer->cursor = tokenizer->start;
    tokenizer->token_start = tokenizer->start;
}

attribute_hot static void Tokenizer_SkipWhitespace(VismutTokenizer *tokenizer);

attribute_nonnull(1, 2) static VismutErrorType
    Tokenizer_ParseNumber(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token);

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseIdentifier(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token);

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseString(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token);

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseOperator(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token);

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseSingleLineComment(VismutTokenizer *restrict tokenizer,
                                     VismutToken *restrict out_token);

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseMultiLineComment(VismutTokenizer *restrict tokenizer,
                                    VismutToken *restrict out_token);

attribute_nonnull(1, 2) attribute_nodiscard VismutErrorType
    VismutTokenizer_Next(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token) {
    const u8 *const limit = tokenizer->limit;

    while (1) {
        Tokenizer_SkipWhitespace(tokenizer);

        const u8 *cur = tokenizer->cursor;

        if (unlikely(cur >= limit)) {
            *out_token = (VismutToken){.type = VISMUT_TOKEN_EOF,
                                       .position = {
                                           .offset = tokenizer->limit - tokenizer->start,
                                           .length = 0,
                                       }};
            return VISMUT_ERROR_OK;
        }

        tokenizer->token_start = cur;
        out_token->position.offset = (u64)cur - (u64)tokenizer->start;

        if (unlikely(cur + 1 < limit && *cur == '/')) {
            if (*(cur + 1) == '/') {
                return Tokenizer_ParseSingleLineComment(tokenizer, out_token);
            } else if (*(cur + 1) == '*') {
                return Tokenizer_ParseMultiLineComment(tokenizer, out_token);
            }
        }

        const u8 c = *cur;
        cur++;
        tokenizer->cursor = cur;

        switch (CharMap[c]) {
        case CT_ALPHA:
            return Tokenizer_ParseIdentifier(tokenizer, out_token);
        case CT_DIGIT:
            return Tokenizer_ParseNumber(tokenizer, out_token);
        case CT_QUOTES:
            return Tokenizer_ParseString(tokenizer, out_token);
        case CT_OPERATOR:
            return Tokenizer_ParseOperator(tokenizer, out_token);
        default:
            return VISMUT_ERROR_UNEXPECTED_CHAR;
        }
    }

    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseSingleLineComment(VismutTokenizer *restrict tokenizer,
                                     VismutToken *restrict out_token) {
    const u8 *const start = tokenizer->token_start;
    const u8 *cur = tokenizer->cursor + 2; // =start + 2
    const u8 *const limit = tokenizer->limit;
    int is_doc_comment = 0;

    if (cur < limit && *cur == '/') {
        is_doc_comment = 1;
        ++cur;
    }

    while (cur < limit && *cur != '\n') {
        ++cur;
    }
    tokenizer->cursor = cur + 1;

    *out_token = (VismutToken){
        .type = VISMUT_TOKEN_SINGLE_LINE_COMMENT,
        .position = (Position){.offset = start - tokenizer->start, .length = cur - start},
        .data.str =
            (StringView){
                .data = (u8 *)start + ((is_doc_comment) ? 3 : 2),
                .length = cur - start,
            },
    };

    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseMultiLineComment(VismutTokenizer *restrict tokenizer,
                                    VismutToken *restrict out_token) {
    const u8 *const start = tokenizer->token_start;
    const u8 *cur = tokenizer->cursor + 2; // =start + 2
    const u8 *const limit = tokenizer->limit;

    while (cur + 1 < limit) {
        if (*cur == '*' && *cur + 1 == '/') {
            break;
        }
        ++cur;
    }
    ++cur;

    tokenizer->cursor = cur;

    *out_token = (VismutToken){
        .type = VISMUT_TOKEN_MULTILINE_COMMENT,
        .position = (Position){.offset = start - tokenizer->start, .length = cur - start},
        .data.str =
            (StringView){
                .data = (u8 *)start + 2,
                .length = cur - start - 2,
            },
    };

    return VISMUT_ERROR_OK;
}

attribute_noinline attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseOperator(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token) {
    const u8 *const start = tokenizer->token_start;
    const u8 *const limit = tokenizer->limit;
    const u8 *cur = tokenizer->cursor;

    const u8 c = *start;
    const u8 next_c = cur < limit ? *cur : '\0';

    int width = 1;

    VismutTokenType type = VISMUT_TOKEN_UNKNOWN;
    switch (c) {
    case '(':
        type = VISMUT_TOKEN_LPAREN;
        break;
    case ')':
        type = VISMUT_TOKEN_RPAREN;
        break;
    case '[':
        type = VISMUT_TOKEN_LBRACKET;
        break;
    case ']':
        type = VISMUT_TOKEN_RBRACKET;
        break;
    case '{':
        type = VISMUT_TOKEN_LBRACE;
        break;
    case '}':
        type = VISMUT_TOKEN_RBRACE;
        break;
    case ';':
        type = VISMUT_TOKEN_SEMICOLON;
        break;
    case ',':
        type = VISMUT_TOKEN_COMMA;
        break;
    case '.':
        type = VISMUT_TOKEN_DOT;
        break;
    case '^':
        type = VISMUT_TOKEN_CIRCUMFLEX;
        break;
    case '~':
        type = VISMUT_TOKEN_TILDA;
        break;
    case '?':
        type = VISMUT_TOKEN_QUESTION_MARK;
        break;
    case '@':
        type = VISMUT_TOKEN_WHILE_STATEMENT;
        break;
    case '#':
        type = VISMUT_TOKEN_CONDITION_STATEMENT;
        break;
    case '/':
        type = VISMUT_TOKEN_SLASH;
        break;
    case '&':
        width = 2;
        switch (next_c) {
        case '&':
            type = VISMUT_TOKEN_AMPERSAND_AMPERSAND;
            break;
        default:
            type = VISMUT_TOKEN_AMPERSAND;
            width = 1;
            break;
        }
        break;

    case '|':
        width = 2;
        switch (next_c) {
        case '|':
            type = VISMUT_TOKEN_LOGICAL_OR;
            break;
        default:
            type = VISMUT_TOKEN_BITWISE_OR;
            width = 1;
            break;
        }
        break;
    case '!':
        width = 2;
        switch (next_c) {
        case '#':
            type = VISMUT_TOKEN_CONDITION_ELSE_IF;
            break;
        case '=':
            type = VISMUT_TOKEN_NOT_EQUALS;
            break;
        default:
            type = VISMUT_TOKEN_EXCLAMATION_MARK;
            width = 1;
            break;
        }
        break;
    case '+':
        width = 2;
        switch (next_c) {
        case '+':
            type = VISMUT_TOKEN_PLUS_PLUS;
            break;
        case '>':
            type = VISMUT_TOKEN_IMPORT_STATEMENT;
            break;
        default:
            type = VISMUT_TOKEN_PLUS;
            width = 1;
            break;
        }
        break;
    case '%':
        width = 2;
        switch (next_c) {
        case '%':
            type = VISMUT_TOKEN_FOR_STATEMENT;
            break;
        default:
            type = VISMUT_TOKEN_PERCENT;
            width = 1;
            break;
        }
        break;
    case '-':
        width = 2;
        switch (next_c) {
        case '-':
            type = VISMUT_TOKEN_MINUS_MINUS;
            break;
        case '>':
            type = VISMUT_TOKEN_ARROW_RIGHT;
            break;
        default:
            type = VISMUT_TOKEN_MINUS;
            width = 1;
            break;
        }
        break;
    case '=':
        width = 2;
        switch (next_c) {
        case '=':
            type = VISMUT_TOKEN_EQUAL_EQUAL;
            break;
        case '>':
            type = VISMUT_TOKEN_FAT_ARROW_RIGHT;
            break;
        default:
            type = VISMUT_TOKEN_EQUAL;
            width = 1;
            break;
        }
        break;
    case '<':
        width = 2;
        switch (next_c) {
        case '+':
            type = VISMUT_TOKEN_EXPORT_STATEMENT;
            break;
        case '=':
            type = VISMUT_TOKEN_LESS_THAN_OR_EQUALS;
            break;
        case '<':
            type = VISMUT_TOKEN_SHIFT_LEFT;
            break;
        case '>':
            type = VISMUT_TOKEN_NAMESPACE_DECLARATION;
            break;
        case '&':
            type = VISMUT_TOKEN_STRUCTURE_DECLARATION;
            break;
        default:
            type = VISMUT_TOKEN_LANGLE;
            width = 1;
            break;
        }
        break;
    case '>':
        width = 2;
        switch (next_c) {
        case '=':
            type = VISMUT_TOKEN_GREATER_THAN_OR_EQUALS;
            break;
        case '>':
            type = VISMUT_TOKEN_SHIFT_RIGHT;
            break;
        default:
            type = VISMUT_TOKEN_RANGLE;
            width = 1;
            break;
        }
        break;
    case '*':
        width = 2;
        switch (next_c) {
        case '*':
            type = VISMUT_TOKEN_STAR_STAR;
            break;
        default:
            type = VISMUT_TOKEN_STAR;
            width = 1;
            break;
        }
        break;
    case '$':
        width = 2;
        switch (next_c) {
        case ':':
            type = VISMUT_TOKEN_INPUT_STATEMENT;
            break;
        case '>':
            type = VISMUT_TOKEN_STRUCTURE_DECLARATION;
            break;
        default:
            type = VISMUT_TOKEN_NAME_DECLARATION;
            width = 1;
            break;
        }
        break;
    case ':':
        width = 2;
        switch (next_c) {
        case ':':
            type = VISMUT_TOKEN_PRINT_STATEMENT;
            break;
        default:
            type = VISMUT_TOKEN_COLON;
            width = 1;
            break;
        }
        break;
    default:
        break;
    }

    cur += width - 1;
    tokenizer->cursor = cur;

    *out_token = (VismutToken){.type = type,
                               .position = {
                                   .length = width,
                                   .offset = cur - start,
                               }};

    return VISMUT_ERROR_OK;
}

attribute_const static inline u32 HexCharToInt(const u8 c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseString(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token) {
    const u8 *const start = tokenizer->token_start;
    const u8 *cur = tokenizer->cursor;
    const u8 *const limit = tokenizer->limit;

    int has_escapes = 0;
    const u8 *fast_cur = cur;

    while (likely(fast_cur < limit)) {
        if (*fast_cur == '"')
            break;
        if (unlikely(*fast_cur == '\\'))
            has_escapes = 1;
        fast_cur++;
    }

    if (unlikely(fast_cur >= limit)) {
        return VISMUT_ERROR_UNTERMINATED_STRING;
    }

    const u32 raw_length = fast_cur - cur;
    const u32 token_length = (fast_cur - start) + 1;

    if (!has_escapes) {
        tokenizer->cursor = fast_cur + 1;
        *out_token = (VismutToken){
            .type = VISMUT_TOKEN_STR_LITERAL,
            .position = (Position){.offset = start - tokenizer->start, .length = token_length},
            .data.str = (StringView){.data = (u8 *)cur, .length = raw_length},
        };
        return VISMUT_ERROR_OK;
    }

    u8 *unescaped_buffer = Arena_Array(tokenizer->arena, u8, raw_length);
    if (!unescaped_buffer)
        return VISMUT_ERROR_OUT_OF_MEMORY;

    u32 out_len = 0;
    while (cur < fast_cur) {
        if (*cur == '\\') {
            cur++;
            switch (*cur) {
            case 'n':
                unescaped_buffer[out_len++] = '\n';
                cur++;
                break;
            case 'r':
                unescaped_buffer[out_len++] = '\r';
                cur++;
                break;
            case 't':
                unescaped_buffer[out_len++] = '\t';
                cur++;
                break;
            case '\\':
                unescaped_buffer[out_len++] = '\\';
                cur++;
                break;
            case '"':
                unescaped_buffer[out_len++] = '"';
                cur++;
                break;
            case '0':
                unescaped_buffer[out_len++] = '\0';
                cur++;
                break;
            case 'x': { // \xNN
                if (cur + 2 >= fast_cur)
                    return VISMUT_ERROR_INVALID_STRING_ESCAPE;
                unescaped_buffer[out_len++] = (HexCharToInt(cur[1]) << 4) | HexCharToInt(cur[2]);
                cur += 3;
                break;
            }
            case 'u':   // \uNNNN
            case 'U': { // \UNNNNNNNN
                const int num_digits = (*cur == 'u') ? 4 : 8;
                cur++;
                if (cur + num_digits > fast_cur)
                    return VISMUT_ERROR_INVALID_STRING_ESCAPE;

                u32 codepoint = 0;
                for (int i = 0; i < num_digits; i++) {
                    // Используем IsHexDigit из твоего кода
                    if (!IsHexDigit(cur[i]))
                        return VISMUT_ERROR_INVALID_STRING_ESCAPE;
                    codepoint = (codepoint << 4) | HexCharToInt(cur[i]);
                }
                cur += num_digits;

                // Кодирование кодовой точки Unicode в UTF-8
                if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
                    return VISMUT_ERROR_INVALID_STRING_ESCAPE;
                } else if (codepoint <= 0x7F) {
                    unescaped_buffer[out_len++] = codepoint;
                } else if (codepoint <= 0x7FF) {
                    unescaped_buffer[out_len++] = 0xC0 | (codepoint >> 6);
                    unescaped_buffer[out_len++] = 0x80 | (codepoint & 0x3F);
                } else if (codepoint <= 0xFFFF) {
                    unescaped_buffer[out_len++] = 0xE0 | (codepoint >> 12);
                    unescaped_buffer[out_len++] = 0x80 | ((codepoint >> 6) & 0x3F);
                    unescaped_buffer[out_len++] = 0x80 | (codepoint & 0x3F);
                } else if (codepoint <= 0x10FFFF) {
                    unescaped_buffer[out_len++] = 0xF0 | (codepoint >> 18);
                    unescaped_buffer[out_len++] = 0x80 | ((codepoint >> 12) & 0x3F);
                    unescaped_buffer[out_len++] = 0x80 | ((codepoint >> 6) & 0x3F);
                    unescaped_buffer[out_len++] = 0x80 | (codepoint & 0x3F);
                } else {
                    return VISMUT_ERROR_INVALID_STRING_ESCAPE;
                }
                break;
            }
            default:
                return VISMUT_ERROR_INVALID_STRING_ESCAPE;
            }
        } else {
            unescaped_buffer[out_len++] = *cur++;
        }
    }

    tokenizer->cursor = fast_cur + 1;
    *out_token = (VismutToken){
        .type = VISMUT_TOKEN_STR_LITERAL,
        .position = (Position){.offset = start - tokenizer->start, .length = token_length},
        .data.str = (StringView){.data = unescaped_buffer, .length = out_len},
    };

    return VISMUT_ERROR_OK;
}

attribute_pure attribute_nonnull(1) attribute_nodiscard static VismutTokenType
    Tokenizer_ParseKeyword3(const u8 buffer[static 3]) {
#define PACK_TOKEN3(a, b, c) ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16))
    u32 token_from_buffer = 0;

    Vismut_MemoryCopy((void *)&token_from_buffer, buffer, 3);
    token_from_buffer &= 0xFFFFFF;

    switch (token_from_buffer) {
    case PACK_TOKEN3('i', '6', '4'):
        return VISMUT_TOKEN_I64_TYPE;
    case PACK_TOKEN3('i', '3', '2'):
        return VISMUT_TOKEN_I32_TYPE;
    case PACK_TOKEN3('i', '1', '6'):
        return VISMUT_TOKEN_I16_TYPE;
    case PACK_TOKEN3('u', '6', '4'):
        return VISMUT_TOKEN_U64_TYPE;
    case PACK_TOKEN3('u', '3', '2'):
        return VISMUT_TOKEN_U32_TYPE;
    case PACK_TOKEN3('u', '1', '6'):
        return VISMUT_TOKEN_I16_TYPE;
    case PACK_TOKEN3('f', '6', '4'):
        return VISMUT_TOKEN_F64_TYPE;
    case PACK_TOKEN3('f', '3', '2'):
        return VISMUT_TOKEN_F32_TYPE;
    case PACK_TOKEN3('s', 't', 'r'):
        return VISMUT_TOKEN_STR_TYPE;
    default:
        return VISMUT_TOKEN_UNKNOWN;
    }

#undef PACK_TOKEN3
}

attribute_pure attribute_nonnull(1) attribute_nodiscard static VismutTokenType
    Tokenizer_ParseKeyword2(const u8 buffer[static 2]) {
#define PACK_TOKEN2(a, b) ((u32)(a) | ((u32)(b) << 8))
    u32 token_from_buffer = 0;

    Vismut_MemoryCopy((void *)&token_from_buffer, buffer, 2);
    token_from_buffer &= 0xFFFF;

    switch (token_from_buffer) {
    case PACK_TOKEN2('i', '8'):
        return VISMUT_TOKEN_I8_TYPE;
    case PACK_TOKEN2('u', '8'):
        return VISMUT_TOKEN_U8_TYPE;
    default:
        return VISMUT_TOKEN_UNKNOWN;
    }

#undef PACK_TOKEN2
}

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseIdentifier(VismutTokenizer *restrict tokenizer,
                              VismutToken *restrict out_token) {
    const u8 *const token_start = tokenizer->token_start;
    const u8 *cur = tokenizer->cursor;
    while (IsValidIdentifier(*cur))
        ++cur;
    const u32 length = cur - token_start;

    tokenizer->cursor = cur;

    VismutTokenType keyword = VISMUT_TOKEN_UNKNOWN;
    if (length == 3 && (keyword = Tokenizer_ParseKeyword3(token_start)) != VISMUT_TOKEN_UNKNOWN) {
        *out_token = (VismutToken){
            .type = keyword,
            .position = (Position){.offset = token_start - tokenizer->start, .length = length},
        };
        return VISMUT_ERROR_OK;
    }
    if (length == 2 && (keyword = Tokenizer_ParseKeyword2(token_start)) != VISMUT_TOKEN_UNKNOWN) {
        *out_token = (VismutToken){
            .type = keyword,
            .position = (Position){.offset = token_start - tokenizer->start, .length = length},
        };
        return VISMUT_ERROR_OK;
    }

    *out_token = (VismutToken){
        .type = VISMUT_TOKEN_IDENTIFIER,
        .position = (Position){.offset = token_start - tokenizer->start, .length = length},
        .data.str =
            (StringView){
                .data = (u8 *)token_start,
                .length = length,
            },
    };

    return VISMUT_ERROR_OK;
}

// Use IsValidIdentifier function (may give "1name" or "Hdss_2")
static StringView Tokenizer_ScanIdentifier(VismutTokenizer *restrict tokenizer) {
    const u8 *cur = tokenizer->cursor;
    const u8 *const keyword_start = cur;
    const u8 *const limit = tokenizer->limit;

    while (cur < limit && IsValidIdentifier(*cur)) {
        ++cur;
    }

    return (StringView){
        .data = (u8 *)keyword_start,
        .length = cur - keyword_start,
    };
}

static VismutErrorType Tokenizer_ParseIntSuffix(VismutTokenizer *restrict tokenizer,
                                                VismutIntSuffix *restrict out_suffix) {
    StringView suffix_str = Tokenizer_ScanIdentifier(tokenizer);
    VismutIntSuffix suffix = ParseIntSuffix(suffix_str);
    if (suffix == VISMUT_INT_SUFFIX_UNKNOWN)
        return VISMUT_ERROR_NUMBER_INVALID_SUFFIX;

    *out_suffix = suffix;
    tokenizer->cursor = suffix_str.data + suffix_str.length;

    return VISMUT_ERROR_OK;
}

static VismutErrorType Tokenizer_ParseFloatSuffix(VismutTokenizer *restrict tokenizer,
                                                  VismutFloatSuffix *restrict out_suffix) {
    StringView suffix_str = Tokenizer_ScanIdentifier(tokenizer);
    VismutFloatSuffix suffix = ParseFloatSuffix(suffix_str);
    if (suffix == VISMUT_FLOAT_SUFFIX_UNKNOWN)
        return VISMUT_ERROR_NUMBER_INVALID_SUFFIX;

    *out_suffix = suffix;
    tokenizer->cursor = suffix_str.data + suffix_str.length;

    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2) attribute_nodiscard static VismutErrorType
    Tokenizer_ParseNumber(VismutTokenizer *restrict tokenizer, VismutToken *restrict out_token) {
    VismutErrorType err;

    const u8 *const start = tokenizer->token_start;
    const u8 *cur = tokenizer->cursor;
    const u8 *const limit = tokenizer->limit;

    typedef enum {
        NB_HEX,
        NB_DEC,
        NB_OCT,
        NB_BIN,
    } NumberBase;

    NumberBase base = NB_DEC;
    if (*start == '0' && cur < limit) {
        const u8 c = *cur & 0xDF; // To upper
        switch (c) {
        case 'X':
            base = NB_HEX;
            ++cur;
            break;
        case 'B':
            base = NB_BIN;
            ++cur;
            break;
        case 'O':
            base = NB_OCT;
            ++cur;
            break;
        default:
            break;
        }
    }

    VismutIntSuffix int_suffix = VISMUT_INT_SUFFIX_NONE;
    VismutFloatSuffix float_suffix = VISMUT_FLOAT_SUFFIX_NONE;

    int is_float = 0;
    switch (base) {
    case NB_HEX:
        while (likely(cur < limit)) {
            const u8 c = (*cur);
            if (IsHexDigit(c)) {
                ++cur;
            } else if (c == 'i' || c == 'u') {
                tokenizer->cursor = cur;
                SAFE_RISKY_EXPRESSION(Tokenizer_ParseIntSuffix(tokenizer, &int_suffix), err);
                break;
            } else if (IsValidIdentifier(c)) {
                return VISMUT_ERROR_PARSING_NUMBER;
            } else {
                break;
            }
        }
        break;
    case NB_OCT:
        while (likely(cur < limit)) {
            const u8 c = *cur;
            if (IsOctDigit(c)) {
                ++cur;
            } else if (c == 'i' || c == 'u') {
                tokenizer->cursor = cur;
                SAFE_RISKY_EXPRESSION(Tokenizer_ParseIntSuffix(tokenizer, &int_suffix), err);
                break;
            } else if (IsValidIdentifier(c)) {
                return VISMUT_ERROR_PARSING_NUMBER;
            } else {
                break;
            }
        }
        break;
    case NB_BIN:
        while (likely(cur < limit)) {
            const u8 c = *cur;
            if (IsBinDigit(c)) {
                ++cur;
            } else if (c == 'i' || c == 'u') {
                tokenizer->cursor = cur;
                SAFE_RISKY_EXPRESSION(Tokenizer_ParseIntSuffix(tokenizer, &int_suffix), err);
                break;
            } else if (IsValidIdentifier(c)) {
                return VISMUT_ERROR_PARSING_NUMBER;
            } else {
                break;
            }
        }
        break;
    case NB_DEC: {
        int has_dot = 0;
        while (likely(cur < limit)) {
            const uint8_t c = *cur;
            if (IsDecDigit(c)) {
                cur++;
                continue;
            }
            if (c == '.' && !has_dot) {
                cur++;
                has_dot = 1;
                is_float = 1;
                continue;
            }
            if ((c & 0xDF) == 'E') {
                cur++;
                is_float = 1;
                if (cur < limit && (*cur == '+' || *cur == '-'))
                    cur++;
                while (cur < limit && IsDecDigit(*cur))
                    ++cur;
                break;
            }
            if ((c == 'i' || c == 'u') && !is_float) {
                tokenizer->cursor = cur;
                SAFE_RISKY_EXPRESSION(Tokenizer_ParseIntSuffix(tokenizer, &int_suffix), err);
                break;
            }
            if (c == 'f') {
                tokenizer->cursor = cur;
                SAFE_RISKY_EXPRESSION(Tokenizer_ParseFloatSuffix(tokenizer, &float_suffix), err);
                is_float = 1;
                break;
            }
            if (IsValidIdentifier(c)) {
                return VISMUT_ERROR_NUMBER_INVALID_SUFFIX;
            }
            break;
        }
    }
    }

    const u64 length = cur - start;
    const u32 suffix_len = (tokenizer->cursor > cur) ? tokenizer->cursor - cur : 0;

    tokenizer->cursor = cur + suffix_len;
    const Position pos = (Position){
        .offset = tokenizer->token_start - tokenizer->start,
        .length = length + suffix_len,
    };

    if (is_float) {
        double number;
        VismutErrorType err;

        SAFE_RISKY_EXPRESSION(VismutConvert_StrToDouble((StringView){(u8 *)start, length}, &number),
                              err);

        *out_token = (VismutToken){
            .type = VISMUT_TOKEN_FLOAT_LITERAL,
            .position = pos,
            .data.f =
                {
                    .value = number,
                    .suffix = float_suffix,
                },
        };

        return VISMUT_ERROR_OK;
    }

    int64_t val = 0;
    const StringView string_buffer = (StringView){
        .data = (u8 *)(base == NB_DEC ? start : start + 2),
        .length = (base == NB_DEC) ? length : length - 2,
    };

    switch (base) {
    case NB_HEX:
        val = VismutConvert_HexStrToUInt(string_buffer);
        break;
    case NB_DEC:
        val = VismutConvert_DecStrToUInt(string_buffer);
        break;
    case NB_OCT:
        val = VismutConvert_OctStrToUInt(string_buffer);
        break;
    case NB_BIN:
        val = VismutConvert_BinStrToUInt(string_buffer);
        break;
    default:
        assert("Unreachable!");
        break;
    }

    *out_token = (VismutToken){
        .type = VISMUT_TOKEN_INT_LITERAL,
        .position = pos,
        .data.i =
            {
                .value = val,
                .suffix = int_suffix,
            },
    };

    return VISMUT_ERROR_OK;
}

attribute_nonnull(1) attribute_hot
    static void Tokenizer_SkipWhitespace(VismutTokenizer *tokenizer) {
    const u8 *curr = tokenizer->cursor;
    const u8 *limit = tokenizer->limit;

    if (limit - curr >= 16) {
        const __m128i v_space = _mm_set1_epi8(' ');
        while (curr + 16 <= limit) {
            const __m128i chunk = _mm_loadu_si128((const __m128i *)curr);

            const __m128i eq_space = _mm_cmpeq_epi8(chunk, v_space);
            const __m128i gt_lower =
                _mm_cmpgt_epi8(chunk, _mm_set1_epi8(0x08)); // > 0x08 ( >= 0x09)
            const __m128i lt_upper =
                _mm_cmplt_epi8(chunk, _mm_set1_epi8(0x0E)); // < 0x0E ( <= 0x0D)
            const __m128i in_range = _mm_and_si128(gt_lower, lt_upper);
            const __m128i is_white = _mm_or_si128(eq_space, in_range);

            const int mask = _mm_movemask_epi8(is_white);

            if (mask == 0xFFFF) {
                curr += 16;
            } else {
                const int not_space_mask = ~mask & 0xFFFF;
                const int offset = __builtin_ctz(not_space_mask);
                curr += offset;
                goto done;
            }
        }
    }

    while (curr < limit && IsSpace(*curr)) {
        curr++;
    }

done:
    tokenizer->cursor = curr;
}
