#ifndef VERB_tokens_op_comments_included
#define VERB_tokens_op_comments_included

#include "string.h"                     // strstr, strlen
#include "_includes.h"
#include "scoping_and_whitespace.h"     // proper behaviour when encountering a newline

VERB_token_t VERB_tokens_op_multi_line_comments(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// for nested multi-line comments, we need:
//      nested ctr
//      inc if "/*" is met, dec if "*/" is met. 
//          escape if \00 is met
    size_t nested_comment_ctr = 1;
// works & doesn't segfault because multi-line comments are exactly 2 characters long, and strings are terminated with \0.
    do{
        if(!memcmp(*string, VERB_tokens_MULTI_LINE_COMMENT_START, VERB_tokens_MULTI_LINE_COMMENT_START_LEN)){
            nested_comment_ctr++;
            *string += VERB_tokens_MULTI_LINE_COMMENT_START_LEN;
            tokeniser->offset += VERB_tokens_MULTI_LINE_COMMENT_START_LEN;
        }
        else if(!memcmp(*string, VERB_tokens_MULTI_LINE_COMMENT_END, VERB_tokens_MULTI_LINE_COMMENT_END_LEN)){
            nested_comment_ctr--;
            *string += VERB_tokens_MULTI_LINE_COMMENT_END_LEN;
            tokeniser->offset += VERB_tokens_MULTI_LINE_COMMENT_END_LEN;
        }
        else{
            if(**string == '\n') VERB_bytecode_op_newline_proper_behaviour(tokeniser);
            else tokeniser->offset++;
            *string += 1;
        } 
    } while(**string && nested_comment_ctr);

    return VERB_TOKEN_special_IGNORE;
}

VERB_token_t VERB_tokens_op_single_line_comments(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// checks if string is terminated by \n or by \00.
// only works because single-line comments are 1-char long
    while(memcmp((*string)++, VERB_tokens_SINGLE_LINE_COMMENT_END, VERB_tokens_SINGLE_LINE_COMMENT_END_LEN)){
        if(!**string) return VERB_TOKEN_special_IGNORE;
    }

    VERB_bytecode_op_newline_proper_behaviour(tokeniser);
    
    return VERB_TOKEN_special_IGNORE;
}

#endif 
