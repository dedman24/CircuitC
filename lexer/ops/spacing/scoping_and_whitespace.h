#ifndef VERB_bytecode_op_scoping_and_whitespace_included
#define VERB_bytecode_op_scoping_and_whitespace_included

#include "stdbool.h"
#include "../_includes.h"

// scoping in VERB is both through INDENTATION (implicit scope markers) and SPECIFIC MARKERKS (explicit scope markers, being { for opening and } for closing).
// the lexer's job is to take all implicit scope references & turn them explicit.
// this means that something like:
// a
//  b
//   c
// is converted into
// a
// {b
// {c}}
// this is all to reduce verbosity.
// I see scoping through ONLY indendation as too limiting & scoping through explicit markers too verbose.

void VERB_bytecode_op_whitespace(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(tokeniser->state == VERB_tokeniser_state_scope_after_newline_and_period) tokeniser->new_whitespace_cnt++;
}

void VERB_bytecode_op_newline_proper_behaviour(VERB_tokeniser_t* const restrict tokeniser){
    tokeniser->line++;
    tokeniser->offset = 0;
}

void VERB_bytecode_op_newline(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_bytecode_op_newline_proper_behaviour(tokeniser);
    VERB_tokeniser_state_assert(tokeniser, VERB_tokeniser_state_scope_after_newline);
}

void VERB_bytecode_op_opened_curly_bracket(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_tokeniser_backend_scope_new(&tokeniser->backend);
}

void VERB_bytecode_op_closed_curly_bracket(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_tokeniser_backend_scope_del(&tokeniser->backend);
}

void VERB_bytecode_op_period(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    VERB_tokeniser_state_assert(tokeniser, VERB_tokeniser_state_scope_after_period);
}

#endif
