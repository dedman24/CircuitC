#ifndef VERB_bytecode_op_comments_included
#define VERB_bytecode_op_comments_included

#include "string.h"                     // strstr, strlen
#include "../_includes.h"
#include "scoping_and_whitespace.h"     // proper behaviour when encountering a newline

void VERB_bytecode__update_newlines(VERB_tokeniser_t* const restrict tokeniser, char* const restrict string, const size_t len){
    char* newstr = strchr(string, '\n') + 1;
    while(newstr < string + len){
        VERB_bytecode_op_newline_proper_behaviour(tokeniser);
        tokeniser->offset = (unsigned long long)(string+len - newstr);
        newstr = strchr(newstr, '\n') + 1;
    }
}

void VERB_bytecode_op_multi_line_comments(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
/*
 * ALL CASES:
 *      comment ends with EOF
 *      comment ends & there's no start to compare with
 *      next_multiline_start < next_multiline_end -> inc nested comment ctr
 *      next_multiline_end > next_multiline_start -> 
 *          dec nested comment ctr
 *          nexted comment ctr == 0 ->
 *              update newline & offset
 *              return
*/
    size_t nested_comment_ctr = 1;
    char *next_multiline_end = NULL, *next_multiline_start = NULL;
// works rather well.
    while(1){
        if(!next_multiline_end) next_multiline_end = strstr(*string, VERB_tokens_MULTI_LINE_COMMENT_END);
        if(!next_multiline_start) next_multiline_start = strstr(*string, VERB_tokens_MULTI_LINE_COMMENT_START);

        if(!next_multiline_end){ *string = *string + strlen(*string); return; }                             // special case where comment ends at EOF.
        if(!next_multiline_start && next_multiline_end){                                                    // special case where there is no new start (AKA no more nested comments in file)
            VERB_bytecode__update_newlines(tokeniser, *string, (size_t)(next_multiline_end - *string));
            *string = next_multiline_end; 
            return; 
        }

        if(next_multiline_start < next_multiline_end){ nested_comment_ctr++; *string = next_multiline_start; next_multiline_start = NULL; }
        else{ 
            nested_comment_ctr--;  
            if(!nested_comment_ctr){ 
                VERB_bytecode__update_newlines(tokeniser, *string, (size_t)(next_multiline_end - *string)); 
                *string = next_multiline_end; 
                return; 
            }
            else{ *string = next_multiline_end; next_multiline_end = NULL; }
        }
    } 
}

void VERB_bytecode_op_single_line_comments(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    char* const newstr = strstr(*string, VERB_tokens_SINGLE_LINE_COMMENT_END);
    if(newstr) *string = newstr;
    else *string = *string + strlen(*string);

    VERB_bytecode_op_newline_proper_behaviour(tokeniser);
}

#endif 
