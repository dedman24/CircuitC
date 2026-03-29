#ifndef VERB_tokens_all_ops_included
#define VERB_tokens_all_ops_included

#include "stdbool.h"

#include "comments.h"
#include "name_value.h"
#include "scoping_and_whitespace.h"
#include "token_eof.h"

void VERB_tokens_op_non_whitespace_character(VERB_tokeniser_t* const restrict tokeniser){
    if(tokeniser->scope_state != VERB_tokeniser_scope_state_none){
        tokeniser->scope_state = VERB_tokeniser_scope_state_none;
        if(tokeniser->old_whitespace_cnt > tokeniser->new_whitespace_cnt){
            for(unsigned int i = 0; i < tokeniser->old_whitespace_cnt - tokeniser->new_whitespace_cnt; i++)
                VERB_tokeniser_backend_scope_del(&tokeniser->backend);
        }
        else{
            for(unsigned int i = 0; i < tokeniser->new_whitespace_cnt - tokeniser->old_whitespace_cnt; i++)
                VERB_tokeniser_backend_scope_new(&tokeniser->backend);
        }
        tokeniser->old_whitespace_cnt = tokeniser->new_whitespace_cnt;
        tokeniser->new_whitespace_cnt = 0;
    }
}

#endif
