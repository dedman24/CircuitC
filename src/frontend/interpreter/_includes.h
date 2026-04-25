#ifndef VERB_frontend_interpreter__includes_included
#define VERB_frontend_interpreter__includes_included

#include "stddef.h"
#include "stdint.h"                                                     // integer types.
#include "interpreter_ctx.h"
#include "../../libraries/rhts/rht.h"                                   // VERB_rht_t & ops.
#include "../../tokens/primitives/bytecode.h"                           // bytecode tokens.
#include "../../lexer/tokeniser/variables/_all_variables.h"             // all variable types.

#define VERB_max(x, y) ((x) > (y)? (x): (y))
#define VERB_min(x, y) ((x) > (y)? (y): (x))

#endif
