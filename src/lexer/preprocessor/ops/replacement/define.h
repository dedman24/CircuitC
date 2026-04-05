#ifndef VERB_preprocessor_ops_define_included
#define VERB_preprocessor_ops_define_included

#include "../_includes.h"
#include "primitives.h"

// 'VERB_preprocessor_put_macro' is some of the worst code on this project.
// IDK how one would clean it up. perhaps by splitting it into two functions.
// the logic behind it is pretty nice. 

// at this point I've given up coding a compiler that would work for any language, easy to retarget. IDK how one would even write a retargetable preprocessor.
// you'd have to rewrite 99% of this file to retarget one. thankfully most languages don't support a preprocessor & those that do support one similar to C.

char* VERB_preprocessor_op_macro_copy_definition(char* const restrict contents, const size_t macrolen, size_t* const restrict deflen){
    VERB_array_t whitespaceless = VERB_array_init();
// copies everything EXCEPT for whitespace until it reaches the end.
    size_t copied = 0;
    while(copied < macrolen){
        const size_t tocpy = VERB_REGEX_not_whitespace_length(contents + copied);
        if(!tocpy){
            copied += VERB_REGEX_whitespace_and_newline_length(contents);
            continue;
        }
        VERB_array_push_string(contents + copied, tocpy, &whitespaceless);
        copied += tocpy;
    }
// terminated by \0 so it's a C string.
    VERB_array_push_char('\0', &whitespaceless);

    VERB_array_extract_all(&whitespaceless, *deflen, char* const restrict def);
    VERB_array_destroy(&whitespaceless, VERB_array_keep_ctx);

    return def;
}

// parses macro definition.
// #define is used for simple '#define X y' textual replacement. for multiline macros, use #macro.
void VERB_preprocessor_op_define(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#define", tokeniser)) return;
    char* const name = *string;
    const size_t namelen = VERB_REGEX_statement_length(name);
    *string += namelen; tokeniser->offset += namelen;

    const size_t macrolen = strcspn(*string, "\n");
    char *const contents = *string, *const macroend = *string + macrolen;

    VERB_preprocessor_macro_0arg(tokeniser, name, namelen, contents, macrolen);
    
    tokeniser->offset += (unsigned long long)(macrolen);
    *string = macroend;
}

#endif
