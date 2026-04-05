#ifndef CIRCUITC_preprocessor_all_preprocessor_ops_included
#define CIRCUITC_preprocessor_all_preprocessor_ops_included

// include this to include all preprocessor operations

#include "conditions/if.h"                      // #if
#include "conditions/else.h"                    // #else
#include "conditions/ifdef.h"                   // #ifdef
#include "conditions/elseif.h"                  // #elseif, #elif
#include "conditions/ifndef.h"                  // #ifndef

#include "replacement/macro.h"                  // #macro
#include "replacement/undef.h"                  // #undef
#include "replacement/define.h"                 // #define

#include "end/end.h"                            // #end
#include "end/endif.h"                          // #endif
#include "end/endonce.h"                        // #endonce
#include "end/endmacro.h"                       // #endmacro
#include "end/endmodule.h"                      // #endmodule

#include "msges/errors.h"                       // #error
#include "msges/warning.h"                      // #warning

#include "once.h"                               // #once
#include "module.h"                             // #module
#include "include.h"                            // #include

#include "embed.h"                              // #embed (TODO: FINISH!

void VERB_preprocessor_op_ignore(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    return;
}

#endif
