#ifndef VISMUT_CORE_IO_FILE_READER_H
#define VISMUT_CORE_IO_FILE_READER_H

#include "../errors/errors.h"
#include "../types.h"

VismutErrorType FileReader_ReadText(const u8 *path, StringView *out_buffer);

#endif
