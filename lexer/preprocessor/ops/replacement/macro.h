#ifndef VERB_preprocessor_ops_macro_included
#define VERB_preprocessor_ops_macro_included

#include "../end/endmacro.h"
#include "../_includes.h"
#include "primitives.h"
#include "define.h"

// VERB MACROS...
//      have to be started with #macro and terminated with #endmacro.
//      handle textual replacement far more 'literally'.
//      may define nested macros within them.
//      in general may have any preprocessor directive within them.
// format of a macro:
// #macro name: args, comma separated \n
//      code, copied without indentation, that may hold further preprocessing directives.
// #endmacro
//
// multi-line macros look cleaner & do not rely on '\' anymore.
// 
// for simple textual replacement/definition of constants at compile-time, #define may still be used, although it does not allow for functions anymore.
// it has been cut down because I want people to use #macro and not #define.

void VERB_preprocessor_macro_manyarg(VERB_tokeniser_t* const restrict tokeniser, char* args_start, char* const macro_start, const size_t macro_len, char* const name, const size_t namelen){
// get name until operator.
// check that operator is comma, error otherwise.
// inc argcnt.
    size_t argcnt = 0;
    char** restrict arguments = NULL;
    size_t* restrict arguments_lengths = NULL;

    while(args_start < macro_start){
        const size_t arglen = VERB_REGEX_statement_length(args_start);
    // linearly searches through all past arguments to see if argument name already exists.
    //  YES it is slowish since it's O(n) but IDC. for how few elements macros usually have (1~8) this might be the best option.
        for(size_t i = 0; i < argcnt; i++){
            if(arglen == arguments_lengths[i] && !memcmp(arguments[i], args_start, arglen))
                VERB_error_report(tokeniser->specifics, VERB_error_macro_redefinition, tokeniser->line, tokeniser->offset, 2, args_start, arglen);
        }

        arguments = realloc(arguments, (argcnt+1)*sizeof(*arguments));
        arguments_lengths = realloc(arguments_lengths, (argcnt+1)*sizeof(*arguments_lengths));
        arguments[argcnt] = malloc(arglen + 1);         // +1 for \0 terminator.

        memcpy(arguments[argcnt], args_start, arglen);
        arguments[argcnt][arglen] = '\0';
        arguments_lengths[argcnt] = arglen;

        argcnt++;
        args_start += arglen;                           // points to ','.

        if(*args_start != ',' && args_start + 1 != macro_start)
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID ARGUMENT SEPARATOR IN #define");
        args_start += VERB_REGEX_whitespace_length(args_start + 1) + 1;     // skips over comma and whitespace following comma. 
    }
    
    size_t deflen;
    char* restrict def = VERB_preprocessor_op_macro_copy_definition(macro_start + 1, macro_len, &deflen);

    VERB_preprocessor_macro_init(tokeniser, name, namelen, def, deflen, arguments, arguments_lengths, argcnt);
}

void VERB_preprocessor_op_macro(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
    char* const name = *string;
    const size_t namelen = VERB_REGEX_statement_length(*string);                            // name+namelen points to :, if it even exists.
    if(!namelen){
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_no_name_after, tokeniser->line, tokeniser->offset, 1, "#macro");
        return;
    }
// can happen if #macro is literally the last thing in the file.
    if(name[namelen] == '\0'){
        VERB_preprocessor_macro_init(tokeniser, name, namelen, NULL, 0, NULL, NULL, 0);
        return;
    }

    char* const args_start = name + VERB_REGEX_whitespace_length(name + namelen + 1);       // +1 so it skips over the ':' thingy.
    char* const macro_start = args_start + strcspn(args_start, "\n");                       // start of macro, without definition.
    char* macro_end_noendmacro;
    char* const macro_end = VERB_preprocessor_op_skip_endmacro(macro_start, &macro_end_noendmacro, tokeniser);
    const size_t macro_len = (size_t)(macro_end_noendmacro - macro_start);
// if args end when macro starts, macro takes in 0 arguments and thus we have to call 0arg version.
    if(args_start == macro_start + 1)
        VERB_preprocessor_macro_0arg(tokeniser, name, namelen, macro_start, macro_len);
    else
        VERB_preprocessor_macro_manyarg(tokeniser, args_start, macro_start, macro_len, name, namelen);

    *string = macro_end;
    return;
}

void VERB_preprocessor_put_macro_0arg(VERB_preprocessor_macro_element_t* const restrict macro, VERB_tokeniser_t* const restrict tokeniser){
    char* definition = macro->definition;
    
    const unsigned long long old_line = tokeniser->line, old_offset = tokeniser->offset;
    while(*definition) VERB_lexer_line(&definition, tokeniser, NULL);
    tokeniser->line = old_line; tokeniser->offset = old_offset;
}

bool VERB_preprocessor_put_macro_manyarg(char* restrict definition, VERB_tokeniser_t* const restrict tokeniser, size_t* const restrict len){
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
void VERB_preprocessor_put_macro(VERB_preprocessor_macro_element_t* const restrict macro, char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(macro->argcnt == 0){ VERB_preprocessor_put_macro_0arg(macro, tokeniser); return; }

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
        corr->len = VERB_REGEX_macro_argument_length(*string, i == macro->argcnt-1);
        *string += corr->len; tokeniser->offset += corr->len;

        if(!VERB_rht_put(&correspondences, macro->args[i], corr, free, macro->arglen[i], VERB_rht_destroy_obj)) free(corr);
        VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
    }
    if(**string){ *string += 1; tokeniser->offset += 1; }   // skips over last \n
// PART 2 - replaces all arguments with their textual representation.
    char* definition = macro->definition;

    VERB_array_t new_definition = VERB_array_init();
    while(*definition){
        size_t statement_length = VERB_REGEX_statement_length(definition);

        if(statement_length){
        // we replace the definition textually with the appropriate one.
            macro_correspondence_t* corr = VERB_rht_search(&correspondences, definition, statement_length);
            if(corr) VERB_array_push_string(corr->replacement, corr->len, &new_definition);
        }
        else if(VERB_preprocessor_put_macro_manyarg(definition, tokeniser, &statement_length)){
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
// only line has to be handled specially, everything else can be handled as normal.
    const unsigned long long old_line = tokeniser->line, old_offset = tokeniser->offset;
    while(*toanalyse) VERB_lexer_line(&toanalyse, tokeniser, NULL);
    tokeniser->line = old_line; tokeniser->offset = old_offset;

    free(toanalyse);

    return;
}

#endif
