#ifndef VERB_tokeniser_ctx_included
#define VERB_tokeniser_ctx_included

#include "backend.h"
#include "stdbool.h"                                                // for boolean type.
#include "tokeniser_type.h"
#include "../../../libraries/regex_stuff.h"
#include "../../../libraries/trees/ptr_tree.h"
#include "../../preprocessor/preprocessor_ctx.h"
#include "../../../libraries/arrays/dynamic_arrays.h"
#include "../../../libraries/errors_warnings/errors_warnings.h"     // for error handling type.

// do NOT change the values of these!
typedef enum{
    VERB_tokeniser_state_none                               = 0,
    VERB_tokeniser_state_after_operator                     = 1,
    VERB_tokeniser_state_priv                               = 2,
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

typedef VERB_array_t VERB_tokeniser_path_t;

#define VERB_tokeniser_path_push(src, srclen, path)                                             \
    do{                                                                                         \
        VERB_array_remove_type(1, char, &(path)); /* remove prev \0 */                          \
        VERB_array_push_char(0, &(path));                                                       \
        VERB_array_push_string(src, srclen, &(path));                                           \
    } while(0)

#define VERB_tokeniser_path_pop(srclen, path)                                                   \
    do{                                                                                         \
        VERB_array_remove_type(srclen + 1, char, &(path));                                      \
        VERB_array_push_char(0, &(path));                                                       \
    } while(0)

#define VERB_tokeniser_path_get(path) ((path).arr)

// all tokens
struct VERB_tokeniser {
    VERB_tree_bytecode_t* keywords;             // holds bst of all keyword strings.
    VERB_tree_bytecode_t* comments;             // holds bst of all comment strings.
    VERB_tree_bytecode_t* whitespaces;          // holds bst of all whitespace characters.
    VERB_tree_ptr_t* overloaded_operators;      // holds all overloaded operators + the types said overload may be applied to.
    VERB_tree_ptr_t* stdlibrary_file_paths;     // tree that holds stdlibrary paths: real paths on system correspondences.
    VERB_preprocessor_t* preprocessor;          // holds all preprocessor statements, all macros & so on.
    VERB_array_t* code;                         // holds tokenised code.
// handling of .
    VERB_tokeniser_backend_t backend;           // backend data.
    VERB_array_t ops;                           // stack of all ops.
    VERB_array_t values;                        // stack of all values.
    VERB_tokeniser_path_t path;                 // path being processed.
// PASSING/RETURNING EXTRA ARGUMENTS
    // ERROR HANDLING
    VERB_error_specifics_t* specifics;          // used to return errors and warnings.
    // OTHER
    size_t nameval_length;                      // used to pass extra state from get_token to function it ends up calling. consider removing.
// CURRENT POSITION WITHIN FILE
    unsigned long long line;                    // current line.
    unsigned long long offset;                  // offset/char num within said line.
    unsigned long long bracket_depth;           // number of brackets processed so far.
// RESULTION OF SCOPES
    unsigned int old_whitespace_cnt;            // scoping depth of previous line.
    unsigned int new_whitespace_cnt;            // scoping depth of this line.
    VERB_tokeniser_state_t state;
};

static void VERB_tokeniser_persistent_data_populate(VERB_tokeniser_persistent_data_t* persistent, VERB_tokeniser_t* tokeniser){
    persistent->modules = tokeniser->preprocessor->modules;
    tokeniser->preprocessor->modules = NULL;                    // HAS to be done, or modules might conflict across files!!!
    
    persistent->line = tokeniser->line;
    persistent->offset = tokeniser->offset;

    persistent->old_whitespace_cnt = tokeniser->old_whitespace_cnt;
    persistent->new_whitespace_cnt = tokeniser->new_whitespace_cnt;
    persistent->state = tokeniser->state;
}

// VERB_tree_destroy(tokeniser->preprocessor->modules, VERB_tree_keep_key); HAS to be called externally.
static void VERB_tokeniser_persistent_data_depopulate(VERB_tokeniser_t* tokeniser, VERB_tokeniser_persistent_data_t* persistent){
//    if(tokeniser->preprocessor->modules) VERB_tree_destroy(tokeniser->preprocessor->modules, VERB_tree_keep_key);
    tokeniser->preprocessor->modules = persistent->modules;

    tokeniser->line = persistent->line;
    tokeniser->offset = persistent->offset;

    tokeniser->old_whitespace_cnt = persistent->old_whitespace_cnt;
    tokeniser->new_whitespace_cnt = persistent->new_whitespace_cnt;
    tokeniser->state = persistent->state;
}

static void VERB_tokeniser_operator(VERB_tokeniser_t* const tokeniser){
    VERB_tokeniser_state_assert(tokeniser, VERB_tokeniser_state_after_operator);
}

static void VERB_tokeniser_not_operator(VERB_tokeniser_t* const tokeniser){
    VERB_tokeniser_state_remove(tokeniser, VERB_tokeniser_state_after_operator);
}

#define VERB_tokeniser_skip_chars(string, tokeniser, cnt)                       \
    do{                                                                         \
        (tokeniser)->offset += cnt;                                             \
        *(string) += cnt;                                                       \
    } while(0)

#define VERB_tokeniser_skip_whitespace(string, tokeniser)                       \
    do{                                                                         \
        const size_t whitespaces = VERB_REGEX_whitespace_length(*(string));     \
        *(string) += whitespaces; (tokeniser)->offset += whitespaces;           \
    } while(0)

void VERB_bytecode_op_newline_proper_behaviour(VERB_tokeniser_t*);

static char* VERB_tokeniser__skip_to_end(VERB_tokeniser_t* const restrict tokeniser, char* string, char* end){
    while(string < end){
        if(*string == '\n'){ VERB_bytecode_op_newline_proper_behaviour(tokeniser); string++; }
        size_t skipped = VERB_REGEX_not_newline_length(string);
        
        string += skipped;
        if(string > end){
            const size_t overshoot = (size_t)(string - end);
            skipped -= overshoot;
        }
        tokeniser->offset += skipped;
    }

    return end;
}

// tokeniser-safe strchr that takes into account newline stuff.
static char* VERB_tokeniser__strchr(VERB_tokeniser_t* const restrict tokeniser, char* restrict string, const char toskipto){
    char* endofstring = strchr(string, toskipto);
    return VERB_tokeniser__skip_to_end(tokeniser, string, endofstring);
}

static char* VERB_tokeniser__skip_to_end_brackets(VERB_tokeniser_t* const restrict tokeniser, char* string, char* end){
    const unsigned long long initial_bracket_depth = tokeniser->bracket_depth;
    while(
        string < end &&
        (tokeniser->bracket_depth != initial_bracket_depth || *string != ')')
    ){
        if(*string == '\n'){ VERB_bytecode_op_newline_proper_behaviour(tokeniser); string++; }
        const size_t skipped_newline = VERB_REGEX_not_newline_length(string);
        const size_t skipped_bracket = (size_t)(strchr(string, ')') - string);
        size_t skipped = skipped_newline > skipped_bracket? skipped_newline: skipped_bracket;

        string += skipped;
        if(string > end){
            const size_t overshoot = (size_t)(string - end);
            skipped -= overshoot;
        }
        tokeniser->offset += skipped;
    }

    return end;
}

// tokeniser-safe strchr that takes into account newline stuff and bracket depth.
static char* VERB_tokeniser__strchr_brackets(VERB_tokeniser_t* const restrict tokeniser, char* restrict string, const char toskipto){
    char* endofstring = strchr(string, toskipto);
    return VERB_tokeniser__skip_to_end_brackets(tokeniser, string, endofstring);
}

// tokeniser-safe strchr that takes into account newline stuff, for multiple characters.
static char* VERB_tokeniser__strchr_many(VERB_tokeniser_t* const restrict tokeniser, char* restrict string, const char* const restrict toskipto){
    char* endofstring = string + strcspn(string, toskipto);
    return VERB_tokeniser__skip_to_end(tokeniser, string, endofstring);
}

// tokeniser-safe strstr that takes into account newline stuff. skips over everything until it reaches the target string.
// unlike C's strstr, this one skips over the substring it's meant to find.
static char* VERB_tokeniser__strstr(VERB_tokeniser_t* const restrict tokeniser, char* restrict string, const char* const restrict toskipto, const char earlyterm){
    char* end = strstr(string, toskipto);
    char* earlyend = strchr(string, earlyterm);
    if(!end && !earlyend) return NULL;
    if(!end || earlyend < end) return earlyend;
    end += strlen(toskipto);                                                // skips over substring.
    return VERB_tokeniser__skip_to_end(tokeniser, string, end);
}

#endif
