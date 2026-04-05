#ifndef VERB_compiler_included
#define VERB_compiler_included

#include "ops/ops_table.h"
#include "ops/includes.h"

// end symbols are expected to be 1, at most 2, so it's fine for this to be O(n)
// token ~ token to search within end_symbols array
// end_symbols ~ array of characters we should search through, return 1 if token is equal to any of them, 0 otherwise.
// end_symbols_cnt ~ number of end symbols
int VERB_compile_is_in_end_symbol_list(const VERB_token_t token, const VERB_token_t* const end_symbols, const size_t end_symbols_cnt){
    for(size_t i = 0; i < end_symbols_cnt; i++) 
        if(end_symbols[i] == token) return 1;
    return 0;
}

// returns NULL if error happens, ptr to new token if no error happens
void* VERB_frontend_compile(VERB_frontend_t* const restrict frontend, VERB_array_t* const restrict bytecode, char* tokens, const VERB_token_t* const restrict end_symbols, const size_t end_symbols_cnt, VERB_token_t* const restrict symbol_met){
    VERB_token_t cur_token;
    
    do{
        cur_token = VERB_token_read(&tokens);
        tokens = VERB_compiler_token_to_code[cur_token](frontend, bytecode, tokens);
        if(!tokens) return NULL;
    } while(!VERB_compile_is_in_end_symbol_list(cur_token, end_symbols, end_symbols_cnt));
// actual symbol that ended execution within end_symbols list
    *symbol_met = cur_token;
// returns updated position of ptr
    return tokens;
}

#endif
