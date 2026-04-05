#ifndef VISMUT_CORE_TOKENIZER_TOKENIZER_CHARMAP_H
#define VISMUT_CORE_TOKENIZER_TOKENIZER_CHARMAP_H
#include "../types.h"

typedef enum {
    CT_UNKNOWN = 0,  // Unknown / Control
    CT_SPACE = 1,    // Whitespace
    CT_DIGIT = 2,    // Digit 0-9
    CT_ALPHA = 3,    // Alpha a-z, A-Z, _
    CT_QUOTES = 4,   // Quotes "
    CT_OPERATOR = 5, // Operators needing switch
} CharType;

static const u8 CharMap[256] = {
    [0x09] = CT_SPACE, // '\t'
    [0x0A] = CT_SPACE, // '\n'
    [0x0B] = CT_SPACE, // '\v'
    [0x0C] = CT_SPACE, // '\f'
    [0x0D] = CT_SPACE, // '\r'
    [0x20] = CT_SPACE, // ' '

    // 0 ... 9
    ['0'] = CT_DIGIT,
    ['1'] = CT_DIGIT,
    ['2'] = CT_DIGIT,
    ['3'] = CT_DIGIT,
    ['4'] = CT_DIGIT,
    ['5'] = CT_DIGIT,
    ['6'] = CT_DIGIT,
    ['7'] = CT_DIGIT,
    ['8'] = CT_DIGIT,
    ['9'] = CT_DIGIT,

    // a ... z
    ['a'] = CT_ALPHA,
    ['b'] = CT_ALPHA,
    ['c'] = CT_ALPHA,
    ['d'] = CT_ALPHA,
    ['e'] = CT_ALPHA,
    ['f'] = CT_ALPHA,
    ['g'] = CT_ALPHA,
    ['h'] = CT_ALPHA,
    ['i'] = CT_ALPHA,
    ['j'] = CT_ALPHA,
    ['k'] = CT_ALPHA,
    ['l'] = CT_ALPHA,
    ['m'] = CT_ALPHA,
    ['n'] = CT_ALPHA,
    ['o'] = CT_ALPHA,
    ['p'] = CT_ALPHA,
    ['q'] = CT_ALPHA,
    ['r'] = CT_ALPHA,
    ['s'] = CT_ALPHA,
    ['t'] = CT_ALPHA,
    ['u'] = CT_ALPHA,
    ['v'] = CT_ALPHA,
    ['w'] = CT_ALPHA,
    ['x'] = CT_ALPHA,
    ['y'] = CT_ALPHA,
    ['z'] = CT_ALPHA,

    // A ... Z
    ['A'] = CT_ALPHA,
    ['B'] = CT_ALPHA,
    ['C'] = CT_ALPHA,
    ['D'] = CT_ALPHA,
    ['E'] = CT_ALPHA,
    ['F'] = CT_ALPHA,
    ['G'] = CT_ALPHA,
    ['H'] = CT_ALPHA,
    ['I'] = CT_ALPHA,
    ['J'] = CT_ALPHA,
    ['K'] = CT_ALPHA,
    ['L'] = CT_ALPHA,
    ['M'] = CT_ALPHA,
    ['N'] = CT_ALPHA,
    ['O'] = CT_ALPHA,
    ['P'] = CT_ALPHA,
    ['Q'] = CT_ALPHA,
    ['R'] = CT_ALPHA,
    ['S'] = CT_ALPHA,
    ['T'] = CT_ALPHA,
    ['U'] = CT_ALPHA,
    ['V'] = CT_ALPHA,
    ['W'] = CT_ALPHA,
    ['X'] = CT_ALPHA,
    ['Y'] = CT_ALPHA,
    ['Z'] = CT_ALPHA,
    ['_'] = CT_ALPHA,

    // Specials
    ['"'] = CT_QUOTES,

    // operators
    ['{'] = CT_OPERATOR,
    ['}'] = CT_OPERATOR,
    ['['] = CT_OPERATOR,
    [']'] = CT_OPERATOR,
    ['('] = CT_OPERATOR,
    [')'] = CT_OPERATOR,
    ['.'] = CT_OPERATOR,
    [','] = CT_OPERATOR,
    [';'] = CT_OPERATOR,
    [':'] = CT_OPERATOR,
    ['^'] = CT_OPERATOR,
    ['~'] = CT_OPERATOR,
    ['?'] = CT_OPERATOR,
    ['@'] = CT_OPERATOR,
    ['!'] = CT_OPERATOR,
    ['*'] = CT_OPERATOR,
    ['<'] = CT_OPERATOR,
    ['>'] = CT_OPERATOR,
    ['='] = CT_OPERATOR,
    ['+'] = CT_OPERATOR,
    ['-'] = CT_OPERATOR,
    ['%'] = CT_OPERATOR,
    ['|'] = CT_OPERATOR,
    ['&'] = CT_OPERATOR,
    ['#'] = CT_OPERATOR,
    ['$'] = CT_OPERATOR,
    ['/'] = CT_OPERATOR,
};

static const uint8_t EscapeTable[256] = {
    ['n'] = '\n', ['t'] = '\t', ['r'] = '\r', ['"'] = '"',  ['\''] = '\'', ['\\'] = '\\',
    ['0'] = '\0', ['v'] = '\v', ['f'] = '\f', ['a'] = '\a', ['b'] = '\b'};

#endif // VISMUT_TOKENIZER_CHARMAP_H
