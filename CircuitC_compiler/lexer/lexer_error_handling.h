#ifndef CIRCUITC_lexer_error_handling_included
#define CIRCUITC_lexer_error_handling_included

#include "stdint.h"
#include "stdlib.h"

typedef struct{
    uint64_t line;
    uint64_t offset;
} CIRCUITC_lexer_error_specifics_t;

CIRCUITC_lexer_error_specifics_t* CIRCUITC_lexer_error_specifics_init(CIRCUITC_lexer_error_specifics_t* error_specifics, const uint64_t line, const uint64_t offset){
    if(!error_specifics) error_specifics = malloc(sizeof(*error_specifics));

    error_specifics->line   = line;
    error_specifics->offset = offset;

    return error_specifics;
}

void CIRCUITC_lexer_error_specifics_destroy(CIRCUITC_lexer_error_specifics_t* error_specifics){
    free(error_specifics);
}

#endif
