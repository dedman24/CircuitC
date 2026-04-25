#ifndef VERB_token_type_bytecode_included
#define VERB_token_type_bytecode_included

#include "stdint.h"

// VERB dataflow instruction bytecodes.
typedef enum: uint8_t{
// normal arithmetic.                   // mneumnonic meaning       | brief description (S == source, D == dest)
    VERB_BC_add,                        // addition                 | S0  + S1 -> D
    VERB_BC_sub,                        // subtraction              | S0  - S0 -> D
    VERB_BC_neg,                        // negation                 |     - S0 -> D
    VERB_BC_mul,                        // multiplication           | S0  * S1 -> D
    VERB_BC_div,                        // division                 | S0  \ S1 -> D
// wrapping arithmetic.
    VERB_BC_w_add,                      // wrapping addition        | S0  + S1 -> D
    VERB_BC_w_sub,                      // wrapping subtraction     | S0  - S1 -> D
    VERB_BC_w_mul,                      // wrapping multiplication  | S0  * S1 -> D
    VERB_BC_mod,                        // modulo                   | S0  % S1 -> D
    VERB_BC_cmod,                       // C-style modulo           | S0 %% S1 -> D (where D's sign is S0.sign ^ S1.sign)
    VERB_BC_shl,                        // shift left               | S0 << S1 -> D
    VERB_BC_shr,                        // shift right              | S0 >> S1 -> D
    VERB_BC_sar,                        // shift right arithmetic   | S0 >> S1 -> D (sign-preserving)
    VERB_BC_rol,                        // rotate left              | S0 >>> S1 -> D
    VERB_BC_ror,                        // rotate right             | S0 <<< S1 -> D
// bitwise.
    VERB_BC_and,                        // and                      | S0  & S1 -> D
    VERB_BC_or,                         // or                       | S0  | S1 -> D
    VERB_BC_xor,                        // xor                      | S0  ^ S1 -> D
    VERB_BC_not,                        // not                      |     ~ S0 -> D
// logical.
    VERB_BC_l_and,                      // logical and              | (S0 != 0) & (S1 != 0) -> D
    VERB_BC_l_or,                       // logical or               | (S0 != 0) | (S1 != 0) -> D
    VERB_BC_l_not,                      // logical not              |  S0 != 0 -> D
    VERB_BC_l_xor,                      // logical xor              | (S0 != 0) != (S1 != 0) -> D
// comparison.
    VERB_BC_eq,                         // equal                    | S0 == S1 -> D
    VERB_BC_neq,                        // not equal                | S0 != S1 -> D
    VERB_BC_gt,                         // greater than             | S0  > S1 -> D
    VERB_BC_lt,                         // less than                | S0  < S1 -> D
    VERB_BC_gte,                        // greater than or equal    | S0 >= S1 -> D
    VERB_BC_lte,                        // less than or equal       | S0 <= S1 -> D
// conditional selection/dataflow-specific operations (conditionals in dataflow machines).
    VERB_BC_if,                         // picks between src & null | S0? S1: NULL -> D
    VERB_BC_else,                       // picks between two srces  | S0? S1: S2 -> D
// functions.
    VERB_BC_call,                       // calls function           | S(...) -> D       IDK how to draw function calls & returns distinctively.
    VERB_BC_ret,                        // returns from function    | D <- S(...)
// memory stuff
    VERB_BC_index,                      // array indexing           | S0:S1 -> D
// misc.
    VERB_BC_convert,                    // type conversion          | (type)S0-> D 
    VERB_BC_complex_type_field,         // field of complex type    | S0~S1 -> D.
    VERB_BC_period,                     // period/statement terminator, no real function in language.
// SPECIAL CHARACTERS, DO NOT REDEFINE NOR MOVE THESE!
    VERB_BC_special_OPENED_ROUND_BRACKET, 
    VERB_BC_special_CLOSED_ROUND_BRACKET, 
    VERB_BC_special_ARGLESS_LOW,        // 0-argument bytecode with the lowest priority. 
    VERB_BC_special_NAME_START,
    VERB_BC_special_NONE,               // no opcode in particular, use as initialiser.
    VERB_BC_special_LAST                // MUST be last bytecode opcode.
} VERB_bytecode_t;


/// /// /// /// /// /// /// /// /// /// ///
///  number of arguments per bytecode   ///
/// /// /// /// /// /// /// /// /// /// /// 

typedef uint8_t VERB_bytecode_argcnt_t;
#define VERB_bytecode_argcnt_assemble(x, y) ((x)<<4 | (y))
#define VERB_bytecode_argcnt_src(x) ((x)>>4 & 0x0f)
#define VERB_bytecode_argcnt_dst(x) ((x) & 0x0f)

const VERB_bytecode_argcnt_t VERB_bytecode_argcnt[] = {
    [VERB_BC_add] = VERB_bytecode_argcnt_assemble(2, 1),
    [VERB_BC_sub] = VERB_bytecode_argcnt_assemble(2, 1),
    [VERB_BC_neg] = VERB_bytecode_argcnt_assemble(1, 1),
};

/// /// /// /// /// /// /// /// /// /// ///
/// O(1) lookup of ops & their strings  ///
/// /// /// /// /// /// /// /// /// /// /// 

const char* const restrict VERB_bytecode_str[] = {
    [VERB_BC_add] = "+"
};

/// /// /// /// /// /// /// /// /// /// ///
///    order of all priority levels     ///
/// /// /// /// /// /// /// /// /// /// /// 

typedef uint8_t VERB_priority_t;

// cool way to handle priority.
// VERB compiler as of right now is a souped up operator parser.
enum{
    VERB_PRIORITY_NONE,                     // DO NOT CHANGE, NOR MOVE, NOR USE!
    VERB_PRIORITY_special_lowest,           // DO NOT CHANGE NOR MOVE!
    VERB_PRIORITY_period,                   // '.' symbol.
    VERB_PRIORITY_if,                       // if statement.
    VERB_PRIORITY_comparison,
    VERB_PRIORITY_logical,
    VERB_PRIORITY_bitwise,
    VERB_PRIORITY_simple_arith,
    VERB_PRIORITY_shifts,
    VERB_PRIORITY_complex_arith,
    VERB_PRIORITY_round_bracket,
    VERB_PRIORITY_special_highest           // DO NOT CHANGE NOR MOVE!
};

// array of all priorities.
const VERB_priority_t VERB_priority[] = {
// normal arithmetic.
    [VERB_BC_add]                                   = VERB_PRIORITY_simple_arith,
    [VERB_BC_sub]                                   = VERB_PRIORITY_simple_arith,
    [VERB_BC_neg]                                   = VERB_PRIORITY_simple_arith,
    [VERB_BC_mul]                                   = VERB_PRIORITY_complex_arith,
    [VERB_BC_div]                                   = VERB_PRIORITY_complex_arith,
// wrapping arithmetic.
    [VERB_BC_mod]                                   = VERB_PRIORITY_complex_arith,
    [VERB_BC_cmod]                                  = VERB_PRIORITY_complex_arith,
// conditional selection/dataflow-specific operations (conditionals in dataflow machines).
    [VERB_BC_if]                                    = VERB_PRIORITY_if,
    [VERB_BC_else]                                  = VERB_PRIORITY_if,
// round brackets.
    [VERB_BC_special_OPENED_ROUND_BRACKET]          = VERB_PRIORITY_round_bracket,
    [VERB_BC_special_CLOSED_ROUND_BRACKET]          = VERB_PRIORITY_NONE,
};

#endif
