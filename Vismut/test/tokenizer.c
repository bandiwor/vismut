#include "../core/tokenizer/tokenizer.h"
#include "vismut_test.h"

static VismutTokenizer SetupTokenizer(Arena *arena, VismutErrorInfo *err, const char *src) {
    return VismutTokenizer_Create(StringView_FromCStr(src), arena, err);
}

#define AS_INT(token) (token).data.i.value
#define AS_INT_SUFFIX(token) (token).data.i.suffix
#define AS_FLOAT(token) (token).data.f.value
#define AS_FLOAT_SUFFIX(token) (token).data.f.suffix
#define AS_SV(token) (token).data.str

TEST(Tokenizer, Operators) {
#define EXPECT_OP(expected_type)                                                                   \
    do {                                                                                           \
        EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                  \
        EXPECT_TOKEN_TYPE(expected_type, token);                                                   \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src = "$ # !# ! ? @ % %% <> $> ^ :: $: +> <+ + - * ** / -> => . , ; : { } [ "
                      "] ( ) < <= > >= = == | || & && ^ ~ << >> !=";

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_OP(VISMUT_TOKEN_NAME_DECLARATION);       // $
    EXPECT_OP(VISMUT_TOKEN_CONDITION_STATEMENT);    // #
    EXPECT_OP(VISMUT_TOKEN_CONDITION_ELSE_IF);      // !#
    EXPECT_OP(VISMUT_TOKEN_EXCLAMATION_MARK);       // !
    EXPECT_OP(VISMUT_TOKEN_QUESTION_MARK);          // ?
    EXPECT_OP(VISMUT_TOKEN_WHILE_STATEMENT);        // @
    EXPECT_OP(VISMUT_TOKEN_PERCENT);                // %
    EXPECT_OP(VISMUT_TOKEN_FOR_STATEMENT);          // %%
    EXPECT_OP(VISMUT_TOKEN_NAMESPACE_DECLARATION);  // <>
    EXPECT_OP(VISMUT_TOKEN_STRUCTURE_DECLARATION);  // $>
    EXPECT_OP(VISMUT_TOKEN_CIRCUMFLEX);             // ^
    EXPECT_OP(VISMUT_TOKEN_PRINT_STATEMENT);        // ::
    EXPECT_OP(VISMUT_TOKEN_INPUT_STATEMENT);        // $:
    EXPECT_OP(VISMUT_TOKEN_IMPORT_STATEMENT);       // +>
    EXPECT_OP(VISMUT_TOKEN_EXPORT_STATEMENT);       // <+
    EXPECT_OP(VISMUT_TOKEN_PLUS);                   // +
    EXPECT_OP(VISMUT_TOKEN_MINUS);                  // -
    EXPECT_OP(VISMUT_TOKEN_STAR);                   // *
    EXPECT_OP(VISMUT_TOKEN_STAR_STAR);              // **
    EXPECT_OP(VISMUT_TOKEN_SLASH);                  // /
    EXPECT_OP(VISMUT_TOKEN_ARROW_RIGHT);            // ->
    EXPECT_OP(VISMUT_TOKEN_FAT_ARROW_RIGHT);        // =>
    EXPECT_OP(VISMUT_TOKEN_DOT);                    // .
    EXPECT_OP(VISMUT_TOKEN_COMMA);                  // ,
    EXPECT_OP(VISMUT_TOKEN_SEMICOLON);              // ;
    EXPECT_OP(VISMUT_TOKEN_COLON);                  // :
    EXPECT_OP(VISMUT_TOKEN_LBRACE);                 // {
    EXPECT_OP(VISMUT_TOKEN_RBRACE);                 // }
    EXPECT_OP(VISMUT_TOKEN_LBRACKET);               // [
    EXPECT_OP(VISMUT_TOKEN_RBRACKET);               // ]
    EXPECT_OP(VISMUT_TOKEN_LPAREN);                 // (
    EXPECT_OP(VISMUT_TOKEN_RPAREN);                 // )
    EXPECT_OP(VISMUT_TOKEN_LANGLE);                 // <
    EXPECT_OP(VISMUT_TOKEN_LESS_THAN_OR_EQUALS);    // <=
    EXPECT_OP(VISMUT_TOKEN_RANGLE);                 // >
    EXPECT_OP(VISMUT_TOKEN_GREATER_THAN_OR_EQUALS); // >=
    EXPECT_OP(VISMUT_TOKEN_EQUAL);                  // =
    EXPECT_OP(VISMUT_TOKEN_EQUAL_EQUAL);            // ==
    EXPECT_OP(VISMUT_TOKEN_BITWISE_OR);             // |
    EXPECT_OP(VISMUT_TOKEN_LOGICAL_OR);             // ||
    EXPECT_OP(VISMUT_TOKEN_AMPERSAND);              // &
    EXPECT_OP(VISMUT_TOKEN_AMPERSAND_AMPERSAND);    // &&
    EXPECT_OP(VISMUT_TOKEN_CIRCUMFLEX);             // ^
    EXPECT_OP(VISMUT_TOKEN_TILDA);                  // ~
    EXPECT_OP(VISMUT_TOKEN_SHIFT_LEFT);             // <<
    EXPECT_OP(VISMUT_TOKEN_SHIFT_RIGHT);            // >>
    EXPECT_OP(VISMUT_TOKEN_NOT_EQUALS);             // !=
    EXPECT_OP(VISMUT_TOKEN_EOF);                    // EOF

    Arena_Destroy(&arena);

#undef EXPECT_OP
}

TEST(Tokenizer, IntegersAndSuffixes) {
#define EXPECT_INT_LITERAL(expected_val, expected_suffix)                                          \
    do {                                                                                           \
        EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                  \
        EXPECT_TOKEN_TYPE(VISMUT_TOKEN_INT_LITERAL, token);                                        \
        EXPECT_EQ_INT((expected_val), AS_INT(token));                                              \
        EXPECT_EQ_INT((expected_suffix), AS_INT_SUFFIX(token));                                    \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src = "42 0xFF 0b1010 0o77 5i8 90i16 37i32 98i64 11u8 1024u16 123u32 0u64";
    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_INT_LITERAL(42, VISMUT_INT_SUFFIX_NONE); // 42

    EXPECT_INT_LITERAL(255, VISMUT_INT_SUFFIX_NONE); // 0xFF
    EXPECT_INT_LITERAL(10, VISMUT_INT_SUFFIX_NONE);  // 0b1010
    EXPECT_INT_LITERAL(63, VISMUT_INT_SUFFIX_NONE);  // 0o77

    EXPECT_INT_LITERAL(5, VISMUT_INT_SUFFIX_I8);   // 5i8
    EXPECT_INT_LITERAL(90, VISMUT_INT_SUFFIX_I16); // 90i16
    EXPECT_INT_LITERAL(37, VISMUT_INT_SUFFIX_I32); // 37i32
    EXPECT_INT_LITERAL(98, VISMUT_INT_SUFFIX_I64); // 98i64

    EXPECT_INT_LITERAL(11, VISMUT_INT_SUFFIX_U8);    // 11u8
    EXPECT_INT_LITERAL(1024, VISMUT_INT_SUFFIX_U16); // 1024u16
    EXPECT_INT_LITERAL(123, VISMUT_INT_SUFFIX_U32);  // 123u32
    EXPECT_INT_LITERAL(0, VISMUT_INT_SUFFIX_U64);    // 0u64

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EOF, token);

    Arena_Destroy(&arena);

#undef EXPECT_INT_LITERAL
}

TEST(Tokenizer, InvalidIntegers) {
#define EXPECT_INT_ERROR(status, str)                                                              \
    do {                                                                                           \
        tokenizer = SetupTokenizer(&arena, &info, str);                                            \
        EXPECT_FAIL_CODE(status, VismutTokenizer_Next(&tokenizer, &token));                        \
        EXPECT_FAIL_CODE(status, info.type);                                                       \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};
    VismutTokenizer tokenizer;
    VismutToken token;

    EXPECT_INT_ERROR(VISMUT_ERR_NUM_PARSE, "0x");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_PARSE, "0b");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_PARSE, "0o");

    EXPECT_INT_ERROR(VISMUT_ERR_NUM_PARSE, "0b1012");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_PARSE, "0o778");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_PARSE, "0xDEADG00D");

    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "99999999999999999999");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "0xFFFFFFFFFFFFFFFFF"); // 17 letters F

    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "2i1");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "129i8");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "256u8");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "32769i16");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "65536u16");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "2147483649i32");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "4294967296u32");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "9223372036854775809i64");
    EXPECT_INT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "18446744073709551616u64");

    EXPECT_INT_ERROR(VISMUT_ERR_NUM_BAD_SUFFIX, "123i99");

    Arena_Destroy(&arena);
