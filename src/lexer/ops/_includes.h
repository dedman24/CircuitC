#ifndef VERB_bytecode_op__includes_included
#define VERB_bytecode_op__includes_included
// i don't want the include guards to conflict with the preprocessor include commands

#include "types/const.h"

#include "../../tokens/token_type.h"
#include "../../tokens/token_processing.h"

#include "../tokeniser/backend/backend.h"
#include "../tokeniser/backend/compatibility.h"
#include "../tokeniser/backend/tokeniser_ctx.h"

#include "../tokeniser/variables/_all_variables.h"

// passing a function ptr would be better probably.
static void VERB_lexer(char* restrict*, VERB_tokeniser_t*, const char*);
static void VERB_lexer_bracket_depth(char* restrict*, VERB_tokeniser_t*, const char);
static void VERB_lexer_scope_depth(char* restrict*, VERB_tokeniser_t*, const char);

#endif
