#include "error_printer.h"
#include <stdio.h>

void ErrorPrinter_Print(const StringView module_name, const VismutErrorInfo *info, FILE *file) {
    if (info == NULL)
        return;

    if (!StringView_IsEmpty(module_name)) {
        fputs("Error caught in \"", file);
        StringView_Write(module_name, file);
        fputs("\"!\n", file);
    } else {
        fputs("Error caught!\n", file);
    }

    u32 line_start = 0;
    u32 line_num = 1;

    for (u32 i = 0; i < info->pos.offset && i < info->source.length; ++i) {
        if (info->source.data[i] == '\n') {
            line_start = i + 1;
            line_num++;
        }
    }

    u32 line_end = info->pos.offset;
    while (line_end < info->source.length && info->source.data[line_end] != '\n' &&
           info->source.data[line_end] != '\r') {
        line_end++;
    }

    const u32 line_length = line_end - line_start;
    const u32 col = info->pos.offset - line_start;

    fprintf(file, "%u | ", line_num);
    if (line_length > 0) {
        fwrite(info->source.data + line_start, 1, line_length, file);
    }
    fputs("\n", file);

    const int prefix_len = snprintf(NULL, 0, "%u | ", line_num);
    for (int i = 0; i < prefix_len; i++) {
        putchar(' ');
    }

    for (u32 i = 0; i < col; i++) {
        if (info->source.data[line_start + i] == '\t') {
            putchar('\t');
        } else {
            putchar('-');
        }
    }

    u32 err_len = info->pos.length > 0 ? info->pos.length : 1; // минимум 1 символ

    if (col + err_len > line_length) {
        err_len = line_length - col;
    }

    for (u32 i = 0; i < err_len; i++) {
        putchar('^');
    }
    printf("\n");

    printf("  %s\n", VismutErrorType_String(info->type));

    switch (info->type) {
    case VISMUT_ERR_OOM:
        printf("  Details: Failed to allocate %lu bytes.\n", info->details.oom.bytes_required);
        printf("  Location: ");
        StringView_Write(info->details.oom.location, file);
        printf("\n");
        break;

    case VISMUT_ERR_UNEXPECTED_CHAR:
        printf("  Details: Got char '%c' (0x%02X)\n", info->details.char_, info->details.char_);
        break;

    case VISMUT_ERR_UNEXPECTED_TOKEN:
        printf("  Details: Unexpected token type, expect = '%s'\n",
               VismutTokenType_String(info->details.token_type));
        break;

    case VISMUT_ERR_UNSUPPORTED_BINARY:
        printf("   Details: %s %s %s.\n", VismutTypeKind_String(info->details.binary.left),
               ASTBinaryNodeType_String(info->details.binary.op),
               VismutTypeKind_String(info->details.binary.right));
        break;

    case VISMUT_ERR_UNSUPPORTED_UNARY:
        printf("   Details: %s %s.\n", ASTUnaryNodeType_String(info->details.unary.op),
               VismutTypeKind_String(info->details.unary.right));
        break;

    default:
        break;
    }
    printf("\n");
}
