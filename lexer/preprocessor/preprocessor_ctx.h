#ifndef VERB_lexer_preprocessor_ctx_included
#define VERB_lexer_preprocessor_ctx_included

#include "../../libraries/rhts/rht.h"                           // recursive hash table & ops.
#include "../../libraries/trees/boolean_tree.h"                 // boolean tree.
#include "../../libraries/trees/bytecode_tree.h"                // bytecode function tree.

typedef enum{
    VERB_preprocessor_if_none,
    VERB_preprocessor_if_true,
    VERB_preprocessor_if_false
} VERB_preprocessor_if_state_t;

typedef struct{
    VERB_tree_bytecode_t* directives;       // functions, function ptrs.
    VERB_tree_boolean_t* modules;           // included modules from given file.
    VERB_rht_t* defined_things;             // things defined with '#define'.
// ISSUE: you can't extract the modules you DIDN'T include from an RHT!
// 
    VERB_rht_t* once_included;              // hash of code guarded by #once ... #endonce statements.
    VERB_preprocessor_if_state_t if_state;
} VERB_preprocessor_t; 

#endif
