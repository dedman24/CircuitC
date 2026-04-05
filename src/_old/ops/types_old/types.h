#ifndef VERB_compiler_ops_types_included
#define VERB_compiler_ops_types_included

// types
#include "int.h"
#include "wire.h"

// type qualifiers
#include "mut.h"

// misc
#include "typedef.h"

// TYPES MAY BE HOWEVER MANY TOKENS LONG ONE WISHES THEM TO BE!
// currently no types longer than one token exist, adding one consists of adding a new token & passing that to 'VERB_compiler_op_type_handler'.
// example: say one wants to add a type called short int. 
// assuming you add a token for this new type, in the tokenisation stage it would become VERB_TOKEN_SHORT followed by VERB_TOKEN_INT.
// we want that sequence to encode a single type, which may be done as:
// defining a short.h file in this folder,
// including it in this file,
// adding 'VERB_TOKEN_SHORTINT' to the enum in 'tokens.h',
// in said file defining a function called 'VERB_compiler_op_shortint' witht he appropriate arguments
// putting said function at index 'VERB_TOKEN_SHORTINT' within the 'VERB_compiler_token_to_code' array defined in 'ops_table.h',
// having said function read the following token & return NULL in case the next one isn't a 'VERB_TOKEN_INT',
// having said function call 'VERB_compiler_op_type_handler' with, as type, 'VERB_TOKEN_SHORTINT'.
// in case one wishes to add new types that start with short, one would need to modify 'VERB_compiler_op_shortint'. so far it would only support 'int' & error out on any other type.
// make it so that adding new supported types starting with short is as easy as adding a new element to an array.
//
// in the time it took me to write this I could've done it myself lol.
// can type qualifiers be implemented in such a way? no, it'd be too complex; many type qualifiers can be applied to a single type.
// we need a separate type qualifier step.
// 
// two types of type qualifiers:
// prefix qualifiers -> implemented by setting type qualifier marker & 
// suffix qualifiers -> implemented through special function that decodes suffix qualifiers after type is specified


#endif
