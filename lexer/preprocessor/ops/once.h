#ifndef VERB_preprocessor_ops_once_included
#define VERB_preprocessor_ops_once_included

// #once is to specify that a given file has to be included only once. 
// upon meeting #once, everything from the #once statement to #endonce HAS to be included only once.
// this means that the compiler has to track which files it has included & which it hasn't. 
// IDEA: compute H(code from this point to #endonce) & put that inside rht. 
// any collision-resistant hash function can be used. 
// I'm using blake2s because of it's speed, ease-of-use (literally 1 function), permissive licence & portability.
// why blake2s and not blake2b? so it runs better on 32-bit machines; the speed gained from blake2b doesn't matter when considering how slow everything else is.

#include "end/endonce.h"
#include "_includes.h"
#include "../../../libraries/BLAKE2s/blake2s.c"             // relevant hash function

#define VERB_PREPROCESSOR_HASH_SIZE 32

// checks that we haven't met H(code between #once and #endonce or \00) yet.
void VERB_preprocessor_op_once(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    char* const endonce_pos = VERB_preprocessor_skip_endonce(string, tokeniser);

    const size_t len = (size_t)(endonce_pos - *string);
    uint8_t hash[VERB_PREPROCESSOR_HASH_SIZE];
    blake2s(hash, VERB_PREPROCESSOR_HASH_SIZE, *string, len, NULL, 0);

    if(VERB_rht_search_and_put(tokeniser->preprocessor->once_included, hash, NULL, NULL, len, VERB_rht_destroy_none))
        *string = endonce_pos;
}

#endif
