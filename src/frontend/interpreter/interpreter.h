#ifndef VERB_frontend_interpreter_included
#define VERB_frontend_interpreter_included

#include "stddef.h"
#include "_includes.h"
#include "ops/_all_ops.h"
#include "interpreter_ctx.h"

static void VERB_interpret(char* const restrict code, const size_t codelen, VERB_rht_t* const restrict token_defs){

}

// interprets const-typed function
static void VERB_interpreter_execute_const_function(VERB_variable_function_t* const restrict function,VERB_variable_token_t* const restrict dst, const size_t dstcnt, VERB_variable_t** const restrict src, const size_t srccnt, VERB_rht_t* const restrict token_defs){
    for(size_t i = 0; i < srccnt; i++)
        function->src[i]->custom_data = src[i]->custom_data;

    VERB_interpret(function->code, function->codelen, token_defs);
}

#endif
