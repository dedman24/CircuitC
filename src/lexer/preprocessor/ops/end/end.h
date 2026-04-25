#ifndef VERB_preprocessor_ops_end_included
#define VERB_preprocessor_ops_end_included

#include "../_includes.h"

// #end, when encountered, tells the compiler to stop parsing a file.

void VERB_preprocessor_op_end(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// simplest way to stop parsing a file is to skip to its terminator. 
    *string = *string + strlen(*string);
}

#endif
