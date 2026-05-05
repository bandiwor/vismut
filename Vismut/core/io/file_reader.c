#include <string.h>
#define _FILE_OFFSET_BITS 64

#include "../defines.h"
#include "../errors/errors.h"
#include "file_reader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

attribute_nodiscard VismutErrorType FileReader_ReadText(const StringView filename,
                                                        Arena *restrict arena,
                                                        StringView *restrict out_buffer,
                                                        VismutErrorDetails *restrict out_details) {
    VismutErrorType err = VISMUT_OK;
    char filename_buffer[256];
    char *c_filename = filename_buffer;

    if (unlikely(filename.length >= COUNTOF(filename_buffer))) {
        c_filename = Arena_Array(arena, char, filename.length + 1, &err, out_details);
        if (err != VISMUT_OK) {
            return err;
        }
    }

    __builtin_memcpy(c_filename, filename.data, filename.length);
    c_filename[filename.length] = '\0';

    FILE *file = fopen(c_filename, "rb");
    if (unlikely(file == NULL)) {
        *out_details = (VismutErrorDetails){
            .filename = filename,
        };
        return VismutErrorType_FromErrno(errno, VISMUT_ERR_IO);
    }

    if (unlikely(fseeko(file, 0, SEEK_END) != 0)) {
        *out_details = (VismutErrorDetails){
            .filename = filename,
        };
        fclose(file);
        return VismutErrorType_FromErrno(errno, VISMUT_ERR_IO);
    }

    const off_t file_size = ftello(file);
    if (unlikely(file_size < 0)) {
        *out_details = (VismutErrorDetails){
            .filename = filename,
        };
        fclose(file);
        return VismutErrorType_FromErrno(errno, VISMUT_ERR_IO);
    }

    if (unlikely(fseeko(file, 0, SEEK_SET) != 0)) {
        *out_details = (VismutErrorDetails){
            .filename = filename,
        };
        fclose(file);
        return VismutErrorType_FromErrno(errno, VISMUT_ERR_IO);
    }

    u8 *buffer = Arena_Array(arena, u8, file_size, &err, out_details);
    if (err != VISMUT_OK) {
        fclose(file);
        return err;
    }

    if (unlikely(fread(buffer, sizeof(u8), file_size, file) != (size_t)file_size)) {
        int err = errno;
        *out_details = (VismutErrorDetails){
            .filename = filename,
        };
        if (ferror(file)) {
            fclose(file);
            return VismutErrorType_FromErrno(err, VISMUT_ERR_IO);
        }
        fclose(file);
        return VISMUT_ERR_IO_EOF;
    }
    fclose(file);

    *out_buffer = (StringView){
        .data = buffer,
        .length = file_size,
    };

    return VISMUT_OK;
}
