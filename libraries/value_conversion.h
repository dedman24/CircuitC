#ifndef VERB_library_value_conversion_included
#define VERB_library_value_conversion_included

#include "NOAHZK_bigint_lib/noahzk_bigint.h"                // variable-width type & ops
#include "errors_warnings/errors_warnings.h"                // proper error handling
#include "dynamic_arrays.h"                                 // dynamic array type & ops
#include "regex_stuff.h"
#include "stdbool.h"                                        // boolean type
#include "stdint.h"                                         // integer types
#include "stddef.h"                                         // size_t

// all of these functions return false on failure.
// error code format:
//      INVALID FORMATTING OF VALUE

bool VERB_CONVERSION_str_decimal_to_int(VERB_array_t* const restrict array, char* string, const size_t value_token_length){
    NOAHZK_variable_width_t integer = NOAHZK_variable_width_INITIALISER; 
    const uint8_t digit_max = 9;

    for(size_t i = 0; i < value_token_length; i++){
        const char ascii_digit = *string++;
        const uint8_t digit = ascii_digit - 0x30;
        if(digit > digit_max) return false;

        NOAHZK_variable_width_mul_constant(&integer, &integer, 10, 1);
        NOAHZK_variable_width_add_constant(&integer, &integer, digit);
    }

    VERB_array_push_string(integer.arr, NOAHZK_GET_WIDTH_FROM_VAR_WIDTH_TYPE(integer), array);
    NOAHZK_variable_width_destroy(&integer, NOAHZK_variable_width_keep_ptr);
    return true;
}

bool VERB_CONVERSION_str_hex_to_int(VERB_array_t* const restrict array, char* string, const size_t value_token_length){
    uint8_t integer = 0;
    const uint8_t digit_max = 15;
    const size_t digits_in_integer = 2;

    for(size_t i = 0; i < value_token_length; i++){
        const char ascii_digit = *string++;
        const uint8_t digit = VERB_REGEX_convert_to_hex(ascii_digit);
        if(digit > digit_max) return false;

        integer = integer << 4 | digit;

        if(i % digits_in_integer == digits_in_integer - 1 || i == value_token_length - 1){
            VERB_array_push_char(integer, array);
            integer = 0;
        }
    }

    return true;
}

bool VERB_CONVERSION_str_binary_to_int(VERB_array_t* const restrict array, char* string, const size_t value_token_length){
    uint8_t integer = 0;
    const uint8_t digit_max = 1;
    const size_t digits_in_integer = 8;
    
    for(size_t i = 0; i < value_token_length; i++){
        const char ascii_digit = *string++;
        const uint8_t digit = ascii_digit - 0x30;
        if(digit > digit_max) return false;

        integer = integer << 1 | digit;
// integer is full or it's the last iter
        if(i % digits_in_integer == digits_in_integer - 1 || i == value_token_length - 1){
            VERB_array_push_char(integer, array);
            integer = 0;
        }
    }

    return true;
}

bool VERB_CONVERSION_str_to_int(VERB_array_t* const restrict array, char* restrict string, const size_t value_token_length){
    bool return_value;
// table holding all prefix functions, pretty easy to modify if needed
    struct{
        char* prefix;
        size_t prefix_length;
        bool (*string_to_integer)(VERB_array_t*, char*, const size_t);
    } TYPE_PREFIXES[] = {
        {"0x", 2, VERB_CONVERSION_str_hex_to_int},
        {"0b", 2, VERB_CONVERSION_str_binary_to_int},
    };
// conversion without prefix to fall back on
    bool (*unprefixed_conversion)(VERB_array_t*, char*, const size_t) = VERB_CONVERSION_str_decimal_to_int;

    const uint64_t entries_in_type_prefix_array = sizeof(TYPE_PREFIXES)/sizeof(*TYPE_PREFIXES);

    for(uint64_t i = 0; i < entries_in_type_prefix_array; i++){
        if(value_token_length > TYPE_PREFIXES[i].prefix_length){
            const size_t length_without_prefix = value_token_length - TYPE_PREFIXES[i].prefix_length;
            char* const string_without_prefix  = string + TYPE_PREFIXES[i].prefix_length;
            
            if(memcmp(string, TYPE_PREFIXES[i].prefix, TYPE_PREFIXES[i].prefix_length) == 0){
                return_value = TYPE_PREFIXES[i].string_to_integer(array, string_without_prefix, length_without_prefix);
                break;
            } 
        }
// no entry with this prefix; calls standard, no-prefix function.
        if(i == entries_in_type_prefix_array - 1) 
            return_value = unprefixed_conversion(array, string, value_token_length);
    }

    return return_value;
}

#endif
