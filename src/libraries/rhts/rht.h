#ifndef VERB_library_rht_included
#define VERB_library_rht_included

#include "stdint.h"             // variable-width integer types
#include "string.h"             // memset, memcpy, memcmp & so on
#include "stdlib.h"             // dynamic memory operations
#include "stdbool.h"            // boolean type

// MUST HAVE THESE VALUES! THEY'RE USED FOR BIT FIELDS! DO NOT ALTER THESE!
typedef enum: uint8_t { VERB_rht_destroy_none = 0, VERB_rht_destroy_key = 1, VERB_rht_destroy_obj = 2 } VERB_rht_destroy_t;

typedef struct VERB_rht{
    void* key;
    struct{                     // defines object & destructor
        void (*destructor)(void*);
        void* ptr;
    } obj;
    struct VERB_rht* rht;
    uint32_t keylen;
    VERB_rht_destroy_t destroy_list;
} VERB_rht_t;

// size of a table.
#define VERB_RHT_SIZE 16
// must be able to hold VERB_RHT_SIZE distinct values.
typedef uint8_t VERB_rht_index_t;

typedef enum{ VERB_rht_keep, VERB_rht_free } VERB_rht_options_t;

// the Recursive Hash Table is a data structure I just made up that's really great at storing random keys inside a table. sadly that's not what we're doing; we're storing names.
// despite not being optimal, it'll do.
// assuming a balanced rht, the time complexities are the following:
// put ~ O(log(n))
// del ~ O(log(n))
// search ~ O(log(n))
// each RHT entry is composed of:
//      a key
//      an object
//      the rht pointed by said key
//      the length of the key
//      the elements of said rht that can be freed (not necessary in most implementations)
// in that order.
// a 'table' is composed of VERB_RHT_SIZE entries.

VERB_rht_index_t VERB_rht_get_index(const uint8_t* const key, const uint32_t r){
// gets either the top or bottom nibble of key at index r/2 based on the value of the last bit in r.
    return (key[r>>1] >> ((r&1) << 2) & 0x0F);
}

VERB_rht_t* VERB_rht_get_entry(VERB_rht_t* restrict rht, const uint8_t* const restrict key, const uint32_t r){
    return &rht[VERB_rht_get_index(key, r)];
}

void* VERB_rht_init(VERB_rht_t* rht){
    if(!rht) rht = calloc(VERB_RHT_SIZE, sizeof(*rht));
    else memset(rht, 0, sizeof(*rht)*VERB_RHT_SIZE);

    return rht;
}

void VERB_rht_destroy(VERB_rht_t* rht, const VERB_rht_options_t freeptr){
    for(uint64_t i = 0; i < VERB_RHT_SIZE; i++){
        if(rht[i].rht) VERB_rht_destroy(rht[i].rht, VERB_rht_free);
        if(rht[i].obj.ptr && rht[i].destroy_list & VERB_rht_destroy_obj) rht[i].obj.destructor(rht[i].obj.ptr);
        if(rht[i].key     && rht[i].destroy_list & VERB_rht_destroy_key) free(rht[i].key);
    }

    if(freeptr == VERB_rht_free) free(rht);
    else memset(rht, 0, sizeof(*rht)*VERB_RHT_SIZE);
}

void VERB_rht_put_entry(VERB_rht_t* const restrict entry, void* const restrict key, void* const restrict ptr, void (*destructor)(void*), const uint32_t keylen, const VERB_rht_destroy_t destroy_list){
    entry->key = key;
    entry->obj.ptr = ptr;
    entry->obj.destructor = destructor;
    entry->keylen = keylen;
    entry->destroy_list = destroy_list;
}

// only difference with put is that put_recursive accepts a parameter to init r.
// returns true if it put the element, false otherwise.
bool VERB_rht_put_recursive(VERB_rht_t* restrict rht, void* const restrict key, void* const restrict ptr, void (*destructor)(void*), const uint32_t keylen, const VERB_rht_destroy_t destroy_list, const uint32_t rinit){
    for(uint32_t r = rinit; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(!entry->key){                                                    // entry is free
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return true;
        }
    // entry already exists, we put nothing & return false.
        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen)) 
            return false;
        if(r != keylen - 1){                                                // entry already has an element ~ we build a sub-rht and we put in that one 
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);
        /* always succeeds */
            VERB_rht_put_recursive(entry->rht, key, ptr, destructor, keylen, destroy_list, r+1);
            return true;
        }
        else{                                                               // that can only be done when r != keylen-1; if that's the case, we do the inverse
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);               // we still build the subrht but we put the entry originally at this location in it & it with ours
        /* always succeeds */
            VERB_rht_put_recursive(entry->rht, entry->key, entry->obj.ptr, entry->obj.destructor, entry->keylen, destroy_list, r+1); 
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return true;
        }
    }
    return false;
}

bool VERB_rht_put(VERB_rht_t* restrict rht, void* const restrict key, void* const restrict ptr, void (*destructor)(void*), const uint32_t keylen, const VERB_rht_destroy_t destroy_list){
    for(uint32_t r = 0; r < keylen; r++){
        VERB_rht_t* const entry = VERB_rht_get_entry(rht, key, r);

        if(!entry->key){                                                    // entry is free
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return true;
        }
    // entry already exists, we put nothing & return false.
        if(entry->keylen == keylen && !memcmp(entry->key, key, keylen))
            return false;
        if(r != keylen - 1){                                                // entry already has an element ~ we build a sub-rht and we put in that one 
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);
        /* always succeeds */
            VERB_rht_put_recursive(entry->rht, key, ptr, destructor, keylen, destroy_list, r+1);
            return true;
        }
        else{                                                               // that can only be done when r != keylen-1; if that's the case, we do the inverse
            if(!entry->rht) entry->rht = VERB_rht_init(NULL);               // we still build the subrht but we put the entry originally at this location in it & it with ours        
        /* always succeeds */
            VERB_rht_put_recursive(entry->rht, entry->key, entry->obj.ptr, entry->obj.destructor, entry->keylen, destroy_list, r+1); 
            VERB_rht_put_entry(entry, key, ptr, destructor, keylen, destroy_list);
            return true;
        }
    }
    return false;
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
