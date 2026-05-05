#include "op_codes.h"

const u8 *VismutOpcode_String(const VismutOpcode type) {
#define X(name, text, fmt) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_OPCODES(X)};
#undef X

    if (unlikely(type >= OP_COUNT || type < 0)) {
        return codes_table[OP_UNKNOWN];
    }

    return codes_table[type];
}

OpcodeFormat VismutOpcode_GetFormat(const VismutOpcode type) {
#define X(name, text, fmt) [name] = fmt,
    static OpcodeFormat codes_table[] = {X_VISMUT_OPCODES(X)};
#undef X

    if (unlikely(type >= OP_COUNT || type < 0)) {
        return codes_table[OP_UNKNOWN];
    }

    return codes_table[type];
}

attribute_const VismutOpcode VismutOpcode_None(const VismutOpcode type) {
    const u32 inst = ((u32)type & 0xFF) << 24;
    return (VismutOpcode)inst;
}

attribute_const VismutOpcode VismutOpcode_A(const VismutOpcode type, const u8 a) {
    const u32 inst = (((u32)type & 0xFF) << 24) | (((u32)a & 0xFF) << 16);
    return (VismutOpcode)inst;
}

attribute_const VismutOpcode VismutOpcode_AB(const VismutOpcode type, const u8 a, const u8 b) {
    const u32 inst = (((u32)type & 0xFF) << 24) | (((u32)a & 0xFF) << 16) | (((u32)b & 0xFF) << 8);
    return (VismutOpcode)inst;
}

attribute_const VismutOpcode VismutOpcode_ABC(const VismutOpcode type, const u8 a, const u8 b,
                                              const u8 c) {
    const u32 inst = (((u32)type & 0xFF) << 24) | (((u32)a & 0xFF) << 16) | (((u32)b & 0xFF) << 8) |
                     ((u32)c & 0xFF);
    return (VismutOpcode)inst;
}

attribute_const VismutOpcode VismutOpcode_ABX(const VismutOpcode type, const u8 a, const u16 bx) {
    const u32 inst = (((u32)type & 0xFF) << 24) | (((u32)a & 0xFF) << 16) | ((u32)bx & 0xFFFF);
    return (VismutOpcode)inst;
}

attribute_const VismutWideInstruction VismutOpcode_ABC_WIDE(const VismutOpcode type, const u8 a,
                                                            const u8 b, const u8 c,
                                                            const u32 extra) {
    const VismutOpcode base = VismutOpcode_ABC(type, a, b, c);
    return (VismutWideInstruction)base | ((VismutWideInstruction)extra << 32);
}

attribute_const VismutWideInstruction VismutOpCode_MakeWide(const VismutInstruction a,
                                                            const VismutInstruction b) {
    return (VismutWideInstruction)a | ((VismutWideInstruction)b << 32);
}

attribute_const VismutOpcode VismutInstruction_DecodeType(const u32 inst) {
    return (VismutOpcode)((inst >> 24) & 0xFF);
}

attribute_const VismutDecodedABC VismutInstruction_DecodeABC(const u32 inst) {
    return (VismutDecodedABC){.type = VismutInstruction_DecodeType(inst),
                              .a = (u8)((inst >> 16) & 0xFF),
                              .b = (u8)((inst >> 8) & 0xFF),
                              .c = (u8)(inst & 0xFF)};
}

attribute_const VismutDecodedABX VismutInstruction_DecodeABX(const u32 inst) {
    return (VismutDecodedABX){
        .type = VismutInstruction_DecodeType(inst),
        .a = (u8)((inst >> 16) & 0xFF),
        .bx = (u16)(inst & 0xFFFF),
    };
}

attribute_const VismutDecodedABCWide
VismutInstruction_DecodeABCWide(const VismutWideInstruction wide_inst) {
    const u32 base = (u32)(wide_inst & 0xFFFFFFFF);

    return (VismutDecodedABCWide){
        .type = (VismutOpcode)((base >> 24) & 0xFF),
        .a = (u8)((base >> 16) & 0xFF),
        .b = (u8)((base >> 8) & 0xFF),
        .c = (u8)(base & 0xFF),
        .extra = (u32)(wide_inst >> 32),
    };
}

attribute_const u32 VismutInstruction_GetExtra(const VismutWideInstruction wide_inst) {
    return (u32)(wide_inst >> 32);
}
