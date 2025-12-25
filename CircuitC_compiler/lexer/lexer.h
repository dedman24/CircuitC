#ifndef CIRCUITC_lexer_included
#define CIRCUITC_lexer_included

#include "NOAHZK_bigint_lib/noahzk_bigint.h"    // bigint type, bigint ops
#include "lexer_error_handling.h"               // error handling
#include "dynamic_arrays.h"                     // dynamic array type & operations
#include "tokeniser.h"                          // getting token from a string

// a lexer converts human-readable CircuitC code into a computer-readable representation of said code, encoded in TOKENS.
// example CircuitC code:
// w foo(w bar, w baz){
//      return bar & baz;
// }
// the lexer would conver this into (where <x> represents the token x)
// <w> <name> 3 ~ foo <(> <w> <name> 3 ~ bar <w> <name> 3 ~ baz <)> <{>
//      <return> <name> 3 ~ bar <&>  <name> 3 ~ baz <;>
// <}>
//
// tokens and the piece of syntax they define are held in tokens.h
// "preprocessor directives" are handled by the lexer.
// 
// the lexer returns a specialised error struct when it errors out;
// the lexer was also designed to be as general-purpose as possible. this means that it can be retargeted from one language onto another very easily.

void CIRCUITC_lexer_put_name(CIRCUITC_array_t* array, char** string, size_t name_token_length){
// a name token is followed by a size_t indicating the length of said name, and the name itself.
    CIRCUITC_array_push_string(array, &name_token_length, sizeof(name_token_length));   // name length
    CIRCUITC_array_push_string(array, *string, name_token_length);                      // name itself
    *string += name_token_length;                                                       // advances string by however long the name is
}

typedef enum{ CIRCUITC_LEXER_ERROR_NONE, CIRCUITC_LEXER_ERROR_WRONG_FORMAT, CIRCUITC_LEXER_ERROR_WRONG_PREFIX } CIRCUITC_lexer_error_t;

CIRCUITC_lexer_error_t CIRCUITC_lexer_string_decimal_to_integer(CIRCUITC_array_t* array, char* string, const size_t value_token_length){
    NOAHZK_variable_width_t integer = NOAHZK_variable_width_var_INITIALIZER; 
    const uint8_t digit_max = 9;

    for(size_t i = 0; i < value_token_length; i++){
        const char ascii_digit = *string++;
        const uint8_t digit = ascii_digit - 0x30;
        if(digit > digit_max) return CIRCUITC_LEXER_ERROR_WRONG_FORMAT;

        NOAHZK_variable_width_mul_constant(&integer, &integer, 10);
        NOAHZK_variable_width_add_constant(&integer, &integer, digit);
    }

    CIRCUITC_array_push_string(array, integer.arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(integer));
    NOAHZK_variable_width_destroy(&integer, NOAHZK_variable_width_keep_ptr);
    return CIRCUITC_LEXER_ERROR_NONE;
}

uint8_t CIRCUITC_lexer_character_hex_to_integer(const char ascii_digit){
// if it's a numeric character, return it converted properly; otherwise convert ascii digit to uppercase and subtract and add or subtract opportune values
    return CIRCUITC_tokeniser_REGEX_is_numeric(ascii_digit)? ascii_digit - 0x30: CIRCUITC_character_toupper(ascii_digit) - 0x41 + 10;
}

CIRCUITC_lexer_error_t CIRCUITC_lexer_string_hex_to_integer(CIRCUITC_array_t* array, char* string, const size_t value_token_length){
    uint8_t integer = 0;
    const uint8_t digit_max = 15;
    const size_t digits_in_integer = 2;

    for(size_t i = 0; i < value_token_length; i++){
        const char ascii_digit = *string++;
        const uint8_t digit = CIRCUITC_lexer_character_hex_to_integer(ascii_digit);
        if(digit > digit_max) return CIRCUITC_LEXER_ERROR_WRONG_FORMAT;

        integer = integer << 4 | digit;

        if(i % digits_in_integer == digits_in_integer - 1 || i == value_token_length - 1){
            CIRCUITC_array_push(array, integer);
            integer = 0;
        }
    }

    return CIRCUITC_LEXER_ERROR_NONE;
}

