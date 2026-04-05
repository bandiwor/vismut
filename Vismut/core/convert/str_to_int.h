#ifndef VISMUT_CORE_CONVERT_STR_TO_INT_H
#define VISMUT_CORE_CONVERT_STR_TO_INT_H
#include "../defines.h"
#include "../types.h"

attribute_pure u64 VismutConvert_BinStrToUInt(StringView);

attribute_pure u64 VismutConvert_OctStrToUInt(StringView);

attribute_pure u64 VismutConvert_DecStrToUInt(StringView);

attribute_pure u64 VismutConvert_HexStrToUInt(StringView);

#endif
