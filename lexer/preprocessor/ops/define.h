#ifndef VERB_preprocessor_ops_define_included
#define VERB_preprocessor_ops_define_included

#include "_includes.h"
#include "string.h"

// 'VERB_preprocessor_put_macro' is some of the worst code on this project.
// IDK how one would clean it up. perhaps by splitting it into two functions.
// the logic behind it is pretty nice. 

// at this point I've given up coding a compiler that would work for any language, easy to retarget. IDK how one would even write a retargetable preprocessor.
// you'd have to rewrite 99% of this file to retarget one. thankfully most languages don't support a preprocessor & those that do support one similar to C.

// in the end, it's not that ugly!

typedef struct{
    char* definition;
    size_t definition_len;
    char** args;
    size_t* arglen;
    size_t argcnt;
} VERB_preprocessor_define_element_t;

void* VERB_preprocessor_define_element_init(char* const restrict definition, const size_t definition_len, char** const restrict args, size_t* const restrict arglen, const size_t argcnt){
    VERB_preprocessor_define_element_t* const restrict element = malloc(sizeof(*element));

    element->definition = definition;
    element->definition_len = definition_len;
    element->args = args;
    element->arglen = arglen;
    element->argcnt = argcnt;
    
    return element;
}

void VERB_preprocessor_define_element_destroy(void* formal_element){
    VERB_preprocessor_define_element_t* element = formal_element;

    free(element->definition);
    for(size_t i = 0; i < element->argcnt; i++)
        free(element->args);

    free(element);
}

#define VERB_preprocessor_define_element(tokeniser, name, namelen, definition, definition_len, args, arglen, argcnt)                                                \
    do{                                                                                                                                                             \
        VERB_preprocessor_define_element_t* INTERNAL_def = VERB_preprocessor_define_element_init(definition, definition_len, args, arglen, argcnt);                 \
    \
        const bool success =                                                                                                                                        \
            VERB_rht_put((tokeniser)->preprocessor->defined_things, name, INTERNAL_def, VERB_preprocessor_define_element_destroy, namelen, VERB_rht_destroy_obj);   \
        if(!success){                                                                                                                                               \
            VERB_error_report((tokeniser)->specifics, VERB_error_macro_redefinition, (tokeniser)->line, (tokeniser)->offset, 2, name, namelen);                     \
            VERB_preprocessor_define_element_destroy(INTERNAL_def);                                                                                                 \
        }                                                                                                                                                           \
    } while(0)

char* VERB_preprocessor_op_define_copy_definition(char* contents, char* restrict* const restrict def, size_t* const restrict deflen){
    VERB_array_t whitespaceless = VERB_array_init();
    
// copies everything EXCEPT for whitespace; works because *string is only updated at the end & thorought the function it points to the start of the string.
    while(*contents != '\n' && *contents){
    // adds proper \n escaping behaviour by removing the \n.
        size_t tocpy;
        if(contents[0] == '\\'){
            if(contents[1] == '\n'){ contents += 2; continue; }
            tocpy = 1;
        }
        else tocpy = strcspn(contents, "\\\n");
        VERB_array_push_string(contents, tocpy, &whitespaceless);
    }
// terminated by \0 so it's a C string.
    VERB_array_push_char('\0', &whitespaceless);

    *deflen = VERB_array_extract_length(&whitespaceless);
    *def = VERB_array_extract(&whitespaceless);
    VERB_array_destroy(&whitespaceless, VERB_array_keep_ctx);

    return contents;
}

char* VERB_preprocessor_op_define_0arg(VERB_tokeniser_t* const restrict tokeniser, char* const contents, char* const name, const size_t namelen){
    char* restrict def; size_t deflen;
    char* const restrict enddef = VERB_preprocessor_op_define_copy_definition(contents, &def, &deflen);

    VERB_preprocessor_define_element(tokeniser, name, namelen, def, deflen, NULL, NULL, 0);
    return enddef;
}

char* VERB_preprocessor_define_manyarg(VERB_tokeniser_t* const restrict tokeniser, char* args_start, char* const args_end, char* const name, const size_t namelen){
// get name until operator.
// check that operator is comma, error otherwise.
// inc argcnt.
    size_t argcnt = 0;
    char** restrict arguments = NULL;
    size_t* restrict arguments_lengths = NULL;

    while(args_start < args_end){
        arguments = realloc(arguments, (argcnt+1)*sizeof(*arguments));
        arguments_lengths = realloc(arguments_lengths, (argcnt+1)*sizeof(*arguments_lengths));

        const size_t arglen = VERB_REGEX_statement_length(args_start);
    // linearly searches through all past arguments to see if argument name already exists.
    //  YES it is slowish since it's O(n) but IDC. for how few elements macros usually have (1~8) this might be the best option.
        for(size_t i = 0; i < argcnt; i++){
            if(arglen == arguments_lengths[i] && !memcmp(arguments[i], args_start, arglen))
                VERB_error_report(tokeniser->specifics, VERB_error_macro_redefinition, tokeniser->line, tokeniser->offset, 2, args_start, arglen);
        }

        arguments[argcnt] = malloc(arglen + 1);
        memcpy(arguments[argcnt], args_start, arglen);
        arguments[argcnt][arglen] = '\0';
        arguments_lengths[argcnt] = arglen;

        argcnt++;
        args_start += arglen;            // points to ','/delimeter.

        if(*args_start != ',' && args_start + 1 != args_end)
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID ARGUMENT SEPARATOR IN #define");
        else if(*args_start != ')' && args_start + 1 == args_end)
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID MACRO TERMINATOR IN #define");
        args_start++;
    }
    
    char* restrict def; size_t deflen;
    char* const restrict enddef = VERB_preprocessor_op_define_copy_definition(args_end + 1, &def, &deflen);

    VERB_preprocessor_define_element(tokeniser, name, namelen, def, deflen, arguments, arguments_lengths, argcnt);
// now points to end of definition.
    return enddef;
}

