#include "errors.h"
#include "../defines.h"
#include <errno.h>

attribute_const VismutErrorType VismutErrorType_FromErrno(const int err,
                                                          const VismutErrorType defaultType) {
    switch (err) {
    case ENOENT:
        return VISMUT_ERROR_IO_FILE_NOT_FOUND;
    case EACCES:
    case EPERM:
        return VISMUT_ERROR_IO_LOCKED;
    case EMFILE:
    case ENFILE:
        return VISMUT_ERROR_IO_RESOURCE_LIMIT;
    case ENOSPC:
    case EDQUOT:
        return VISMUT_ERROR_IO_DISK_FULL;
    case EIO:
        return VISMUT_ERROR_IO_FATAL;
    case EISDIR:
        return VISMUT_ERROR_IO_FILE_NOT_FOUND;
    default:
        return defaultType;
    }
}

attribute_const const u8 *VismutErrorType_String(const VismutErrorType error) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_ERRORS(X)};
#undef X

    if (unlikely(error > VISMUT_ERROR_COUNT || error < 0)) {
        return (const u8 *)VISMUT_ERROR_UNKNOWN;
    }

    return codes_table[error];
}
