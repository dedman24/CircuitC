#ifndef VERB_lexer_preprocessor_included
#define VERB_lexer_preprocessor_included

#include "ops/_all_ops.h"                           // all operations.
#include "ops/_includes.h"                          // all useful includes.
#include "preprocessor_ctx.h"                       // preprocessor context.

// the VERB preprocessor is very heavily inspired by the C preprocessor, as is the whole language.
// one may modify the directives supported by the preprocessor, add new ones and so on. I quite like how the preprocessor overall came out in the end.

// ALL PREPROCESSOR COMMANDS!
// written in the order I wrote them, perhaps change to make searching for them more efficient.
struct{
    char* id;                                                                                           // char string under which preprocessor macro is recognised.
    void (*procedure)(char* restrict* const restrict, VERB_tokeniser_t* const restrict);     // ptr to procedure that implements the preprocessor macro.
} VERB_preprocessor_macros[] = {
    { "include",   VERB_preprocessor_op_include   },
    { "undef",     VERB_preprocessor_op_undef     },
    { "ifdef",     VERB_preprocessor_op_ifdef     },
    { "endif",     VERB_preprocessor_op_endif     },
    { "else",      VERB_preprocessor_op_else      },
    { "ifndef",    VERB_preprocessor_op_ifndef    },
    { "define",    VERB_preprocessor_op_define    },
    { "once",      VERB_preprocessor_op_once      },
    { "endonce",   VERB_preprocessor_op_endonce   },
    { "error",     VERB_preprocessor_op_error     },
    { "warning",   VERB_preprocessor_op_warning   },
    { "module",    VERB_preprocessor_op_module    },
    { "endmodule", VERB_preprocessor_op_endmodule },
    { "#",         VERB_preprocessor_op_ignore    },            // ignored by preprocessor, removed when building #define macros.
    { "ignore",    VERB_preprocessor_op_ignore    },            // ignored by preprocessor, used to build #define macros. useful for inverse stringificaion.
    { "macro",     VERB_preprocessor_op_macro     },
    { "endmacro",  VERB_preprocessor_op_endmacro  },
    { "if",        VERB_preprocessor_op_if        },
    { "elseif",    VERB_preprocessor_op_elseif    },
    { "elsif",     VERB_preprocessor_op_elsif     },
    { "elif",      VERB_preprocessor_op_elif      },
    { "end",       VERB_preprocessor_op_end       },
};

VERB_tree_bytecode_t* VERB_token_preprocessor_commands_init(){
    VERB_tree_bytecode_t* directives;

    for(size_t i = 0; i < sizeof(VERB_preprocessor_macros)/sizeof(*VERB_preprocessor_macros); i++)
        VERB_tree_bytecode_put(&directives, VERB_preprocessor_macros[i].id, strlen(VERB_preprocessor_macros[i].id), VERB_preprocessor_macros[i].procedure);

    return directives;
}

VERB_preprocessor_t* VERB_preprocessor_init(VERB_preprocessor_t* preprocessor){
    if(!preprocessor) preprocessor = malloc(sizeof(*preprocessor));

    preprocessor->directives = VERB_token_preprocessor_commands_init();
    preprocessor->defined_things = VERB_rht_init(NULL);
    preprocessor->once_included = VERB_rht_init(NULL);

    return preprocessor;
}

void VERB_preprocessor_destroy(VERB_preprocessor_t* const preprocessor){
    VERB_tree_bytecode_destroy(preprocessor->directives, VERB_tree_keep_key);
    VERB_tree_boolean_destroy(preprocessor->modules, VERB_tree_keep_key);
    VERB_rht_destroy(preprocessor->defined_things, VERB_rht_free);
    VERB_rht_destroy(preprocessor->once_included, VERB_rht_free);

    free(preprocessor);
}

// returns appropriate token.
// preprocessor directives have to be handled before other tokens.
void VERB_preprocessor_handle(char* restrict* const restrict string, const size_t length, VERB_tokeniser_t* tokeniser){
    VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
    VERB_token_fun_t procedure = VERB_tree_bytecode_search(tokeniser->preprocessor->directives, *string, length);
// if procedure == NULL, string is not a preprocessor directive
// directives analysed first so someone cannot write something like
// #define #ifdef #ifndef
// #define #ifndef #ifdef
// & have that work.
    if(procedure){
        *string += length;                      // skips over directive name
        procedure(string, tokeniser);
        return;
    }

// if definition == NULL, string is not an #define-d symbol.
    VERB_preprocessor_macro_element_t* const restrict definition = VERB_rht_search(tokeniser->preprocessor->defined_things, *string, length);
    if(definition){
        *string += length;                      // skips over directive name
        VERB_preprocessor_put_macro(definition, string, tokeniser);
        return;
    }

    VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "unknown preprocessor command.");
    return;
}

#endif