// parses macro definition.
VERB_bytecode_t VERB_preprocessor_op_define(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// format of a definition:
// #define NAME(arg0,...,argn) textual_replacement \n
// we:
//      get end of definition.
//      parse all arguments into separate array.
//      store definition.
// TODO: add multiline comments.
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#define", tokeniser)) return VERB_TOKEN_special_IGNORE;
    char* const name = *string;
    const size_t namelen = VERB_REGEX_statement_length(name);
    *string += namelen; tokeniser->offset += namelen;

    char *args_start = *string, *def_end = NULL;
    if(*args_start != '(')
        def_end = VERB_preprocessor_op_define_0arg(tokeniser, args_start, name, namelen);
    else{
        char* const args_end = *string + strcspn(*string, ")");
        def_end = VERB_preprocessor_define_manyarg(tokeniser, args_start, args_end, name, namelen);
    }

    tokeniser->offset += (unsigned long long)(def_end - *string);
    *string = def_end;
    return VERB_BC_special_IGNORE;
}

VERB_bytecode_t VERB_preprpocessor_put_macro_0arg(VERB_preprocessor_define_element_t* const restrict macro, VERB_tokeniser_t* const restrict tokeniser){
    char* definition = macro->definition;
    
    const unsigned long long old_line = tokeniser->line, old_offset = tokeniser->offset;
    while(*definition) VERB_lexer_line(&definition, tokeniser, false);
    tokeniser->line = old_line; tokeniser->offset = old_offset;

    return VERB_BC_special_IGNORE;
}


bool VERB_preprocessor_put_macro_preprocessor_command(char* restrict definition, VERB_tokeniser_t* const restrict tokeniser, size_t* const restrict len){
    *len = VERB_REGEX_preprocessor_length(definition);
// ignores ## macro commands, which HAVE to be parsed at this level
    if(*len == 2 && definition[0] == '#' && definition[1] == '#') return false;
// #ignore specifies that macro resolution should not expand or preprocess the following statement & that it should process it directly.
    if(*len == strlen("#ignore") && memcmp(definition, "#ignore", strlen("#ignore"))){
        VERB_preprocessor_op__skip_whitespaces(tokeniser, &definition);
        *len = VERB_REGEX_statement_length(definition);
    }

    return true;
}

// takes definition.
// builds up textual replacement.
// parses that.
VERB_bytecode_t VERB_preprocessor_put_macro(VERB_preprocessor_define_element_t* const restrict macro, char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(macro->argcnt == 0) return VERB_preprpocessor_put_macro_0arg(macro, tokeniser);

// PART 1 - builds correspondences between arg & respective element. perhaps using an RHT is overkill.
// correspondences rht is set up as 'macro argument name' (key/formal parameter) -> 'real argument name'.
// parses call to macro.
    VERB_rht_t correspondences; VERB_rht_init(&correspondences);
// this here below is so arglen does not have to be recomputed each time; unironically allocating memory dynamically might be slower ;-;.
// if the compiler's slow, it's likely to be a 'death by a thousand cuts' situation more than anything.
    typedef struct{
        char* replacement;
        size_t len;
    } macro_correspondence_t;
// skips all whitespace between macro name & first parenthesis.
    VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
    uint64_t nested_depth = 1;

    if(**string != '(') 
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "MACRO NOT FOLLOWED BY PARENTHESES");
    *string += 1; tokeniser->offset += 1;
    for(size_t i = 0; i < macro->argcnt; i++){
        VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
        
        macro_correspondence_t* corr = malloc(sizeof(*corr));
        corr->replacement = *string;
// allows for weird stuff to be passed as a macro argument.
// TODO: newlines in macros act weirdly! FIX THEM!
        corr->len = VERB_REGEX_macro_argument_length(*string, i == macro->argcnt-1);
        *string += corr->len; tokeniser->offset += corr->len;

        if(!VERB_rht_put(&correspondences, macro->args[i], corr, free, macro->arglen[i], VERB_rht_destroy_obj)) free(corr);
        VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
    }
    if(**string){ *string += 1; tokeniser->offset += 1; }   // skips over last \n
// PART 2 - replaces all arguments with their textual representation.
// TODO: add ignoring ##.
    char* definition = macro->definition;

    VERB_array_t new_definition = VERB_array_init();
    while(*definition){
        size_t statement_length = VERB_REGEX_statement_length(definition);

        if(statement_length){
        // we replace the definition textually with the appropriate one.
            macro_correspondence_t* corr = VERB_rht_search(&correspondences, definition, statement_length);
            if(corr) VERB_array_push_string(corr->replacement, corr->len, &new_definition);
        }
        else if(VERB_preprocessor_put_macro_preprocessor_command(definition, tokeniser, &statement_length)){
            if(!statement_length && !(statement_length = VERB_REGEX_not_statement_length(definition))) 
                statement_length = 1;
            VERB_array_push_string(definition, statement_length, &new_definition);
        }

        definition += statement_length;
    }

// PART 3 - cleanup & construction of new code.
    VERB_rht_destroy(&correspondences, VERB_rht_keep);
    VERB_array_push_char('\0', &new_definition);

    char* toanalyse = VERB_array_extract(&new_definition);
    VERB_array_destroy(&new_definition, VERB_array_keep_ctx);

    const unsigned long long old_line = tokeniser->line, old_offset = tokeniser->offset;
    while(*toanalyse) VERB_lexer_line(&toanalyse, tokeniser, false);
    tokeniser->line = old_line; tokeniser->offset = old_offset;

    free(toanalyse);

    return VERB_BC_special_IGNORE;
}

#endif
