#include "Vismut/args/parse_args.h"
#include "Vismut/core/ast/ast_builder.h"
#include "Vismut/core/ast/ast_parser.h"
#include "Vismut/core/ast/ast_printer.h"
#include "Vismut/core/ast/value.h"
#include "Vismut/core/defines.h"
#include "Vismut/core/errors/errors.h"
#include "Vismut/core/io/file_reader.h"
#include "Vismut/core/memory/arena.h"
#include "Vismut/core/memory/string_pool.h"
#include "Vismut/core/tokenizer/tokenizer.h"
#include <stdio.h>

static VismutErrorType vismut_start(const int argc,
                                    const char *restrict const *restrict const argv) {
    VismutArgs args;
    if (!VismutArgs_Parse(argv, argc, &args)) {
        return VISMUT_ERROR_PARSING_ARGS;
    }
    if (args.is_help_int) {
        VismutArgs_PrintHelp(argv[0]);
        return VISMUT_ERROR_OK;
    }

    printf("Reading file %s...:\n", args.input_file);

    StringView file_contents = {0};
    VismutErrorType err = 0;

    SAFE_RISKY_EXPRESSION(FileReader_ReadText(args.input_file, &file_contents), err);

    printf("%.*s\n", (int)file_contents.length, file_contents.data);

    VismutErrorInfo error_info = {0};
    Arena arena = Arena_Create();
    VismutTokenizer tokenizer =
        VismutTokenizer_Create(file_contents, args.input_file, &arena, &error_info);

    StringPool string_pool = StringPool_Create();
    SAFE_RISKY_EXPRESSION(StringPool_Init(&string_pool, 512), err);

    VismutTypeContext type_ctx;
    SAFE_RISKY_EXPRESSION(VismutTypeContext_Init(&type_ctx, 512), err);

    ASTBuilder ast_builder = ASTBuilder_Create(&tokenizer);
    ASTParser parser = ASTParser_Create(&ast_builder, &string_pool, &type_ctx);

    SAFE_RISKY_EXPRESSION(ASTParser_Parse(&parser), err);

    ASTPrinter ast_printer =
        ASTPrinter_Create(ast_builder.nodes, ast_builder.nodes_length, stdout, 0);
    ASTPrinter_Print(&ast_printer, parser.module_node);

    return VISMUT_ERROR_OK;
}

int main(int argc, const char *const *const argv) {
    const VismutErrorType code = vismut_start(argc, argv);
    if (unlikely(code != VISMUT_ERROR_OK)) {
        printf(":: Vismut exit with status %d (%s)\n", code, VismutErrorType_String(code));
        VismutArgs_PrintHelp(argv[0]);
    }

    return code;
}
