#include "../core/parser/ast_parser.h"
#include "../core/dumper/ast_dumper.h"
#include "vismut_test.h"
#include <stdlib.h>

typedef struct {
    Arena arena;
    VismutTypeContext type_ctx;
    SymbolsTable symbols_table;
    VismutErrorInfo err_info;
} ParserTestFixture;

static void SetupFixture(ParserTestFixture *f) {
    VismutErrorType err;
    VismutErrorDetails details;

    f->arena = Arena_Create();
    f->err_info = (VismutErrorInfo){0};

    err = VismutTypeContext_Init(&f->type_ctx, 512, &details);
    if (err != VISMUT_OK) {
        exit(1);
    }

    f->symbols_table = SymbolsTable_Create(StringView_FromCStr("test_main"));
}

static void TeardownFixture(ParserTestFixture *f) {
    Arena_Destroy(&f->arena);
}

static void ExpectAstDump(const char *src, const char *expected_dump) {
    ParserTestFixture f;
    SetupFixture(&f);

    VismutTokenizer tokenizer =
        VismutTokenizer_Create(StringView_FromCStr(src), &f.arena, &f.err_info);
    ASTBuilder builder = ASTBuilder_Create(&tokenizer);
    ASTParser parser = ASTParser_Create(&builder, &f.type_ctx, &f.symbols_table, NULL, 0);

    VismutErrorType err = ASTParser_Parse(&parser);

    if (unlikely(err != VISMUT_OK)) {
        fprintf(stderr, "Parse failed unexpectedly with error: %s at offset: %u\n",
                VismutErrorType_String(err), f.err_info.pos.offset);
        VISMUT_FAIL();
    }

    ASTDumper dumper = ASTDumper_Create(&builder, AST_DUMPER_NONE);
    StringView dump_sv = ASTDumper_Dump(&dumper, parser.module_node);

    EXPECT_SV_EQ(expected_dump, dump_sv);

    ASTDumper_Destroy(&dumper);
    TeardownFixture(&f);
}

static void ExpectParseError(const char *src, VismutErrorType expected_err) {
    ParserTestFixture f;
    SetupFixture(&f);

    VismutTokenizer tokenizer =
        VismutTokenizer_Create(StringView_FromCStr(src), &f.arena, &f.err_info);
    ASTBuilder builder = ASTBuilder_Create(&tokenizer);
    ASTParser parser = ASTParser_Create(&builder, &f.type_ctx, &f.symbols_table, NULL, 0);

    VismutErrorType actual_err = ASTParser_Parse(&parser);
    EXPECT_FAIL_CODE(expected_err, actual_err);
    EXPECT_FAIL_CODE(expected_err, f.err_info.type);

    TeardownFixture(&f);
}

TEST(Parser, LiteralsAndExpressions) {
    ExpectAstDump("42i32;", "module(lit(42))");
    ExpectAstDump("3.141592f32;", "module(lit(3.141592))");
    ExpectAstDump("\"hello\n\\n\";", "module(lit(\"hello\\n\\n\"))");
}

TEST(Parser, BinaryPrecedence) {
    ExpectAstDump("1 + 2 * 3;", "module(bin(+, lit(1), bin(*, lit(2), lit(3))))");
    ExpectAstDump("(1 + 2) * 3;", "module(bin(*, bin(+, lit(1), lit(2)), lit(3)))");
    ExpectAstDump("3 ** 2 ** 1;", "module(bin(**, lit(3), bin(**, lit(2), lit(1))))");
    ExpectAstDump(
        "3 - 2 ** 4 - 5 / 41",
        "module(bin(-, bin(-, lit(3), bin(**, lit(2), lit(4))), bin(/, lit(5), lit(41))))");
}

TEST(Parser, VariableDeclarations) {
    ExpectAstDump("$x = 10;", "module(var_decl(x, lit(10)))");
    ExpectAstDump("$y = 2.718281828;", "module(var_decl(y, lit(2.718281828)))");
    ExpectAstDump("$z!: i32 = -5;", "module(var_decl!(z, un(-, lit(5))))");
}

TEST(Parser, Assignments) {
    ExpectAstDump("x = 42;", "module(assign(`x`, =, lit(42)))");
}

TEST(Parser, Conditions) {
    ExpectAstDump("#x { 1; } !# y { 2; } ! { 3; }",
                  "module(cond(`x`, lit(1), cond(`y`, lit(2), lit(3))))");
    ExpectAstDump("#x { 1; } !# y { 2; }", "module(cond(`x`, lit(1), cond(`y`, lit(2), None)))");
    ExpectAstDump("#a > b { c }", "module(cond(bin(>, `a`, `b`), `c`, None))");
}

