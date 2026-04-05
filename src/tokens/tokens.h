#ifndef VERB_tokens_included
#define VERB_tokens_included

#include "stdint.h"                                         // variable-width types
#include "stddef.h"                                         // size_t, ssize_t types
#include "token_type.h"                                     // actual token type
#include "token_processing.h"                               // ops that process tokens
#include "../lexer/ops/_all_ops.h"                          // all token operations
#include "../libraries/trees/bytecode_tree.h"               // tree type & ops.
#include "../libraries/arrays/dynamic_arrays.h"             // dynamic array ops
#include "../lexer/tokeniser/backend/tokeniser_type.h"      // tokeniser type

typedef struct{                                                     // struct used to define tokens
    char* symbol;
    VERB_token_fun_t fun;
} VERB_token_definition_t;

// all keywords
const VERB_token_definition_t VERB_keywords[] = {
    {"NULL", NULL}
// bla bla bla
// TODO: ADD ALL VERB KEYWORDS!
};

// all comment symbols
// format is:
// symbol that opens comment, index of symbol that closes comment in VERB_comments_end
const VERB_token_definition_t VERB_comments_begin[] = {
    {"//", VERB_bytecode_op_single_line_comments},                    // one-line comment opening symbol
    {"/*", VERB_bytecode_op_multi_line_comments},                     // multi-line comment opening/closing symbol
};

// all whitespaces
const VERB_token_definition_t VERB_whitespaces[] = {
    {" ",    VERB_bytecode_op_whitespace},                          // normal space
    {"\t",   VERB_bytecode_op_whitespace},                          // horizontal tab
    {"\v",   VERB_bytecode_op_whitespace},                          // vertical tab
    {"\r",   VERB_bytecode_op_whitespace},                          // carriage return
    {"\f",   VERB_bytecode_op_whitespace},                          // line feed
    {"\n",   VERB_bytecode_op_newline},
    {"\x00", VERB_bytecode_op_eof}                                  // end of file/human-readable string 0x00. DO NOT CHANGE!
};

VERB_tree_bytecode_t* VERB_token_init(const VERB_token_definition_t* definition, const uint64_t definition_size){
    VERB_tree_bytecode_t* syntax_tree = NULL;
// iterates over all syntax
    for(uint64_t i = 0; i < definition_size; i++)
        VERB_tree_bytecode_put(&syntax_tree, definition[i].symbol, strlen(definition[i].symbol), definition[i].fun);

    return syntax_tree;
}

VERB_tree_bytecode_t* VERB_token_keywords_init(){
    return VERB_token_init(VERB_keywords, sizeof(VERB_keywords)/sizeof(*VERB_keywords));
}

VERB_tree_bytecode_t* VERB_token_comments_init(){
    return VERB_token_init(VERB_comments_begin, sizeof(VERB_comments_begin)/sizeof(*VERB_comments_begin));
}

VERB_tree_bytecode_t* VERB_token_whitespaces_init(){
    return VERB_token_init(VERB_whitespaces, sizeof(VERB_whitespaces)/sizeof(*VERB_whitespaces));
}

#endif