#undef EXPECT_INT_ERROR
}

TEST(Tokenizer, FloatsAndSuffixes) {
#define EXPECT_FLOAT_LITERAL(expected_val, expected_suffix)                                        \
    do {                                                                                           \
        EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                  \
        EXPECT_TOKEN_TYPE(VISMUT_TOKEN_FLOAT_LITERAL, token);                                      \
        EXPECT_EQ_FLOAT((expected_val), AS_FLOAT(token));                                          \
        EXPECT_EQ_INT((expected_suffix), AS_FLOAT_SUFFIX(token));                                  \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src = "0.0 3.14159 0.5 123.456 "
                      "1e3 1.5e-2 2E+4 0.001e-3 "
                      "3.14f32 2.718f64 1e5f32 0.1E-2f64";

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_FLOAT_LITERAL(0.0, VISMUT_FLOAT_SUFFIX_NONE);     // 0.0
    EXPECT_FLOAT_LITERAL(3.14159, VISMUT_FLOAT_SUFFIX_NONE); // 3.14159
    EXPECT_FLOAT_LITERAL(0.5, VISMUT_FLOAT_SUFFIX_NONE);     // 0.5
    EXPECT_FLOAT_LITERAL(123.456, VISMUT_FLOAT_SUFFIX_NONE); // 123.456

    EXPECT_FLOAT_LITERAL(1000.0, VISMUT_FLOAT_SUFFIX_NONE);   // 1e3 (1 * 10^3)
    EXPECT_FLOAT_LITERAL(0.015, VISMUT_FLOAT_SUFFIX_NONE);    // 1.5e-2 (1.5 * 10^-2)
    EXPECT_FLOAT_LITERAL(20000.0, VISMUT_FLOAT_SUFFIX_NONE);  // 2E+4
    EXPECT_FLOAT_LITERAL(0.000001, VISMUT_FLOAT_SUFFIX_NONE); // 0.001e-3

    EXPECT_FLOAT_LITERAL(3.14, VISMUT_FLOAT_SUFFIX_F32);     // 3.14f32
    EXPECT_FLOAT_LITERAL(2.718, VISMUT_FLOAT_SUFFIX_F64);    // 2.718f64
    EXPECT_FLOAT_LITERAL(100000.0, VISMUT_FLOAT_SUFFIX_F32); // 1e5f32
    EXPECT_FLOAT_LITERAL(0.001, VISMUT_FLOAT_SUFFIX_F64);    // 0.1E-2f64

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EOF, token);

#undef EXPECT_FLOAT_LITERAL
}

