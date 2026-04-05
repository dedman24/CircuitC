#ifndef VERB_bytecode_op_name_value_included
#define VERB_bytecode_op_name_value_included

#include "_includes.h"
#include "../../libraries/value_conversion.h"
#include "../../libraries/arrays/dynamic_arrays.h"
#include "../../libraries/errors_warnings/errors_warnings.h"

void VERB_put_name(VERB_array_t* const restrict array, char* restrict* restrict string, const size_t name_token_length, VERB_tokeniser_t* const restrict tokeniser){
// a name token is followed by a size_t indicating the length of said name, and the name itself.
    VERB_array_push_string(&name_token_length, sizeof(name_token_length), array);   // name length
    VERB_array_push_string(*string, name_token_length, array);                      // name itself
    *string += name_token_length; tokeniser->offset += name_token_length;           // advances string by however long the name is
}

// VERB currently supports the following types of values:
// base | name
// 2      binary (such as 0b00011011)
// 10     decimal (such as 01234567)
// 16     hexadecimal (such ax 0x0123ABCD)
// hexadecimal values always start with 0x; all encoding formats but decimal must start with a prefix
void VERB_put_value(VERB_array_t* const restrict array, char* restrict* restrict string, const size_t value_token_length, VERB_tokeniser_t* const restrict tokeniser){ 
    bool return_value;
// checks for all prefixes; done this way because all value prefixes CURRENTLY are 2 long and if the msg is less than 2
    if(value_token_length > 2) 
        return_value = VERB_CONVERSION_str_to_int(array, *string, value_token_length); 
    else 
        return_value = VERB_CONVERSION_str_decimal_to_int(array, *string, value_token_length);

    if(!return_value)
        VERB_error_report(tokeniser->specifics, VERB_error_value, tokeniser->line, tokeniser->offset, 0);

    *string += value_token_length; tokeniser->offset += value_token_length;
}

// TODO: FIX THESE!

void VERB_bytecode_op_name(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_bytecode_push(VERB_TOKEN_special_NAME, tokeniser->code);
    VERB_put_name(tokeniser->code, string, tokeniser->nameval_length, tokeniser);
}

void VERB_bytecode_op_value(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    
}

#endif
