#ifndef VISMUT_CORE_PRINTER_ERROR_PRINTER
#define VISMUT_CORE_PRINTER_ERROR_PRINTER
#include "../errors/error_details.h"

void ErrorPrinter_Print(const StringView module_name, const VismutErrorInfo *info, FILE *file);

#endif
