#ifndef VISMUT_CORE_COMPILER_OP_CODES_H
#define VISMUT_CORE_COMPILER_OP_CODES_H
#include "../types.h"

typedef enum {
    OP_FMT_NONE,     // No regs
    OP_FMT_A,        // reg A
    OP_FMT_sAx,      // signed A-extended
    OP_FMT_AB,       // reg A, reg B
    OP_FMT_ABx,      // reg A, unsigned B-extended
    OP_FMT_AsBx,     // reg A, signed B-extended
    OP_FMT_ABC,      // reg A, reg B, reg C
    OP_FMT_ABC_WIDE, // reg A, reg B, reg C, u32
} OpcodeFormat;

#define X_VISMUT_OPCODES(X)                                                                        \
    X(OP_UNKNOWN, "????", OP_FMT_NONE)                                                             \
    X(OP_HALT, "HALT", OP_FMT_NONE)                                                                \
    X(OP_LOAD_CONST, "LOAD_CONST", OP_FMT_ABx)                                                     \
    X(OP_LOAD_SMALL_INT, "LOAD_SMALL_INT", OP_FMT_ABx)                                             \
    X(OP_LOAD_GLOBAL, "LOAD_GLOBAL", OP_FMT_ABx)                                                   \
    X(OP_STORE_GLOBAL, "STORE_GLOBAL", OP_FMT_ABx)                                                 \
    X(OP_MOVE, "MOVE", OP_FMT_AB)                                                                  \
    X(OP_JUMP, "JUMP", OP_FMT_sAx)                                                                 \
    X(OP_JUMP_IF_FALSE, "JUMP_IF_FALSE", OP_FMT_AsBx)                                              \
    X(OP_CALL, "CALL", OP_FMT_ABC_WIDE)                                                            \
    X(OP_RET, "RET", OP_FMT_A)                                                                     \
                                                                                                   \
    X(OP_ADD_I, "ADD_I", OP_FMT_ABC)                                                               \
    X(OP_ADD_F, "ADD_F", OP_FMT_ABC)                                                               \
    X(OP_SUB_I, "SUB_I", OP_FMT_ABC)                                                               \
    X(OP_SUB_F, "SUB_F", OP_FMT_ABC)                                                               \
    X(OP_MUL_I, "MUL_I", OP_FMT_ABC)                                                               \
    X(OP_MUL_F, "MUL_F", OP_FMT_ABC)                                                               \
    X(OP_DIV_I, "DIV_I", OP_FMT_ABC)                                                               \
    X(OP_DIV_U, "DIV_U", OP_FMT_ABC)                                                               \
    X(OP_DIV_F, "DIV_F", OP_FMT_ABC)                                                               \
    X(OP_REM_I, "REM_I", OP_FMT_ABC)                                                               \
    X(OP_REM_U, "REM_U", OP_FMT_ABC)                                                               \
    X(OP_REM_F, "REM_F", OP_FMT_ABC)                                                               \
    X(OP_POW_I, "POW_I", OP_FMT_ABC)                                                               \
    X(OP_POW_F, "POW_F", OP_FMT_ABC)                                                               \
    X(OP_POW_F_I, "POW_F_I", OP_FMT_ABC)                                                           \
                                                                                                   \
    X(OP_SHL, "SHL", OP_FMT_ABC)                                                                   \
    X(OP_SHR_I, "SHR_I", OP_FMT_ABC)                                                               \
    X(OP_SHR_U, "SHR_U", OP_FMT_ABC)                                                               \
    X(OP_BOR, "BOR", OP_FMT_ABC)                                                                   \
    X(OP_BAND, "BAND", OP_FMT_ABC)                                                                 \
    X(OP_BXOR, "BXOR", OP_FMT_ABC)                                                                 \
                                                                                                   \
    X(OP_OR, "OR", OP_FMT_ABC)                                                                     \
    X(OP_AND, "AND", OP_FMT_ABC)                                                                   \
                                                                                                   \
    X(OP_EQ_I, "EQ_I", OP_FMT_ABC)                                                                 \
    X(OP_EQ_F, "EQ_F", OP_FMT_ABC)                                                                 \
    X(OP_NEQ_I, "NEQ_I", OP_FMT_ABC)                                                               \
    X(OP_NEQ_F, "NEQ_F", OP_FMT_ABC)                                                               \
                                                                                                   \
    X(OP_LESS_I, "LESS_I", OP_FMT_ABC)                                                             \
    X(OP_LESS_U, "LESS_U", OP_FMT_ABC)                                                             \
    X(OP_LESS_F, "LESS_F", OP_FMT_ABC)                                                             \
    X(OP_LEQ_I, "LEQ_I", OP_FMT_ABC)                                                               \
    X(OP_LEQ_U, "LEQ_U", OP_FMT_ABC)                                                               \
    X(OP_LEQ_F, "LEQ_F", OP_FMT_ABC)                                                               \
    X(OP_GREATER_I, "GREATER_I", OP_FMT_ABC)                                                       \
    X(OP_GREATER_U, "GREATER_U", OP_FMT_ABC)                                                       \
    X(OP_GREATER_F, "GREATER_F", OP_FMT_ABC)                                                       \
    X(OP_GEQ_I, "GEQ_I", OP_FMT_ABC)                                                               \
    X(OP_GEQ_U, "GEQ_U", OP_FMT_ABC)                                                               \
    X(OP_GEQ_F, "GEQ_F", OP_FMT_ABC)                                                               \
                                                                                                   \
    X(OP_NEG_I, "NEG_I", OP_FMT_AB)                                                                \
    X(OP_NEG_F, "NEG_F", OP_FMT_AB)                                                                \
    X(OP_NOT, "NOT", OP_FMT_AB)                                                                    \
    X(OP_BNOT, "BNOT", OP_FMT_AB)                                                                  \
                                                                                                   \
    X(OP_ITOF, "ITOF", OP_FMT_AB)                                                                  \
    X(OP_FTOI, "FTOI", OP_FMT_AB)

