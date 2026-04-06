#define _FILE_OFFSET_BITS 64

#include "file_reader.h"
#include "../defines.h"
#include "../errors/errors.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

VismutErrorType FileReader_ReadText(const u8 *restrict filename, StringView *restrict out_buffer) {
    FILE *file = fopen((const char *restrict)filename, "rb");
    if (unlikely(file == NULL)) {
        return VismutErrorType_FromErrno(errno, VISMUT_ERROR_IO);
    }

    if (unlikely(fseeko(file, 0, SEEK_END) != 0)) {
        fclose(file);
        return VismutErrorType_FromErrno(errno, VISMUT_ERROR_IO);
    }

    const off_t file_size = ftello(file);
    if (unlikely(file_size < 0)) {
        fclose(file);
        return VismutErrorType_FromErrno(errno, VISMUT_ERROR_IO);
    }

    if (unlikely(fseeko(file, 0, SEEK_SET) != 0)) {
        fclose(file);
        return VismutErrorType_FromErrno(errno, VISMUT_ERROR_IO);
    }

    u8 *buffer = malloc(file_size + 1);
    if (unlikely(buffer == NULL)) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    if (unlikely(fread(buffer, sizeof(u8), file_size, file) != (size_t)file_size)) {
        int err = errno;
        free(buffer);
        if (ferror(file)) {
            fclose(file);
            return VismutErrorType_FromErrno(err, VISMUT_ERROR_IO);
        }
        fclose(file);
        return VISMUT_ERROR_IO_UNEXPECTED_EOF;
    }
    fclose(file);

    buffer[file_size] = '\0';
    *out_buffer = (StringView){
        .data = buffer,
        .length = file_size,
    };

    return VISMUT_ERROR_OK;
}
