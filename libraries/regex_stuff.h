#ifndef VERB_library_regex_stuff_included
#define VERB_library_regex_stuff_included

#include "stdbool.h"                                        // boolean types
#include "stdint.h"                                         // integer types
#include "stddef.h"                                         // size_t
#include "string.h"                                         // strspn, strcspn

// TODO: redefine these using strcspn & strspn!

// all characters allowed to be the first or second through last character in a statement (eg. variables)
#define VERB_REGEX_string_statements_first "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$"
#define VERB_REGEX_string_statements_last  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$0123456789"
// all operator characters (at least one use per symbol is shown; this is not an EXHAUSTIVE list)
// \\ -> escape character symbol '\'.
// \" -> double quotes (strings).
// '  -> single quotes (characters).
// <> -> less-than, greater-than, shifts, type casts.
// ()[]{} -> various brackets.
// +-*/% -> various arithmetic operatots.
// &|~! -> various logical operators.
// ?  -> optional arguments.
// :; -> array indexing.
// ,  -> to separate function arguments.
// .  -> to terminate statements.

// regex for preprocessor commands.
#define VERB_REGEX_preprocessor_command "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_#"

// all characters that cannot be inside a call to a macro (comma, as it delimits arguments).
#define VERB_REGEX_string_macro_reject      ","
#define VERB_REGEX_string_macro_reject_last ",)"

#define VERB_REGEX_string_operators "\\\"'<>()[]{}+-*/%&|~!?!:;,.@"
// all characters allowed to be the first or second or third through last character in a number 
// letters included to support hex numbers. 
// TODO: consider adding support for base64 under a prefix like 0= or 0@ (unreadable!).
// TODO: consider allowing custom numeric formats? through 'conversion' keyword.
#define VERB_REGEX_string_numeric_first  "0123456789"
#define VERB_REGEX_string_numeric_second "0123456789abcdefABCDEFxo"         // adds 'x' for 0x and 'o' for 0o
#define VERB_REGEX_string_numeric_last   "0123456789abcdefABCDEF"
// all whitespace
#define VERB_REGEX_string_whitespace " \t\v\r\f"

#define VERB_REGEX_string_not_statements_first (VERB_REGEX_string_operators VERB_REGEX_string_whitespace "\n" VERB_REGEX_string_numeric_first "#")
// assumes VERB_REGEX_preprocessor_commands holds all of the extra characters in VERB_REGEX_string_numeric_second not in VERB_REGEX_string_numeric_last.
#define VERB_REGEX_string_not_statements_last  (VERB_REGEX_string_operators VERB_REGEX_string_whitespace "\n" VERB_REGEX_string_numeric_last VERB_REGEX_preprocessor_command)

// converts ascii digit encoding hex character to nibble
uint8_t VERB_REGEX_convert_to_hex(const char ascii_digit){
    return '0' <= ascii_digit && ascii_digit <= '9'? ascii_digit - '0': (ascii_digit & ~0x20) - 'A';
}

// length of macro argument; W preprocessor directives??
#define VERB_REGEX_macro_argument_length(string, islast)  \
    ((islast)? strcspn((string), VERB_REGEX_string_macro_reject): strcspn((string), VERB_REGEX_string_macro_reject_last))

size_t VERB_REGEX_statement_length(char* string){
    if(!strchr(VERB_REGEX_string_statements_first, *string)) return 0;
    return strspn(string, VERB_REGEX_string_statements_last);
}

// length of string not a statement. this accepts numbers & operators & all that is not a statement.
size_t VERB_REGEX_not_statement_length(char* string){
    if(!strchr(VERB_REGEX_string_not_statements_first, *string)) return 0;
    return strspn(string, VERB_REGEX_string_not_statements_last);
}

size_t VERB_REGEX_preprocessor_length(char* const restrict string){
    return strspn(string, VERB_REGEX_preprocessor_command);
}

// returns length substring that starts at string not accepted by regex [a-zA-Z_#@$0-9\n ]* (where \n is newline)
size_t VERB_REGEX_operator_length(char* string){
    return strspn(string, VERB_REGEX_string_operators);
}

// returns length of largest substring whose start is 'string' that is accepted the following regex:[0-9][a-zA-Z0-9]*
// if no string is accepted by said REGEX, it returns 0.
size_t VERB_REGEX_numeric_length(char* string){
// DO NOT CHANGE THIS; THIS CAN LEAD TO UNSAFE CODE VERY EASILY!
    if(!strchr(VERB_REGEX_string_numeric_first, *string)) return 0;
    if(*(string + 1) && !strchr(VERB_REGEX_string_numeric_second, *string)) return 0;
    return strspn(string, VERB_REGEX_string_numeric_last);
}

size_t VERB_REGEX_whitespace_length(char* string){
    return strspn(string, VERB_REGEX_string_whitespace);
}

size_t VERB_REGEX_not_whitespace_length(char* string){
    return strcspn(string, VERB_REGEX_string_whitespace);
}

#endif