CIRCUITC_lexer_error_t CIRCUITC_lexer_string_binary_to_integer(CIRCUITC_array_t* array, char* string, const size_t value_token_length){
    uint8_t integer = 0;
    const uint8_t digit_max = 1;
    const size_t digits_in_integer = 8;
    
    for(size_t i = 0; i < value_token_length; i++){
        const char ascii_digit = *string++;
        const uint8_t digit = ascii_digit - 0x30;
        if(digit > digit_max) return CIRCUITC_LEXER_ERROR_WRONG_FORMAT;

        integer = integer << 1 | digit;
// integer is full or it's the last iter
        if(i % digits_in_integer == digits_in_integer - 1 || i == value_token_length - 1){
            CIRCUITC_array_push(array, integer);
            integer = 0;
        }
    }

    return CIRCUITC_LEXER_ERROR_NONE;
}

// CIRCUITC currently supports the following types of values:
// base | name
// 2      binary (such as 0b00011011)
// 10     decimal (such as 01234567)
// 16     hexadecimal (such ax 0x0123ABCD)
// hexadecimal values always start with 0x; all encoding formats but decimal must start with a prefix
CIRCUITC_lexer_error_t CIRCUITC_lexer_put_value(CIRCUITC_array_t* array, char** string, const size_t value_token_length){ 
    CIRCUITC_lexer_error_t return_value;
// checks for all prefixes
    if(value_token_length > 2){
// why use an array of structs like these instead of comparing and calling the function outright? so adding new prefixes is easier.
// still this is a pretty ugly approach.
        struct {
            char* prefix;
            size_t prefix_length;
            CIRCUITC_lexer_error_t (*string_to_integer)(CIRCUITC_array_t*, char*, const size_t);
        } TYPE_PREFIXES[] = {
            {"0x", 2, CIRCUITC_lexer_string_hex_to_integer},
            {"0b", 2, CIRCUITC_lexer_string_binary_to_integer}
        };

        const uint64_t entries_in_type_prefix_array = sizeof(TYPE_PREFIXES)/sizeof(*TYPE_PREFIXES);
        const size_t length_without_prefix = value_token_length - 2;
        char* string_without_prefix  = *string + 2;

        for(uint64_t i = 0; i < entries_in_type_prefix_array; i++){
            if(memcmp(*string, TYPE_PREFIXES[i].prefix, TYPE_PREFIXES[i].prefix_length) == 0){
                return_value = TYPE_PREFIXES[i].string_to_integer(array, string_without_prefix, length_without_prefix);
                break;
            } 
// no entry with this prefix; returns -1
            else if(i == entries_in_type_prefix_array - 1) return CIRCUITC_LEXER_ERROR_WRONG_PREFIX;
        }
    } 
    else return_value = CIRCUITC_lexer_string_decimal_to_integer(array, *string, value_token_length);

    *string += value_token_length;
    return return_value;
}

// given string of CircuitC code, converts it to string of tokens that has to be freed by user
void* CIRCUITC_lexer(char* string, CIRCUITC_lexer_error_t* error_code){
// static initialization? it doesn't really matter which one I use in the end.
    CIRCUITC_tokeniser_t tokeniser; CIRCUITC_tokeniser_init(&tokeniser);
    CIRCUITC_array_t array; CIRCUITC_array_init(&array);
    CIRCUITC_token_t token;

    *error_code = CIRCUITC_LEXER_ERROR_NONE;
    size_t current_line = 0, current_offset = 0;

    do{
        size_t nameval_token_length = 0, lines_skipped;
        token = CIRCUITC_token_get(&string, &tokeniser, &lines_skipped, &current_offset, &nameval_token_length);

        CIRCUITC_array_push(&array, token);
// adds name or value to tokens
        if(token == CIRCUITC_TOKEN_NAME) CIRCUITC_lexer_put_name(&array, &string, nameval_token_length);
        else if(token == CIRCUITC_TOKEN_VALUE){
            *error_code = CIRCUITC_lexer_put_value(&array, &string, nameval_token_length);
            if(*error_code != CIRCUITC_LEXER_ERROR_NONE){
                CIRCUITC_array_destroy(&array, CIRCUITC_array_keep_ctx);
                current_offset -= nameval_token_length;
                return CIRCUITC_lexer_error_specifics_init(NULL, current_line, current_offset);
            }
        } 

        current_line += lines_skipped;
    } while(token != CIRCUITC_TOKEN_EOF);

    char* tokenised_string = CIRCUITC_array_extract_array(&array);
    CIRCUITC_array_destroy(&array, CIRCUITC_array_keep_ctx);

    return tokenised_string;
}

#endif
