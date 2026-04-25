#ifndef VERB_bytecode_op_fieldof_included
#define VERB_bytecode_op_fieldof_included

#include "../_includes.h"
#include "../_primitives/primitives.h"

void VERB_bytecode_op_fieldof(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// ensures top of stack is something one can extract a field out of.
    VERB_op__opcode(tokeniser, VERB_BC_complex_type_field);
    VERB_array_pop_type(VERB_bytecode_t, &tokeniser->ops);
// gets the complex type to process.
    const VERB_variable_token_t complex_tok = VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values);
    VERB_variable_t* const restrict complex_var = VERB_variable_search_byToken(&tokeniser->backend, complex_tok);
    
    if(!complex_var){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "VARIABLE SPECIFIED DOES NOT EXIST");
        return;
    }
    if(complex_var->type->group != VERB_TYPEGROUP_COMPLEX || !complex_var->custom_data){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "VARIABLE SPECIFIED IS NOT COMPATIBLE WITH COMPLEX TYPES");
        return;
    }
    VERB_tokeniser_skip_whitespace(string, tokeniser);

    const size_t fieldlen = VERB_REGEX_statement_length(*string);
    char* const fieldname = *string;
    VERB_variable_t* const restrict field = VERB_variable_complex_search_byName(complex_var->custom_data, fieldname, fieldlen);
    if(!field){
        const char t = fieldname[fieldlen];
        fieldname[fieldlen] = '\0';
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 3, "VARIABLE SPECIFIED DOES NOT HAVE FIELD '", fieldname, "'");
        fieldname[fieldlen] = t;
        return;
    }
    VERB_op__token(string, tokeniser, field->name);
    VERB_tokeniser_skip_chars(string, tokeniser, fieldlen);
}

#endif
