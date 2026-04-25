#ifndef VERB_library_tree_builder_included
#define VERB_library_tree_builder_included

#include "string.h"                                         // memcmp.
#include "stdlib.h"                                         // dynamic memory operations.
#include "stdint.h"                                         // types.
#include "stdbool.h"                                        // boolean type.

// all I need is an opaque type that I can use to search for values, and from which I can get values.
// more to it if it's fast, but all programming languages have few keywords (<200), so speed isn't that important.

// this file allows one to build a binary search tree of any type one wishes.

// given a type x and a user-defined name for said tree xxx, this library exposes the following:
// 
//      VERB_tree_xxx_t                         // type of tree.
//      VERB_tree_xxx_destroy                   // destroys tree.
//      VERB_tree_xxx_search                    // searches for element in tree, returns element.
//      VERB_tree_xxx_search_bool               // searches for element in tree, returns true if element is in tree, false otherwise.
//      VERB_tree_xxx_put                       // puts element in tree.
//      VERB_tree_xxx_del                       // deletes element from tree.
//
// since it's a binary tree, all these run in O(logn).
// my overreliance on dogwater binary trees will kill me. 
// I mean, it's a simple algorithm; unless one can prove THIS is at fault then I'll keep using it.
// I want a generic interface for search algorithms that hides the actual algorithm :PPP.

// stuff outside tree builder.

#define VERB_tree_min(x, y) ((x) > (y)? (y): (x))

enum{
    VERB_tree_lt,
    VERB_tree_gt
};

typedef enum{ VERB_tree_keep_key, VERB_tree_free_key } VERB_tree_options_t;

