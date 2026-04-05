#ifndef VERB_preprocessor_macro_primitives_included
#define VERB_preprocessor_macro_primitives_included

#include "../_includes.h"

typedef struct{
    char* definition;
    size_t definition_len;
    char** args;
    size_t* arglen;
    size_t argcnt;
} VERB_preprocessor_macro_element_t;

void* VERB_preprocessor_macro_element_init(char* const restrict definition, const size_t definition_len, char** const restrict args, size_t* const restrict arglen, const size_t argcnt){
    VERB_preprocessor_macro_element_t* const restrict element = malloc(sizeof(*element));

    element->definition = definition;
    element->definition_len = definition_len;
    element->args = args;
    element->arglen = arglen;
    element->argcnt = argcnt;
    
    return element;
}

void VERB_preprocessor_macro_element_destroy(void* formal_element){
    VERB_preprocessor_macro_element_t* element = formal_element;

    free(element->definition);
    for(size_t i = 0; i < element->argcnt; i++)
        free(element->args);

    free(element);
}

#define VERB_preprocessor_macro_init(tokeniser, name, namelen, definition, definition_len, args, arglen, argcnt)                                                    \
    do{                                                                                                                                                             \
        VERB_preprocessor_macro_element_t* INTERNAL_def = VERB_preprocessor_macro_element_init(definition, definition_len, args, arglen, argcnt);                   \
    \
        const bool success =                                                                                                                                        \
            VERB_rht_put((tokeniser)->preprocessor->defined_things, name, INTERNAL_def, VERB_preprocessor_macro_element_destroy, namelen, VERB_rht_destroy_obj);   \
        if(!success){                                                                                                                                               \
            VERB_error_report((tokeniser)->specifics, VERB_error_macro_redefinition, (tokeniser)->line, (tokeniser)->offset, 2, name, namelen);                     \
            VERB_preprocessor_macro_element_destroy(INTERNAL_def);                                                                                                 \
        }                                                                                                                                                           \
    } while(0)

#define VERB_preprocessor_macro_0arg(tokeniser, name, namelen, contents, macrolen)                              \
    do{                                                                                                         \
        size_t deflen;                                                                                          \
        char* const restrict def = VERB_preprocessor_op_macro_copy_definition(contents, macrolen, &deflen);     \
        VERB_preprocessor_macro_init(tokeniser, name, namelen, def, deflen, NULL, NULL, 0);                     \
    } while(0)

#endif