TEST(Tokenizer, InvalidFloats) {
#define EXPECT_FLOAT_ERROR(status, str)                                                            \
    do {                                                                                           \
        tokenizer = SetupTokenizer(&arena, &info, str);                                            \
        EXPECT_FAIL_CODE(status, VismutTokenizer_Next(&tokenizer, &token));                        \
        EXPECT_FAIL_CODE(status, info.type);                                                       \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};
    VismutTokenizer tokenizer;
    VismutToken token;

    EXPECT_FLOAT_ERROR(VISMUT_ERR_NUM_PARSE, "1e");
    EXPECT_FLOAT_ERROR(VISMUT_ERR_NUM_PARSE, "1.5e+");
    EXPECT_FLOAT_ERROR(VISMUT_ERR_NUM_PARSE, "2E-");

    EXPECT_FLOAT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "1e315");
    EXPECT_FLOAT_ERROR(VISMUT_ERR_NUM_OVERFLOW, "2.5e400");

    Arena_Destroy(&arena);
#undef EXPECT_FLOAT_ERROR
}

TEST(Tokenizer, ValidStringsAndEscapes) {
#define EXPECT_VALID_STRING(expected_str)                                                          \
    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                      \
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_STR_LITERAL, token);                                            \
    EXPECT_SV_EQ(expected_str, AS_SV(token));

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src =
        "\"fast_path\" "
        "\"line1\\nline2\\t\\\\\\\"\" "          // line1\nline2\t\"
        "\"hex \\x41\\x7E\" "                    // \x41 = 'A', \x7E = '~'
        "\"utf8 \\u041f\\u0438\\u0432\\u043e\" " // \u041f... = Пиво (Cyrillic, 2 bytes per char)
        "\"emoji \\U0001F680\"";                 // \U0001F680 = 🚀 (Rocket, 4 bytes)

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_VALID_STRING("fast_path");
    EXPECT_VALID_STRING("line1\nline2\t\\\"");
    EXPECT_VALID_STRING("hex A~");
    EXPECT_VALID_STRING("utf8 Пиво");
    EXPECT_VALID_STRING("emoji 🚀");

    VismutTokenizer t_null = SetupTokenizer(&arena, &info, "\"null\\0byte\"");
    EXPECT_SUCCESS(VismutTokenizer_Next(&t_null, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_STR_LITERAL, token);
    EXPECT_EQ_INT(9, AS_SV(token).length);
    EXPECT_EQ_INT('l', AS_SV(token).data[3]);
    EXPECT_EQ_INT('\0', AS_SV(token).data[4]);
    EXPECT_EQ_INT('b', AS_SV(token).data[5]);

    Arena_Destroy(&arena);

#undef EXPECT_VALID_STRING
}

