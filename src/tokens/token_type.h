#ifndef VERB_token_type_included
#define VERB_token_type_included

#include "stdint.h"                         // variable-width types.
#include "primitives/type.h"                // type-specifying type.
#include "primitives/bytecode.h"            // bytecode type.
#include "../lexer/tokeniser/backend/tokeniser_type.h"

// assumes there are a maximum of 256 tokens; consider changing this!
// >mfw I want to expand the language but I wrote it so there may only be 256 tokens & so any expansion is impossible :(((
// I'm kidding it should be rather straightforward, I wrote the code so the size of a VERB token isn't implied anywhere. check whether this is true or not!
typedef uint8_t VERB_token_t;
typedef void (*VERB_token_fun_t) (char* restrict* const restrict, VERB_tokeniser_t* const restrict);

// thought there would be more of these.
enum{
// you may redefine these or add new ones to add new functionality to your language; I don't recommend changing their numerical value.
// symbols so ubiquitous I don't want to put them into a single category.
    VERB_TOKEN_SEMICOLON,                               // ;
    VERB_TOKEN_DOUBLE_QUOTES,                           // "
    VERB_TOKEN_COMMA,                                   // ,
    VERB_TOKEN_PERIOD,                                  // .
    VERB_TOKEN_QUESTION_MARK,                           // ?

    VERB_TOKEN_OPENED_BRACKET,                          // (
    VERB_TOKEN_CLOSED_BRACKET,                          // )
    VERB_TOKEN_OPENED_SQUARE_BRACKET,                   // [
    VERB_TOKEN_CLOSED_SQUARE_BRACKET,                   // ]
    VERB_TOKEN_OPENED_CURLY_BRACKET,                    // {
    VERB_TOKEN_CLOSED_CURLY_BRACKET,                    // }
// operators
    // comparison
        VERB_TOKEN_EQUALITY,                            // ==
        VERB_TOKEN_STRICT_EQUALITY,                     // ===
        VERB_TOKEN_INEQUALITY,                          // !=
        VERB_TOKEN_STRICT_INEQUALITY,                   // !==
        VERB_TOKEN_GREATER_THAN_OR_EQUAL,               // >=
        VERB_TOKEN_LESS_THAN_OR_EQUAL,                  // <=
        VERB_TOKEN_GREATER_THAN,                        // >
        VERB_TOKEN_LESS_THAN,                           // <
    // arithmetic
        VERB_TOKEN_ADDITION,                            // +
        VERB_TOKEN_SUBTRACTION,                         // -
        VERB_TOKEN_ASTERISK,                            // *
        VERB_TOKEN_MULTIPLICATION = VERB_TOKEN_ASTERISK,
        VERB_TOKEN_DIVISION,                            // /
        VERB_TOKEN_REMAINDER,                           // %
        VERB_TOKEN_C_REMAINDER,                         // %%
        VERB_TOKEN_SHIFT_LEFT,                          // <<
        VERB_TOKEN_SHIFT_RIGHT,                         // >>
    // wrapping arithmetic
        VERB_TOKEN_WRAPPING_ADDITION,                   // %+
        VERB_TOKEN_WRAPPING_SUBTRACTION,                // %-
        VERB_TOKEN_WRAPPING_MULTIPLICATION,             // %*
        VERB_TOKEN_ROTATE_LEFT,                         // <<<
        VERB_TOKEN_ROTATE_RIGHT,                        // >>>
    // bitwise
        VERB_TOKEN_BITWISE_AND,                         // &
        VERB_TOKEN_BITWISE_OR,                          // |
        VERB_TOKEN_BITWISE_XOR,                         // ^
        VERB_TOKEN_BITWISE_NOT,                         // ~
        VERB_TOKEN_BITWISE_NAND,                        // ~&
        VERB_TOKEN_BITWISE_NOR,                         // ~|
        VERB_TOKEN_BITWISE_XNOR,                        // ~^
    // logical
        VERB_TOKEN_LOGICAL_AND,                         // &&
        VERB_TOKEN_LOGICAL_OR,                          // ||
//        VERB_TOKEN_LOGICAL_XOR,                       // ^^  -> alias of !=
        VERB_TOKEN_LOGICAL_NOT,                         // !
        VERB_TOKEN_LOGICAL_NAND,                        // !&&
        VERB_TOKEN_LOGICAL_NOR,                         // !||
//        VERB_TOKEN_LOGICAL_XNOR,                      // !^^ -> alias of ==
    // types
        VERB_TOKEN_TYPE_CONVERSION,                     // ->
        VERB_TOKEN_METHOD_OF,                           // ::
        VERB_TOKEN_TYPEOF,                              // @
        VERB_TOKEN_UNDERLYING_TYPEOF,                   // @@
    // memory
        VERB_TOKEN_TYPE_ELEMENT,                        // ~
        VERB_TOKEN_ARRAY_START,                         // :
        VERB_TOKEN_ARRAY_END,                           // ;
        VERB_TOKEN_in,                                  // in
// statements
    // loop statements
        VERB_TOKEN_for,                                 // for
        VERB_TOKEN_while,                               // while
        VERB_TOKEN_do,
        VERB_TOKEN_break,
        VERB_TOKEN_continue,
    // conditional statements
        VERB_TOKEN_if,
        VERB_TOKEN_switch,
    // return statements
        VERB_TOKEN_return,
// scope qualifiers
    VERB_TOKEN_unportable,
    VERB_TOKEN_unportable_warn,
    VERB_TOKEN_unportable_err,

// custom methods
    VERB_TOKEN_init,                                    // initialises complex type.
    VERB_TOKEN_destroy,                                 // destroys complex type.
// constants
    // pointer constants
        VERB_TOKEN_NULL,                                // special alias of false.
    // boolean constants
        VERB_TOKEN_true,
        VERB_TOKEN_false,
    // other constants required by the standard implemented as constants within their respective types instead of as their own separate tokens.
// special functions
    VERB_TOKEN_cerror,                                  // compile-time error.
    VERB_TOKEN_rerror,                                  // run-time error.
    VERB_TOKEN_cassert,                                 // asserts at compile-time some condition.
    VERB_TOKEN_rassert,                                 // asserts at run-time some condition.
// misc
    VERB_TOKEN_this,
// DO NOT REDEFINE THESE!        
    // special characters
        VERB_TOKEN_WHITESPACE,
        VERB_TOKEN_NEWLINE,
        VERB_TOKEN_special_NAME,
        VERB_TOKEN_special_VALUE,
        VERB_TOKEN_special_EOF,                         // end of file.
        VERB_TOKEN_special_IGNORE,                      // always ignored.
};

#define VERB_TOKEN_NEW_SCOPE VERB_TOKEN_OPENED_CURLY_BRACKET
#define VERB_TOKEN_DEL_SCOPE VERB_TOKEN_CLOSED_CURLY_BRACKET

// special values for comment start, end
#define VERB_tokens_MULTI_LINE_COMMENT_START        "/*"
#define VERB_tokens_MULTI_LINE_COMMENT_END          "*/"
#define VERB_tokens_SINGLE_LINE_COMMENT_START       "//"
#define VERB_tokens_SINGLE_LINE_COMMENT_END         "\n"

#endif
