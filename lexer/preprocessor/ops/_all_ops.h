#ifndef CIRCUITC_preprocessor_all_preprocessor_ops_included
#define CIRCUITC_preprocessor_all_preprocessor_ops_included

// include this to include all preprocessor operations

#include "include.h"        // #include and #include "..." as ..."
#include "undef.h"          // #undef
#include "ifdef.h"          // #ifdef
#include "ifndef.h"         // #ifndef
#include "endif.h"          // #endif
#include "else.h"           // #else
#include "define.h"         // #define
#include "once.h"           // #once
#include "endonce.h"        // #endonce
#include "error.h"          // #error
#include "warning.h"        // #warning
#include "module.h"         // #module
#include "endmodule.h"      // #endmodule
#include "embed.h"          // #embed (TODO: FINISH!)

VERB_bytecode_t VERB_preprocessor_op_ignore(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    return VERB_BC_special_IGNORE;
}

#endif
