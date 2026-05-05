#ifndef VISMUT_CORE_IO_FILE_READER_H
#define VISMUT_CORE_IO_FILE_READER_H
#include "../errors/error_details.h"
#include "../errors/errors.h"
#include "../memory/arena.h"
#include "../types.h"

attribute_nodiscard VismutErrorType FileReader_ReadText(const StringView filename,
                                                        Arena *restrict arena,
                                                        StringView *restrict out_buffer,
                                                        VismutErrorDetails *restrict out_details);

#endif
