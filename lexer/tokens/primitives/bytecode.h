#ifndef VERB_token_type_bytecode_included
#define VERB_token_type_bytecode_included

#include "stdint.h"

// VERB dataflow instruction bytecodes.
// the VERB virtual machine is 
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
    VERB_BC_mod,                        // modulo                   | S0  % S1 -> D
    VERB_BC_cmod,                       // C-style modulo           | S0  % S1 -> D (where D's sign is S0.sign ^ S1.sign)
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
    VERB_BC_eq,                         // lax equality             | S0 == S1 -> D
    VERB_BC_strict_eq,                  // strict equality          | S0 == S1 && S.type == S.type -> D
    VERB_BC_neq,                        // lax inequality           | S0 != S1 -> D
    VERB_BC_strict_neq,                 // strict inequality        | S0 != S1 || S.type != S.type -> D
    VERB_BC_gt,                         // greater than             | S0  > S1 -> D
    VERB_BC_lt,                         // less than                | S0  < S1 -> D
    VERB_BC_gte,                        // greater than or equal    | S0 >= S1 -> D
    VERB_BC_lte,                        // less than or equal       | S0 <= S1 -> D
// conditional selection/dataflow-specific operations (only way to implement loops & conditionals in dataflow machines).
    VERB_BC_pick,                       // picks dest based on src  | S0 -> S1? D0: D1
    VERB_BC_merge,                      // merges two paths         | S0, S1 -> D       (very clearly in merge only one of the two source tokens is supposed to exist)
// functions.
    VERB_BC_call,                       // calls function           | S(...) -> D       IDK how to draw function calls & returns distinctively.
    VERB_BC_ret,                        // returns from function    | D <- S(...)
// misc.
    VERB_BC_comma,                      // comma operator           | S1  -> D (first S discarded)
    VERB_BC_conversion,                 // type conversion          | (S1.type)S0-> D 
    VERB_BC_farg,                       // function argument        | special symbol.
// SPECIAL CHARACTERS, DO NOT REDEFINE NOR MOVE THESE!
    VERB_BC_special_START,              // HAS to be start of all special bytecode characters.
    VERB_BC_special_OPENED_ROUND_BRACKET, 
    VERB_BC_special_CLOSED_ROUND_BRACKET, 
    VERB_BC_special_VALUE,
    VERB_BC_special_IGNORE,             // always ignored.
    VERB_BC_special_NAME_START,         // MUST be last character.
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

enum{
    VERB_PRIORITY_special_lowest,           // DO NOT CHANGE NOR MOVE!
    VERB_PRIORITY_comparison,
    VERB_PRIORITY_strict_comparison,
    VERB_PRIORITY_logical,
    VERB_PRIORITY_bitwise,
    VERB_PRIORITY_simple_arith,
    VERB_PRIORITY_shifts,
    VERB_PRIORITY_complex_arith,
    VERB_PRIORITY_special_highest           // DO NOT CHANGE NOR MOVE!
};

// array of all priorities.
const VERB_priority_t VERB_priority[] = {
// normal arithmetic.
    [VERB_BC_add] = VERB_PRIORITY_simple_arith,
    [VERB_BC_sub] = VERB_PRIORITY_simple_arith,
    [VERB_BC_neg] = VERB_PRIORITY_simple_arith,
    [VERB_BC_mul] = VERB_PRIORITY_complex_arith,
    [VERB_BC_div] = VERB_PRIORITY_complex_arith,
// wrapping arithmetic.
    [VERB_BC_mod] = VERB_PRIORITY_complex_arith,
    [VERB_BC_cmod] = VERB_PRIORITY_complex_arith,

};

#endif
