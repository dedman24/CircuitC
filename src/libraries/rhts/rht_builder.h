#ifndef VERB_library_rht_builder_included
#define VERB_library_rht_builder_included

#include "stdint.h"             // variable-width integer types
#include "string.h"             // memset, memcpy, memcmp & so on
#include "stdlib.h"             // dynamic memory operations
#include "stdbool.h"            // boolean type

// Recursive Hash Tables are data structures great at storing random values or ever-increasing numerical IDs.
// the RHT builder library provides one a standard way of building many different rhts optimised to one's wishes.
// rht.h provides the most generic rht ever, that can hold many objects of different types each with a different destructor, of various variable key lengths.
// this comes at a cost of requiring 40 bytes/element, which is awful.
// 

#define VERB_rht_size 16
#define VERB_rht_destroy_obj true

// constants user has to create:
// VERB_rht_ typename _size         ~ size of rht; MUST be either 16 or 256.
// VERB_rht_destroy_obj             ~ whether or not to destroy obj; MUST be either true (destroy) or false (do not destoy).

#define VERB_rht_build_nokeylen(typename, keytype, objtype, table_size, objdestructor)

// placeholders.
typedef uint64_t VERB_rht_keytype_t;
typedef void* VERB_rht_objtype_t;

typedef struct VERB_rht{
    struct VERB_rht* rht;
    VERB_rht_keytype_t key;
    VERB_rht_objtype_t obj;
} VERB_rht_t;

// must be able to hold VERB_rht_size distinct values.
typedef uint8_t VERB_rht_index_t;

void objdestructor(void*);
/* void keydestructor(void*); */

typedef enum{ VERB_rht_keep, VERB_rht_free } VERB_rht_options_t;

static VERB_rht_index_t VERB_rht_get_index(VERB_rht_keytype_t key, const uint32_t r){
/* evaluated at compile time */
    if(VERB_rht_size == 16) return key >> (r*16) & 0x0f;
    else return key >> (r*8) & 0xFF;
}

static VERB_rht_t* VERB_rht_get_entry(VERB_rht_t* restrict rht, VERB_rht_keytype_t key, const uint32_t r){
    return &rht[VERB_rht_get_index(key, r)];
}

void* VERB_rht_init(void){
    return calloc(VERB_rht_size, sizeof(VERB_rht_t));
}

void VERB_rht_destroy(VERB_rht_t* rht, const VERB_rht_options_t freeptr){
    for(uint64_t i = 0; i < VERB_rht_size; i++){
        if(rht[i].rht) VERB_rht_destroy(rht[i].rht, VERB_rht_free);
        if(VERB_rht_destroy_obj) objdestructor(rht[i].obj);
    }

    if(freeptr == VERB_rht_free) free(rht);
    else memset(rht, 0, sizeof(*rht)*VERB_rht_size);
}

void VERB_rht_put_entry(VERB_rht_t* const restrict entry, VERB_rht_keytype_t key, VERB_rht_objtype_t obj){
    entry->key = key;
    entry->obj = obj;
}

// only difference with put is that put_recursive accepts a parameter to init r.
void VERB_rht_put_recursive(VERB_rht_t* restrict rht, VERB_rht_keytype_t key, VERB_rht_objtype_t obj, const uint32_t rinit){
    for(uint32_t r = rinit; r < sizeof(key); r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(!entry->key){                                                    // entry is free
            VERB_rht_put_entry(entry, key, obj);
            return;
        }
        if(entry->key == key){                                              // we already put this value inside the rht ~ we only update object
            if(obj != entry->obj) objdestructor(obj);
            entry->obj = obj;
            return;
        }
        if(r != sizeof(key) - 1){                                           // entry already has an element ~ we build a sub-rht and we put in that one 
            if(!entry->rht) entry->rht = VERB_rht_init();
            VERB_rht_put_recursive(entry->rht, key, obj);
            return;
        }
        else{                                                               // that can only be done when r != keylen-1; if that's the case, we do the inverse
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);               // we still build the subrht but we put the entry originally at this location in it & it with ours
            VERB_rht_put_recursive(entry->rht, entry->key, entry->obj.ptr, entry->obj.destructor, entry->keylen, destroy_list, r+1); 
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return;
        }
    }
}

