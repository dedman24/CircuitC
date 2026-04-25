#ifndef VERB_library_bytecode_tree_included
#define VERB_library_bytecode_tree_included

#include "tree_builder.h"
#include "../../tokens/token_type.h"                        // actual token type.
#include "../../lexer/tokeniser/backend/tokeniser_type.h"   // for tokeniser ctx type.


VERB_tree_make(bytecode, VERB_token_fun_t);


#endif