TEST(Tokenizer, StringErrors) {
#define EXPECT_STRING_ERROR(status, str)                                                           \
    tokenizer = SetupTokenizer(&arena, &info, str);                                                \
    EXPECT_FAIL_CODE(status, VismutTokenizer_Next(&tokenizer, &token));                            \
    EXPECT_FAIL_CODE(status, info.type);

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};
    VismutTokenizer tokenizer;
    VismutToken token;

    EXPECT_STRING_ERROR(VISMUT_ERR_UNTERMINATED_STRING, "\"lorem ipsum");
    EXPECT_STRING_ERROR(VISMUT_ERR_UNTERMINATED_STRING, "\"hard escape\\\"");
    EXPECT_STRING_ERROR(VISMUT_ERR_BAD_ESCAPE, "\"bad \\x4G hex\"");
    EXPECT_STRING_ERROR(VISMUT_ERR_UNTERMINATED_STRING, "\"bad \\x");
    EXPECT_STRING_ERROR(VISMUT_ERR_BAD_ESCAPE, "\"bad \\u12\"");
    EXPECT_STRING_ERROR(VISMUT_ERR_INVALID_UNICODE, "\"surrogate \\uD800\"");
    EXPECT_STRING_ERROR(VISMUT_ERR_INVALID_UNICODE, "\"too big \\U00120000\"");

    Arena_Destroy(&arena);

#undef EXPECT_STRING_ERROR
}

TEST(Tokenizer, KeywordsAndIdentifiers) {
#define EXPECT_KEYWORD(expected_type)                                                              \
    do {                                                                                           \
        EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                  \
        EXPECT_TOKEN_TYPE((expected_type), token);                                                 \
    } while (0)

#define EXPECT_IDENT(expected_str)                                                                 \
    do {                                                                                           \
        EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                  \
        EXPECT_TOKEN_TYPE(VISMUT_TOKEN_IDENTIFIER, token);                                         \
        EXPECT_SV_EQ((expected_str), token.data.str);                                              \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src = "i1 i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 str "
                      "my_var _hidden camelCase var123 i32_suffix";

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_KEYWORD(VISMUT_TOKEN_I1_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_I8_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_I16_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_I32_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_I64_TYPE);

    EXPECT_KEYWORD(VISMUT_TOKEN_U8_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_U16_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_U32_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_U64_TYPE);

    EXPECT_KEYWORD(VISMUT_TOKEN_F32_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_F64_TYPE);
    EXPECT_KEYWORD(VISMUT_TOKEN_STR_TYPE);

    EXPECT_IDENT("my_var");
    EXPECT_IDENT("_hidden");
    EXPECT_IDENT("camelCase");
    EXPECT_IDENT("var123");
    EXPECT_IDENT("i32_suffix");

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EOF, token);

    Arena_Destroy(&arena);

#undef EXPECT_KEYWORD
#undef EXPECT_IDENT
}

