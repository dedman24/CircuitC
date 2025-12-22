#ifndef CIRCUITC_lexer_included
#define CIRCUITC_lexer_included

#include "tokeniser.h"

// a lexer converts human-readable CircuitC code into a computer-readable representation of said code, encoded in TOKENS.
// example CircuitC code:
// w foo(w bar, w baz){
//      return bar & baz;
// }
// the lexer would conver this into (where <x> represents the token x)
// <w> <name> 3 ~ foo <(> <w> <name> 3 ~ bar <w> <name> 3 ~ baz <)> <{>
//      <return> <name> 3 ~ bar <&>  <name> 3 ~ baz <;>
// <}>
//
// tokens and the piece of syntax they define are held in tokens.h
// "preprocessor directives" are handled by the lexer.
// 
// the lexer returns a specialised error struct when it errors out. (TODO: THIS!)
//
// the lexer was also designed to be as general-purpose as possible. this means that it can be retargeted from one language onto another very easily in theory.


// given string of CircuitC code, converts it to string of tokens that has to be freed by user
void* CIRCUITC_lexer(char* string){

}

// TODO: REMOVE THIS!
int main(){
    return 0;
}

#endif
