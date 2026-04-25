#ifndef VERB_tokeniser_variables_complextypes_included
#define VERB_tokeniser_variables_complextypes_included

#include "variable_definition.h"
#include "../../../libraries/rhts/rht.h"
#include "../../../libraries/trees/boolean_tree.h"

typedef enum{
    VERB_VARIABLE_COMPLEX_FLAG_none                     = 0,
    VERB_VARIABLE_COMPLEX_FLAG_implicitalways           = 1,        // to always convert to the type it aliases in any operation.
} VERB_variable_complex_flags_t;

typedef struct{
    VERB_rht_t* fields;                                             // fields indexed by their name, points to said fields' VERB_variable_t thingy.
    VERB_variable_t* simple_type_alias;                             // ptr to variable the type aliases to.
    VERB_tree_boolean_t* implcit_objects;                           // implicit objects it can be casted to.
    VERB_variable_complex_flags_t flags;                            // complex type flags.
} VERB_variable_complex_t;

static VERB_variable_t* VERB_variable_complex_search_byName(VERB_variable_complex_t* const restrict var, char* const restrict fieldname, const size_t fieldlen){
    return VERB_rht_search(var->fields, fieldname, fieldlen);
}

static bool VERB_variable_complex_check_implicit_tags__recursive(VERB_tree_boolean_t* const restrict tree0, VERB_tree_boolean_t* const restrict tree1){
    if(tree0->child[0] && VERB_variable_complex_check_implicit_tags__recursive(tree0->child[0], tree1)) return true;
    if(tree0->child[1] && VERB_variable_complex_check_implicit_tags__recursive(tree0->child[1], tree1)) return true;
    return false;
}

// O(nlogn) algorithm, checks if any of the entries in tree0 are also in tree1, returns true if so.
static bool VERB_variable_complex_check_implicit_tags(VERB_tree_boolean_t* const restrict tree0, VERB_tree_boolean_t* const restrict tree1){
    if(!tree0) return false;
    if(tree0->child[0] && VERB_variable_complex_check_implicit_tags__recursive(tree0->child[0], tree1)) return true;
    if(tree0->child[1] && VERB_variable_complex_check_implicit_tags__recursive(tree0->child[1], tree1)) return true;
    return false;
}

#endif