TEST(Tokenizer, ValidComments) {
#define EXPECT_VALID_COMMENT(type, content)                                                        \
    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));                                      \
    EXPECT_TOKEN_TYPE(type, token);                                                                \
    EXPECT_SV_EQ(content, AS_SV(token));

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src = "// simple comment\n"
                      "/// doc-string for FN\n"
                      "/* multiline\n"
                      "   comment */\n"
                      "x "
                      "// comment at the and without \\n!";

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_VALID_COMMENT(VISMUT_TOKEN_SINGLE_LINE_COMMENT, " simple comment");
    EXPECT_VALID_COMMENT(VISMUT_TOKEN_DOC_COMMENT, " doc-string for FN");
    EXPECT_VALID_COMMENT(VISMUT_TOKEN_MULTILINE_COMMENT, " multiline\n   comment ");

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_IDENTIFIER, token);
    EXPECT_SV_EQ("x", AS_SV(token));

    EXPECT_VALID_COMMENT(VISMUT_TOKEN_SINGLE_LINE_COMMENT, " comment at the and without \\n!");

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EOF, token);

    Arena_Destroy(&arena);
#undef EXPECT_VALID_COMMENT
}

TEST(Tokenizer, CommentErrors) {
#define EXPECT_COMMENT_ERROR(status, str)                                                          \
    do {                                                                                           \
        tokenizer = SetupTokenizer(&arena, &info, str);                                            \
        EXPECT_FAIL_CODE(status, VismutTokenizer_Next(&tokenizer, &token));                        \
        EXPECT_FAIL_CODE(status, info.type);                                                       \
    } while (0)

    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};
    VismutTokenizer tokenizer;
    VismutToken token;

    EXPECT_COMMENT_ERROR(VISMUT_ERR_UNTERMINATED_COMMENT, "/* this comment was not closed...");
    EXPECT_COMMENT_ERROR(VISMUT_ERR_UNTERMINATED_COMMENT, "/* hihihi *");

    Arena_Destroy(&arena);
#undef EXPECT_COMMENT_ERROR
}

TEST(Tokenizer, PositionsAndOffsets) {
    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    // let x = 42;
    // 01234567890123
    const char *src = "let x = 42;";

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    // "let"
    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_EQ_INT(0, token.position.offset);
    EXPECT_EQ_INT(3, token.position.length);

    // "x"
    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_EQ_INT(4, token.position.offset);
    EXPECT_EQ_INT(1, token.position.length);

    // "="
    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_EQ_INT(6, token.position.offset);
    EXPECT_EQ_INT(1, token.position.length);

    // "42"
    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_EQ_INT(8, token.position.offset);
    EXPECT_EQ_INT(2, token.position.length);

    Arena_Destroy(&arena);
}

TEST(Tokenizer, EmptyAndWhitespace) {
    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};
    VismutToken token;

    VismutTokenizer t1 = SetupTokenizer(&arena, &info, "");
    EXPECT_SUCCESS(VismutTokenizer_Next(&t1, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EOF, token);

    VismutTokenizer t2 = SetupTokenizer(&arena, &info, "   \t\n  \r\n \t ");
    EXPECT_SUCCESS(VismutTokenizer_Next(&t2, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EOF, token);

    Arena_Destroy(&arena);
}

TEST(Tokenizer, GluedTokens) {
    Arena arena = Arena_Create();
    VismutErrorInfo info = {0};

    const char *src = "x=-42+y";

    VismutTokenizer tokenizer = SetupTokenizer(&arena, &info, src);
    VismutToken token;

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_IDENTIFIER, token);

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_EQUAL, token);

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_MINUS, token); // Минус отдельно!

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_INT_LITERAL, token);
    EXPECT_EQ_INT(42, AS_INT(token));

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_PLUS, token);

    EXPECT_SUCCESS(VismutTokenizer_Next(&tokenizer, &token));
    EXPECT_TOKEN_TYPE(VISMUT_TOKEN_IDENTIFIER, token);

    Arena_Destroy(&arena);
}
