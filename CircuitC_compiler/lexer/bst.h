#ifndef CIRCUITC_bst_included
#define CIRCUITC_bst_included

#include "stdlib.h"             // dynamic memory ops
#include "stdint.h"             // types
#include "string.h"             // strcmp
#include "stdlib.h"             // calloc

// all I need is an opaque type that I can use to search for values, and from which I can get values.
// more to it if it's fast, but CIRCUITC has very few keywords, so speed isn't that important.

typedef uintptr_t CIRCUITC_value_t;

typedef struct CIRCUITC_tree{
    struct CIRCUITC_tree* lt;
    struct CIRCUITC_tree* gt;
    char* key;
    CIRCUITC_value_t value;
} CIRCUITC_tree_t;

typedef enum{ CIRCUITC_tree_keep_key, CIRCUITC_tree_free_key } CIRCUITC_tree_options_t;
typedef enum{ CIRCUITC_tree_no_error, CIRCUITC_tree_error } CIRCUITC_tree_error_code_t;

void CIRCUITC_tree_destroy(CIRCUITC_tree_t* tree, CIRCUITC_tree_options_t freekey){
    if(!tree) return;

    if(tree->lt) CIRCUITC_tree_destroy(tree->lt, freekey);
    if(tree->gt) CIRCUITC_tree_destroy(tree->gt, freekey);

    if(freekey == CIRCUITC_tree_free_key) free(tree->key);
    free(tree);
}

CIRCUITC_tree_t* CIRCUITC_tree_node_make(char* key, CIRCUITC_value_t value){
    CIRCUITC_tree_t* node = calloc(1, sizeof(CIRCUITC_tree_t));

    node->key = key;
    node->value = value;

    return node;
}

void CIRCUITC_tree_node_destroy(CIRCUITC_tree_t* node, CIRCUITC_tree_options_t freekey){
    if(freekey == CIRCUITC_tree_free_key) free(node->key);
    free(node);
}

CIRCUITC_value_t CIRCUITC_tree_search(CIRCUITC_tree_t* tree, char* key, const size_t keylen, CIRCUITC_tree_error_code_t* error_code){
    int result;
    while(result = strncmp(key, tree->key, keylen)){ 
        if(result < 0 && tree->lt) tree = tree->lt;
        else if(result > 0 && tree->gt) tree = tree->gt;
        else{
            if(error_code) *error_code = CIRCUITC_tree_error;
            return 0;
        }
    }

    if(error_code) *error_code = CIRCUITC_tree_no_error;
    return tree->value;
}

// returns tree node that holds key if said key is present within tree, otherwise the tree node that would have held key newly allocated (if alloc_new_node is set), or NULL otherwise.
// if alloc_new_node is set, it never returns NULL.
// father ~ holds father to said node, AKA node that points to said node. if said node is root, father will be NULL. 
// father may not be passed, I.E. NULL may be passed in its place if one does not care for it.
// father_path ~ whether node is lt (-1) or gt (1) field of father. must be present if father is present, undefined value if node to search is root.
CIRCUITC_tree_t* CIRCUITC_tree_search_for_node(CIRCUITC_tree_t* tree, char* key, CIRCUITC_tree_t** father, int* father_path, int alloc_new_node){
    if(father) *father = NULL;

    int result;
    while(result = strcmp(key, tree->key)){
        if(father){ *father = tree; *father_path = result; }

        if(result < 0 && tree->lt) tree = tree->lt;
        else if(result > 0 && tree->gt) tree = tree->gt;
        else if(!alloc_new_node) return NULL;
        else{
            if(result < 0){ tree->lt = CIRCUITC_tree_node_make(key, 0); return tree->lt; }
            else{ tree->gt = CIRCUITC_tree_node_make(key, 0); return tree->gt; }
        }
    }
    return tree;
}

// searches for largest node in tree
CIRCUITC_tree_t* CIRCUITC_tree_search_largest(CIRCUITC_tree_t* tree, CIRCUITC_tree_t** father){
    if(father) *father = NULL;
    while(tree->gt){
        if(father) *father = tree; 
        tree = tree->gt;
    } 
    return tree;
}

void CIRCUITC_tree_put(CIRCUITC_tree_t** tree, char* key, CIRCUITC_value_t value){
    if(!*tree){
        *tree = CIRCUITC_tree_node_make(key, value);
        return;
    }
    
    CIRCUITC_tree_t* new_node = CIRCUITC_tree_search_for_node(*tree, key, NULL, NULL, 1);
    new_node->value = value;
}

// replaces replaced with replacer; basically moves everything from replacer to replaced, and frees replacer & replaced's key (if freekey is set)
void CIRCUITC_tree_node_replace(CIRCUITC_tree_t* replaced, CIRCUITC_tree_t* replacer, CIRCUITC_tree_options_t freekey){
    replaced->value = replacer->value;
    
    char* replaced_original_key = replaced->key;
    replaced->key = replacer->key;
    replacer->key = replaced_original_key;

    replaced->lt = replacer->lt;
    replaced->gt = replacer->gt;

    CIRCUITC_tree_node_destroy(replacer, freekey);
}

// instead of deleting root, does nothing.
void CIRCUITC_tree_del(CIRCUITC_tree_t* tree, char* key, CIRCUITC_tree_options_t freekey){
    int father_path;
    CIRCUITC_tree_t *father, *node_to_delete = CIRCUITC_tree_search_for_node(tree, key, &father, &father_path, 0);
    if(!node_to_delete) return;             // node does not exist

    if(node_to_delete->lt && node_to_delete->gt){           // replace node_to_delete with largest node in lt 
        CIRCUITC_tree_t *father_replacing_node, *replacing_node = CIRCUITC_tree_search_largest(node_to_delete->lt, &father_replacing_node);

        father_replacing_node->gt = replacing_node->lt;
        replacing_node->lt = NULL;
        CIRCUITC_tree_node_replace(node_to_delete, replacing_node, freekey);
    }
    else if(node_to_delete->lt || node_to_delete->gt){      // replace node_to_delete with one of the two child nodes
        CIRCUITC_tree_t* replacing_node;
        if(node_to_delete->lt) replacing_node = node_to_delete->lt;
        else replacing_node = node_to_delete->gt;

        CIRCUITC_tree_node_replace(node_to_delete, replacing_node, freekey);
    }  
    else if(father){                                        // deletes node_to_delete only if it isn't root, by setting its reference in father to NULL
        if(father_path > 0) father->gt = NULL;
        else father->lt = NULL;

        CIRCUITC_tree_node_destroy(node_to_delete, freekey);
    }
}

#endif
