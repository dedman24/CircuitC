#ifndef VERB_tokeniser_ctx_included
#define VERB_tokeniser_ctx_included

#include "backend.h"
#include "stdbool.h"                                                // for boolean type.
#include "tokeniser_type.h"
#include "../../../libraries/arrays/dynamic_arrays.h"
#include "../../../libraries/errors_warnings/errors_warnings.h"     // for error handling type.
#include "../../preprocessor/preprocessor_ctx.h"

// do NOT change the values of these!
typedef enum{
    VERB_tokeniser_state_none                               = 0,
    VERB_tokeniser_state_scope_after_newline                = 1,
    VERB_tokeniser_state_scope_after_period                 = 2,
    VERB_tokeniser_state_scope_after_newline_and_period     = 3,
    VERB_tokeniser_state_after_operator                     = 4,
    VERB_tokeniser_state_priv                               = 8,
} VERB_tokeniser_state_t;

#define VERB_TOKENISER_STATE_SCOPE_MASK (VERB_tokeniser_state_scope_after_newline | VERB_tokeniser_state_scope_after_period)

#define VERB_tokeniser_state_assert(tokeniser, new_state)                   \
    do{                                                                     \
        (tokeniser)->state |= (new_state);                                  \
    } while(0)

#define VERB_tokeniser_state_toggle(tokeniser, new_state)                   \
    do{                                                                     \
        (tokeniser)->state ^= (new_state);                                  \
    } while(0)

#define VERB_tokeniser_state_remove(tokeniser, new_state)                   \
    do{                                                                     \
        (tokeniser)->state &= ~(new_state);                                 \
    } while(0)

#define VERB_tokeniser_state_check(tokeniser, tocheck) ((tokeniser)->state & (tocheck))

// persistent data that has to be tracked when tokenising other files recursively.
typedef struct{
    VERB_tree_boolean_t* modules;               // for preprocessor; bst of modules to include.
// CURRENT POSITION WITHIN FILE
    unsigned long long line;                    // current line.
    unsigned long long offset;                  // offset/char num within said line.
// RESULTION OF SCOPES
    unsigned int old_whitespace_cnt;            // scoping depth of previous line.
    unsigned int new_whitespace_cnt;            // scoping depth of this line.
    VERB_tokeniser_state_t state;               // state of tokeniser.
} VERB_tokeniser_persistent_data_t;

// all tokens
struct VERB_tokeniser {
    VERB_tree_bytecode_t* keywords;             // holds bst of all keyword strings.
    VERB_tree_bytecode_t* comments;             // holds bst of all comment strings.
    VERB_tree_bytecode_t* whitespaces;          // holds bst of all whitespace characters.
    VERB_preprocessor_t* preprocessor;          // holds all preprocessor statements, all macros & so on.
    VERB_array_t* code;                         // holds tokenised code.
// handling of .
    VERB_tokeniser_backend_t backend;           // backend data.
    VERB_array_t ops;                           // stack of all ops.
    VERB_array_t values;                        // stack of all values.
// PASSING/RETURNING EXTRA ARGUMENTS
    // ERROR HANDLING
    VERB_error_specifics_t* specifics;          // used to return errors and warnings.
    // OTHER
    size_t nameval_length;                      // used to pass extra state from get_token to function it ends up calling. consider removing.
// CURRENT POSITION WITHIN FILE
    unsigned long long line;                    // current line.
    unsigned long long offset;                  // offset/char num within said line.
    unsigned int bracket_depth;
// RESULTION OF SCOPES
    unsigned int old_whitespace_cnt;            // scoping depth of previous line.
    unsigned int new_whitespace_cnt;            // scoping depth of this line.
    VERB_tokeniser_state_t state;
};

void VERB_tokeniser_persistent_data_populate(VERB_tokeniser_persistent_data_t* persistent, VERB_tokeniser_t* tokeniser){
    persistent->modules = tokeniser->preprocessor->modules;
    tokeniser->preprocessor->modules = NULL;                    // HAS to be done, or modules might conflict across files!!!
    
    persistent->line = tokeniser->line;
    persistent->offset = tokeniser->offset;

    persistent->old_whitespace_cnt = tokeniser->old_whitespace_cnt;
    persistent->new_whitespace_cnt = tokeniser->new_whitespace_cnt;
    persistent->state = tokeniser->state;
}

// VERB_tree_destroy(tokeniser->preprocessor->modules, VERB_tree_keep_key); HAS to be called externally.
void VERB_tokeniser_persistent_data_depopulate(VERB_tokeniser_t* tokeniser, VERB_tokeniser_persistent_data_t* persistent){
//    if(tokeniser->preprocessor->modules) VERB_tree_destroy(tokeniser->preprocessor->modules, VERB_tree_keep_key);
    tokeniser->preprocessor->modules = persistent->modules;

    tokeniser->line = persistent->line;
    tokeniser->offset = persistent->offset;

    tokeniser->old_whitespace_cnt = persistent->old_whitespace_cnt;
    tokeniser->new_whitespace_cnt = persistent->new_whitespace_cnt;
    tokeniser->state = persistent->state;
}

#endif
