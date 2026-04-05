#ifndef VERB_compiler_ops_typedef_included
#define VERB_compiler_ops_typedef_included

#include "../includes.h"

// puts all tokens within the type definition inside the type definition array.
void* VERB_compiler_op_typedef_recursive(VERB_compilation_t* const restrict compilation, VERB_array_t* restrict type_definition, char* restrict tokens){
    VERB_token_t token = VERB_TOKEN_NONE;

    while(1){
        token = VERB_token_read(&tokens);
        if(token == VERB_TOKEN_EOF || token == VERB_TOKEN_SEMICOLON) break;

        if(token == VERB_TOKEN_NAME){
            const size_t type_name_len = VERB_token_get_nameval_size(&tokens);
            char* const type_name = tokens;

            VERB_compilation_scope_obj_t* const restrict type = VERB_compilation_scope_search(compilation, type_name, type_name_len);
// is not a valid, existing type; is therefore the new type's name, so we return the start of its position so the caller may handle it correctly.
            if(type->type != VERB_compilation_scope_obj_type){
                VERB_token_skip_back_area(&tokens, sizeof(type_name_len));
                return tokens;
            }

            VERB_array_push_string(type_definition, type->resolution, type->resolution_size);
            VERB_token_skip_over_area(&tokens, type_name_len);
        }
        else VERB_array_push_string(type_definition, &token, sizeof(token));
    }

    return tokens;
}

void* VERB_compiler_op_typedef(VERB_compilation_t* const restrict compilation, VERB_array_t* restrict bytecode, char* restrict tokens){
// what should typedef even do?
// add name to scope
// whenever a name is encountered, look for it within scope

// this necesitates a scope_obj struct that tells us whether a name is a type, a variable, a function & all its relevant data.
// what if the typedef is itself another typedefed thing, or something with qualifiers, or a struct or something along those lines?
// we need a recursively-handling typedef function.
    
    VERB_array_t type_definition; VERB_array_init(&type_definition);

    tokens = VERB_compiler_op_typedef_recursive(compilation, &type_definition, tokens);
    if(!tokens){
        VERB_array_destroy(&type_definition, VERB_array_keep_ctx);
        return NULL;
    } 

// extracts type definition
    size_t size_arr = VERB_array_extract_array_length(&type_definition);
    void* arr = VERB_array_extract_array(&type_definition);
    VERB_array_destroy(&type_definition, VERB_array_keep_ctx);
// gets type name
    const size_t type_name_len = VERB_token_get_nameval_size(&tokens);
    char* const restrict type_name = tokens;
    VERB_token_skip_over_area(&tokens, type_name_len);
// pushes type name to scope
    VERB_compilation_scope_obj_t* type_element;
    VERB_compilation_scope_obj_init(&type_element, VERB_compilation_scope_obj_type, arr, size_arr, 0);
    VERB_compilation_scope_push(compilation, type_name, type_name_len, type_element);

    return tokens;
}

#endif
