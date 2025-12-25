#ifndef CIRCUITC_tokens_included
#define CIRCUITC_tokens_included

#include "stdint.h"             // variable-width types
#include "bst.h"                // bst type

// all CircuitC keywords as tokens
// notation:
// <expr> ~ expression
// <int>  ~ signed integer
// <uint> ~ unsigned integer

// new keywords defined by CircuitC:
//  w ~ generic wire type of undefined with
//  w(<expr>) ~ wire type of width expr
//  w<num> ~ wire type of width num
//  bool ~ another name for w1

typedef uint8_t CIRCUITC_token_t;

// DO NOT CHANGE THESE TOKENS UNLESS YOU KNOW WHAT YOU'RE DOING!
#define CIRCUITC_TOKEN_WHITESPACE                   0x00U
#define CIRCUITC_TOKEN_NEWLINE                      0x01U
#define CIRCUITC_TOKEN_EOF                          0x02U
#define CIRCUITC_TOKEN_NAME                         0x03U
#define CIRCUITC_TOKEN_VALUE                        0x04U
// you may change these if you want to add functionality to your language
#define CIRCUITC_TOKEN_INCLUDE                      0x05U
#define CIRCUITC_TOKEN_W                            0x10U           // width-less generic wire type

// DO NOT CHANGE THESE TOKENS UNLESS YOU KNOW WHAT YOU'RE DOING!
#define CIRCUITC_TOKEN_METADATA_EXTENDED         0x100U             // signals that, for a symbol x, xy is also another valid token, and thus one should check for it too.

#define CIRCUITC_TOKEN_METADATA_MASK                0xFF
#define CIRCUITC_TOKEN_METADATA_GET(tok)            ((tok)&~CIRCUITC_TOKEN_METADATA_MASK)


typedef struct{                                                     // struct used to define tokens
    char* symbol;
    uint8_t value;
} CIRCUITC_token_definition_t;

// all keywords
const CIRCUITC_token_definition_t CIRCUITC_keywords[] = {
    {"w", CIRCUITC_TOKEN_W},
// bla bla bla
// TODO: ADD ALL CIRCUITC TOKENS
};

// all comment symbols
// format is:
// symbol that opens comment, index of symbol that closes comment in CIRCUITC_comments_end
const CIRCUITC_token_definition_t CIRCUITC_comments_begin[] = {
    {"//", 0},                                                      // one-line comment opening symbol
    {"/*", 1},                                                      // multi-line comment opening/closing symbol
};

// closing comment symbols, counterparts of CIRCUITC_comments_begin
// other than these, a valid comment closing symbol is \x00.
// currently comments may only have one end per beginning (save for \x00 which closes all comment types)
// thus all comments are treated as a section of string enclosed by a beginning character string and an ending character string, which the lexer skips over when encountered.
const char* CIRCUITC_comments_end[] = {
    "\n",                                                           // closing counterpart of // (that is, the comment symbol // is closed with \n)
    "*/"                                                            // closing counterpart of /- (that is, the comment symbol /* is closed with */)
};

// all whitespaces
const CIRCUITC_token_definition_t CIRCUITC_whitespaces[] = {
    {" ",  CIRCUITC_TOKEN_WHITESPACE},                              // normal space
    {"\n", CIRCUITC_TOKEN_NEWLINE},                                 // newline, handled differently so that lexer knows what line an error occurs on
    {"\t", CIRCUITC_TOKEN_WHITESPACE},                              // horizontal tab
    {"\v", CIRCUITC_TOKEN_WHITESPACE},                              // vertical tab
    {"\r", CIRCUITC_TOKEN_WHITESPACE},                              // carriage return
    {"\f", CIRCUITC_TOKEN_WHITESPACE},                              // line feed
    {"\x00", CIRCUITC_TOKEN_EOF}                                    // end of file/human-readable string 0x00. DO NOT CHANGE!
};

CIRCUITC_tree_t* CIRCUITC_token_init(const CIRCUITC_token_definition_t* definition, const uint64_t definition_size){
    CIRCUITC_tree_t* syntax_tree = NULL;
// iterates over all syntax
    for(uint64_t i = 0; i < definition_size; i++)
        CIRCUITC_tree_put(&syntax_tree, definition[i].symbol, definition[i].value);

    return syntax_tree;
}

CIRCUITC_tree_t* CIRCUITC_token_keywords_init(){
    return CIRCUITC_token_init(CIRCUITC_keywords, sizeof(CIRCUITC_keywords)/sizeof(*CIRCUITC_keywords));
}

CIRCUITC_tree_t* CIRCUITC_token_comments_init(){
    return CIRCUITC_token_init(CIRCUITC_comments_begin, sizeof(CIRCUITC_comments_begin)/sizeof(*CIRCUITC_comments_begin));
}

CIRCUITC_tree_t* CIRCUITC_token_whitespaces_init(){
    return CIRCUITC_token_init(CIRCUITC_whitespaces, sizeof(CIRCUITC_whitespaces)/sizeof(*CIRCUITC_whitespaces));
}

#endif