void VERB_rht_put(VERB_rht_t* restrict rht, void* const restrict key, void* const restrict ptr, void (*destructor)(void*), const uint32_t keylen, const VERB_rht_destroy_t destroy_list){
    for(uint32_t r = 0; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(!entry->key){                                                    // entry is free
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return;
        }
        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)){    // we already put this value inside the rht ~ we only update object
            if(entry->obj.ptr && entry->destroy_list & VERB_rht_destroy_obj) entry->obj.destructor(entry->obj.ptr);
            entry->obj.ptr = ptr;
            return;
        }
        if(r != keylen - 1){                                                // entry already has an element ~ we build a sub-rht and we put in that one 
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);
            VERB_rht_put_recursive(entry->rht, key, ptr, destructor, keylen, destroy_list, r+1);
            return;
        }
        else{                                                               // that can only be done when r != keylen-1; if that's the case, we do the inverse
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);               // we still build the subrht but we put the entry originally at this location in it & it with ours
            VERB_rht_put_recursive(entry->rht, entry->key, entry->obj.ptr, entry->obj.destructor, entry->keylen, destroy_list, r+1); 
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return;
        }
    }
}

// returns NULL on failure, obj otherwise
void* VERB_rht_search(VERB_rht_t* restrict rht, void* const restrict key, const uint32_t keylen){
    for(uint32_t r = 0; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)) return entry->obj.ptr;
        else if(!entry->rht) return NULL;

        rht = entry->rht;
    }

    return NULL;
}

// returns NULL on failure, obj otherwise
bool VERB_rht_search_bool(VERB_rht_t* restrict rht, void* const restrict key, const uint32_t keylen){
    for(uint32_t r = 0; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)) return true;
        else if(!entry->rht) return false;

        rht = entry->rht;
    }

    return false;
}

// returns 1 if it can successfully delete the entry, 0 if it cannot
bool VERB_rht_del(VERB_rht_t* restrict rht, void* const restrict key, const uint32_t keylen){
    for(uint32_t r = 0; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)){
            if(entry->obj.ptr && entry->destroy_list & VERB_rht_destroy_obj) entry->obj.destructor(entry->obj.ptr);
            if(entry->key     && entry->destroy_list & VERB_rht_destroy_key) free(entry->key);
            free(entry->key);
            entry->key = NULL;

            return true;
        }
        else if(!entry->rht) return false;

        rht = entry->rht;
    }

    return false;
}

// searches for entry, returns false & puts it in rht if it cannot find it, returns true otherwise.
bool VERB_rht_search_and_put(VERB_rht_t* restrict rht, void* const restrict key, void* const restrict ptr, void (*destructor)(void*), const uint32_t keylen, const VERB_rht_destroy_t destroy_list){
// behaviour:
//      search in RHT
//      whenever we encounter something that would normally mean our entry does not exist, we do the appropriate thing put would do.
    VERB_rht_t* entry = NULL;
    for(uint32_t r = 0; r < keylen; r++){
        entry = VERB_rht_get_entry(rht, key, r);

        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)) return true;
        if(r == keylen - 1) break;
        if(!entry->rht){
            entry->rht = VERB_rht_init(NULL);
            VERB_rht_put_recursive(entry->rht, key, ptr, destructor, keylen, destroy_list, r+1);
            return false;
        }
        
        rht = entry->rht;
    }

    if(!entry->rht) entry->rht = VERB_rht_init(NULL);
    VERB_rht_put_recursive(entry->rht, entry->key, entry->obj.ptr, entry->obj.destructor, entry->keylen, destroy_list, keylen+1); 
    VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
    return false;
}

void* VERB_rht_search_and_del(VERB_rht_t* restrict rht, void* const restrict key, const uint32_t keylen){
    for(uint32_t r = 0; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)){
            void* old_ptr = entry->obj.ptr;
            entry->obj.ptr = NULL;
            if(entry->key && entry->destroy_list & VERB_rht_destroy_key) free(entry->key); 

            return old_ptr;
        }
        else if(!entry->rht) return NULL;

        rht = entry->rht;
    }
    return NULL;
}

#endif
