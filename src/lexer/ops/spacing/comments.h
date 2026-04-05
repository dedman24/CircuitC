#ifndef VERB_bytecode_op_comments_included
#define VERB_bytecode_op_comments_included

#include "string.h"                     // strstr, strlen
#include "../_includes.h"
#include "scoping_and_whitespace.h"     // proper behaviour when encountering a newline

void VERB_bytecode_op_multi_line_comments(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    size_t nested_comment_ctr = 1;
    char *next_multiline_end = NULL, *next_multiline_start = NULL;
// works rather well.
    do{
        if(!next_multiline_end) next_multiline_end = strstr(*string, VERB_tokens_MULTI_LINE_COMMENT_END);
        if(!next_multiline_start) next_multiline_start = strstr(*string, VERB_tokens_MULTI_LINE_COMMENT_START);

        if(!next_multiline_end){ *string = *string + strlen(*string); return; }
        if(!next_multiline_start &&  next_multiline_end){ *string = next_multiline_end; return; } 

        if(next_multiline_start < next_multiline_end){ nested_comment_ctr++; *string = next_multiline_start; next_multiline_start = NULL; }
        else{ nested_comment_ctr--; *string = next_multiline_end; next_multiline_end = NULL; }
    } while(**string && nested_comment_ctr);
}

void VERB_bytecode_op_single_line_comments(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    char* const newstr = strstr(*string, VERB_tokens_SINGLE_LINE_COMMENT_END);
    if(newstr) *string = newstr;
    else *string = *string + strlen(*string);

    VERB_bytecode_op_newline_proper_behaviour(tokeniser);
}

#endif 