TEST(Parser, Loops) {
    ExpectAstDump("@x < 10 { x = x + 1; }",
                  "module(loop(bin(<, `x`, lit(10)), assign(`x`, =, bin(+, `x`, lit(1)))))");
}

TEST(Parser, FunctionDeclarations) {
    ExpectAstDump("$add(a: i32, b: i32) -> i32 { ^a + b; }",
                  "module(fn_decl(add, (a: i32, a: i32), ret(bin(+, `a`, `b`))))");
    ExpectAstDump("$foo() { 42 }", "module(fn_decl(foo, (), lit(42)))");
}

TEST(Parser, FunctionCalls) {
    ExpectAstDump("add(1, 2);", "module(`add`(lit(1), lit(2)))");
    ExpectAstDump("foo();", "module(`foo`())");
    ExpectAstDump("foo(bar(foo() + bar()));", "module(`foo`(`bar`(bin(+, `foo`(), `bar`()))))");
}

TEST(Parser, TypeCasting) {
    ExpectAstDump("i32(3.14);", "module(cast(i32, lit(3.14)))");
    ExpectAstDump("i32(3);", "module(cast(i32, lit(3)))");
    ExpectAstDump("i1(1);", "module(cast(i1, lit(1)))");
    ExpectAstDump("i64(-20);", "module(cast(i64, un(-, lit(20))))");
}

TEST(Parser, Tuples) {
    ExpectAstDump("(1, \"text\", 3.0);", "module(tuple(lit(1), lit(\"text\"), lit(3)))");
    ExpectAstDump("(1,);", "module(tuple(lit(1)))");
    ExpectAstDump("(,);", "module(unit)");
}

TEST(Parser, Dots) {
    ExpectAstDump("a.b;", "module(`a`.b)");
    ExpectAstDump("a.b.c;", "module(`a`.b.c)");
}

TEST(Parser, ErrorLiteralsAndExpressions) {
    ExpectParseError("1 + ;", VISMUT_ERR_SYNTAX);
}

TEST(Parser, ErrorBinaryPrecedence) {
    ExpectParseError("* 2;", VISMUT_ERR_SYNTAX);
    ExpectParseError("1 + * 2;", VISMUT_ERR_SYNTAX);
    ExpectParseError("(1 + 2 * 3;", VISMUT_ERR_UNEXPECTED_TOKEN);
}

TEST(Parser, ErrorVariableDeclarations) {
    ExpectParseError("$ = 10;", VISMUT_ERR_UNEXPECTED_TOKEN);
    ExpectParseError("$x: = 10;", VISMUT_ERR_TYPE_SYNTAX);
    ExpectParseError("$x: i32;", VISMUT_ERR_UNEXPECTED_TOKEN);
}

TEST(Parser, ErrorAssignments) {
    ExpectParseError("42 = x;", VISMUT_ERR_ASSIGN_RVALUE);
    ExpectParseError("(x + 1) = 2;", VISMUT_ERR_ASSIGN_RVALUE);
    ExpectParseError("foo() = 10;", VISMUT_ERR_ASSIGN_RVALUE);
}

TEST(Parser, ErrorConditions) {
    ExpectParseError("#x", VISMUT_ERR_SYNTAX);
    ExpectParseError("! { 1; }", VISMUT_ERR_SYNTAX);
    ExpectParseError("#x { 1; } !# { 2; }", VISMUT_ERR_SYNTAX);
}

TEST(Parser, ErrorLoops) {
    ExpectParseError("@x < 10", VISMUT_ERR_SYNTAX);
    ExpectParseError("@{ x = 1; }", VISMUT_ERR_SYNTAX);
}

TEST(Parser, ErrorFunctionDeclarations) {
    ExpectParseError("$foo { 42; }", VISMUT_ERR_UNEXPECTED_TOKEN);
    ExpectParseError("$outer() { $inner() {} }", VISMUT_ERR_NESTED_FN);
    ExpectParseError("^42;", VISMUT_ERR_RETURN_OUTSIDE_FN);
}

TEST(Parser, ErrorFunctionCalls) {
    ExpectParseError("add(1, 2;", VISMUT_ERR_SYNTAX);
    ExpectParseError("add(1,, 2);", VISMUT_ERR_SYNTAX);
}

TEST(Parser, ErrorTypeCasting) {
    ExpectParseError("i32(3.14;", VISMUT_ERR_UNEXPECTED_TOKEN);
    ExpectParseError("i32();", VISMUT_ERR_SYNTAX);
}

TEST(Parser, ErrorTuples) {
    ExpectParseError("(1, \"text\", 3.0", VISMUT_ERR_SYNTAX);
}