// builds a tree that holds a given type.
#define VERB_tree_make(typename, type)                                                                                                          \
typedef struct VERB_tree_##typename{                                                                                                            \
    struct VERB_tree_##typename* child[2];                                                                                                      \
    char* key;                                                                                                                                  \
    size_t keylen;                                                                                                                              \
    type value;                                                                                                                                 \
} VERB_tree_##typename##_t;                                                                                                                     \
\
void VERB_tree_##typename##_destroy(VERB_tree_##typename##_t* tree, const VERB_tree_options_t freekey){                                         \
    if(!tree) return;                                                                                                                           \
    \
    VERB_tree_##typename##_destroy(tree->child[VERB_tree_lt], freekey);                                                                         \
    VERB_tree_##typename##_destroy(tree->child[VERB_tree_gt], freekey);                                                                         \
    \
    if(freekey == VERB_tree_free_key) free(tree->key);                                                                                          \
    free(tree);                                                                                                                                 \
}   \
\
VERB_tree_##typename##_t* VERB_tree_##typename##_node_make(char* key, const size_t keylen, type value){                                         \
    VERB_tree_##typename##_t* node = calloc(1, sizeof(VERB_tree_##typename##_t));                                                               \
    \
    node->key = key;                                                                                                                            \
    node->keylen = keylen;                                                                                                                      \
    node->value = value;                                                                                                                        \
    \
    return node;                                                                                                                                \
}   \
\
void VERB_tree_##typename##_node_destroy(VERB_tree_##typename##_t* const node, const VERB_tree_options_t freekey){                              \
    if(freekey == VERB_tree_free_key) free(node->key);                                                                                          \
    free(node);                                                                                                                                 \
}   \
\
type VERB_tree_##typename##_search(const VERB_tree_##typename##_t* restrict tree, const char* const restrict key, const size_t keylen){         \
    int result;                                                                                                                                 \
    while((result = memcmp(key, tree->key, VERB_tree_min(tree->keylen, keylen)))){                                                              \
        tree = tree->child[result > 0];                                                                                                         \
        if(!tree) return ((type)0);                                                                                                             \
    }                                                                                                                                           \
    \
    return tree->value;                                                                                                                         \
}   \
\
bool VERB_tree_##typename##_search_bool(const VERB_tree_##typename##_t* restrict tree, const char* const restrict key, const size_t keylen){    \
    int result;                                                                                                                                 \
    while((result = memcmp(key, tree->key, VERB_tree_min(tree->keylen, keylen)))){                                                              \
        tree = tree->child[result > 0];                                                                                                         \
        if(!tree) return false;                                                                                                                 \
    }                                                                                                                                           \
    \
    return true;                                                                                                                                \
}   \
\
VERB_tree_##typename##_t* VERB_tree_##typename##_search_for_node(VERB_tree_##typename##_t* tree, char* restrict key, const size_t keylen, VERB_tree_##typename##_t** father, int* father_path, const bool alloc_new_node){  \
    if(father) *father = NULL;                                                                                                                  \
    \
    int result;                                                                                                                                 \
    while((result = memcmp(key, tree->key, VERB_tree_min(tree->keylen, keylen)))){                                                              \
        if(father){ *father = tree; *father_path = result; }                                                                                    \
    \
        VERB_tree_##typename##_t* const restrict child = tree->child[result > 0];                                                               \
    \
        if(!child){                                                                                                                             \
            if(!alloc_new_node) return NULL;                                                                                                    \
            tree->child[result > 0] = VERB_tree_##typename##_node_make(key, keylen, 0);                                                         \
            return tree->child[result > 0];                                                                                                     \
        }                                                                                                                                       \
        tree = child;                                                                                                                           \
    }                                                                                                                                           \
    return tree;                                                                                                                                \
}   \
\
VERB_tree_##typename##_t* VERB_tree_##typename##_search_largest(VERB_tree_##typename##_t* restrict tree, VERB_tree_##typename##_t* restrict* father){\
    if(father) *father = NULL;                                                                                                                  \
    while(tree->child[VERB_tree_gt]){                                                                                                           \
        if(father) *father = tree;                                                                                                              \
        tree = tree->child[VERB_tree_gt];                                                                                                       \
    }                                                                                                                                           \
    return tree;                                                                                                                                \
}   \
\
void VERB_tree_##typename##_put(VERB_tree_##typename##_t** restrict tree, char* restrict key, const size_t keylen, type value){                 \
    if(!*tree){                                                                                                                                 \
        *tree = VERB_tree_##typename##_node_make(key, keylen, value);                                                                           \
        return;                                                                                                                                 \
    }   \
    \
    VERB_tree_##typename##_t* new_node = VERB_tree_##typename##_search_for_node(*tree, key, keylen, NULL, NULL, true);                          \
    new_node->value = value;                                                                                                                    \
}   \
\
void VERB_tree_##typename##_node_replace(VERB_tree_##typename##_t* const restrict replaced, VERB_tree_##typename##_t* const restrict replacer, const VERB_tree_options_t freekey){\
    replaced->value = replacer->value;                                                                                                          \
    \
    char* replaced_original_key = replaced->key;                                                                                                \
    replaced->key = replacer->key;                                                                                                              \
    replacer->key = replaced_original_key;                                                                                                      \
    \
    replaced->child[VERB_tree_lt] = replacer->child[VERB_tree_lt];                                                                              \
    replaced->child[VERB_tree_gt] = replacer->child[VERB_tree_gt];                                                                              \
    \
    VERB_tree_##typename##_node_destroy(replacer, freekey);                                                                                     \
}   \
\
bool VERB_tree_##typename##_del(VERB_tree_##typename##_t* restrict* const restrict tree, char* const restrict key, const size_t keylen, VERB_tree_options_t freekey){\
    int father_path;                                                                                                                            \
    VERB_tree_##typename##_t *father;                                                                                                           \
    VERB_tree_##typename##_t *node_to_delete =                                                                                                  \
        VERB_tree_##typename##_search_for_node(*tree, key, keylen, &father, &father_path, false);                                               \
    if(!node_to_delete) return false;                                                                                                           \
    \
    if(node_to_delete->child[VERB_tree_lt] && node_to_delete->child[VERB_tree_gt]){                                                             \
        VERB_tree_##typename##_t* restrict father_replacing_node;                                                                               \
        VERB_tree_##typename##_t* const restrict replacing_node =                                                                               \
            VERB_tree_##typename##_search_largest(node_to_delete->child[VERB_tree_lt], &father_replacing_node);                                 \
    \
        father_replacing_node->child[VERB_tree_gt] = replacing_node->child[VERB_tree_lt];                                                       \
        replacing_node->child[VERB_tree_lt] = NULL;                                                                                             \
        VERB_tree_##typename##_node_replace(node_to_delete, replacing_node, freekey);                                                           \
    }   \
    else if(node_to_delete->child[VERB_tree_lt] || node_to_delete->child[VERB_tree_gt]){                                                        \
        VERB_tree_##typename##_t* replacing_node;                                                                                               \
        replacing_node = node_to_delete->child[VERB_tree_lt]?                                                                                   \
            node_to_delete->child[VERB_tree_lt]: node_to_delete->child[VERB_tree_gt];                                                           \
        \
        VERB_tree_##typename##_node_replace(node_to_delete, replacing_node, freekey);                                                           \
    }   \
    else if(father){                                                                                                                            \
        father->child[father_path > VERB_tree_lt] = NULL;                                                                                       \
        VERB_tree_##typename##_node_destroy(node_to_delete, freekey);                                                                           \
    }   \
    else{   \
        VERB_tree_##typename##_node_destroy(*tree, freekey);                                                                                    \
        *tree = NULL;                                                                                                                           \
    }   \
    \
    return true;                                                                                                                                \
}

#endif