typedef enum {
#define X(name, text, fmt) name,
    X_VISMUT_OPCODES(X)
#undef X
        OP_COUNT,
} VismutOpcode;

const u8 *VismutOpcode_String(VismutOpcode type);

OpcodeFormat VismutOpcode_GetFormat(VismutOpcode type);

typedef u32 VismutInstruction;
typedef u64 VismutWideInstruction;

attribute_const static inline VismutInstruction
VismutInstruction_MakeNone(const VismutOpcode opcode) {
    return (VismutInstruction)(opcode & 0xFF);
}

attribute_const static inline VismutInstruction VismutInstruction_MakeA(const VismutOpcode opcode,
                                                                        const u8 a) {
    return (VismutInstruction)((opcode & 0xFF) | ((u32)a << 8));
}

attribute_const static inline VismutInstruction VismutInstruction_MakeAB(const VismutOpcode opcode,
                                                                         const u8 a, const u8 b) {
    return (VismutInstruction)((opcode & 0xFF) | ((u32)a << 8) | ((u32)b << 16));
}

attribute_const static inline VismutInstruction
VismutInstruction_MakeABC(const VismutOpcode opcode, const u8 a, const u8 b, const u8 c) {
    return (VismutInstruction)((opcode & 0xFF) | ((u32)a << 8) | ((u32)b << 16) | ((u32)c << 24));
}

attribute_const static inline VismutInstruction VismutInstruction_MakeABx(const VismutOpcode opcode,
                                                                          const u8 a, const u16 b) {
    return (VismutInstruction)((opcode & 0xFF) | ((u32)a << 8) | ((u32)b << 16));
}

attribute_const static inline VismutInstruction
VismutInstruction_MakeAsBx(const VismutOpcode opcode, const u8 a, const i16 b) {
    return (VismutInstruction)((opcode & 0xFF) | ((u32)a << 8) | (((u32)b & 0xFFFF) << 16));
}

attribute_const static inline VismutInstruction VismutInstruction_MakesAx(const VismutOpcode opcode,
                                                                          const i32 a) {
    return (VismutInstruction)((opcode & 0xFF) | (((u32)a & 0xFFFFFF) << 8));
}

attribute_const static inline VismutInstruction
VismutInstruction_MakeWide(const VismutInstruction base, const u32 extra) {
    return ((u64)extra << 32) | (u32)base;
}

attribute_const static inline u8 VismutInstruction_Opcode(const VismutInstruction instr) {
    return (u8)(instr & 0xFF);
}

attribute_const static inline u8 VismutInstruction_A(const VismutInstruction instr) {
    return (u8)((instr >> 8) & 0xFF);
}

attribute_const static inline u8 VismutInstruction_B(const VismutInstruction instr) {
    return (u8)((instr >> 16) & 0xFF);
}

attribute_const static inline u8 VismutInstruction_C(const VismutInstruction instr) {
    return (u8)((instr >> 24) & 0xFF);
}

attribute_const static inline u16 VismutInstruction_Bx(const VismutInstruction instr) {
    return (u16)((instr >> 16) & 0xFFFF);
}

attribute_const static inline i32 VismutInstruction_sBx(const VismutInstruction instr) {
    return ((i32)instr) >> 16;
}

attribute_const static inline i32 VismutInstruction_sAx(const VismutInstruction instr) {
    return ((i32)instr) >> 8;
}

#endif
