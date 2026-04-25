#ifndef VERB_frontend_interpreter_arith_included
#define VERB_frontend_interpreter_arith_included

// basic arithmetic ops. the following are implemented here:
// +
// -
// /
// *

#include "../_includes.h"

#define VERB_interpreter_op_initialise_2src_1dst(stok0, stok1, dtok, sname0, sname1, dname, rht)                                                    \
    VERB_variable_t* const restrict sname0 = VERB_variable_search_byToken_rht(rht, stok0);                                                          \
    VERB_variable_t* const restrict sname1 = VERB_variable_search_byToken_rht(rht, stok1);                                                          \
    VERB_variable_t* const restrict dname  = VERB_variable_search_byToken_rht(rht, dtok)

#define VERB_interpreter_op_initialise_2src_1dst_arith(stok0, stok1, dtok, sname0, sname1, dname, rht)                                              \
    VERB_variable_arith_t* const restrict sname0 = ((VERB_variable_t*)VERB_variable_search_byToken_rht(rht, stok0))->custom_data;                   \
    VERB_variable_arith_t* const restrict sname1 = ((VERB_variable_t*)VERB_variable_search_byToken_rht(rht, stok1))->custom_data;                   \
    VERB_variable_arith_t* dname;                                                                                                                   \
    do{                                                                                                                                             \
        VERB_variable_t* const restrict dvar = ((VERB_variable_t*)VERB_variable_search_byToken_rht(rht, dtok)) ->custom_data;                       \
        if(!dvar->custom_data) dvar->custom_data = VERB_variable_arith_init(sname0->type, VERB_max(sname0->precision, sname1->precision));          \
        dname = dvar->custom_data;                                                                                                                  \
    } while(0)
    
#define VERB_interpreter_op_initialise_2src_1dst_arith_variable_def_given(sdef0, sdef1, ddef, sname0, sname1, dname)                                \
    VERB_variable_arith_t* const restrict sname0 = ((VERB_variable_t*)sdef0)->custom_data;                                                          \
    VERB_variable_arith_t* const restrict sname1 = ((VERB_variable_t*)sdef1)->custom_data;                                                          \
    VERB_variable_arith_t* dname;                                                                                                                   \
    do{                                                                                                                                             \
        if(!((VERB_variable_t*)ddef)->custom_data)                                                                                                  \
            ((VERB_variable_t*)ddef)->custom_data                                                                                                   \
                = VERB_variable_arith_init(sname0->type, VERB_variable_arith_precision_max(sname0, sname1));                                        \
        dname = ((VERB_variable_t*)ddef)->custom_data;                                                                                              \
    } while(0)

#define VERB_interpreter_op__generic(d, s0, s1, sintOp, uintOp, boolOp, f32Op, f64Op)       \
    do{                                                                                     \
        switch(s0->type){                                                                   \
        case VERB_VARIABLE_ARITH_SINT:                                                      \
            sintOp(&d->i, &s0->i, &s1->i);                                                  \
            break;                                                                          \
        case VERB_VARIABLE_ARITH_UINT:                                                      \
            uintOp(&d->i, &s0->i, &s1->i);                                                  \
            break;                                                                          \
        case VERB_VARIABLE_ARITH_BOOL:                                                      \
            d->b = s0->b boolOp s1->b;                                                      \
            break;                                                                          \
        case VERB_VARIABLE_ARITH_FLOAT32:                                                   \
            d->f32 = s0->f32 f32Op s1->f32;                                                 \
            break;                                                                          \
        case VERB_VARIABLE_ARITH_FLOAT64:                                                   \
            d->f64 = s0->f64 f64Op s1->f64;                                                 \
            break;                                                                          \
        }                                                                                   \
    } while(0)


static void VERB_interpreter_op_mul_impl(VERB_variable_t** const restrict sdef, VERB_variable_t** const restrict ddef){
    VERB_interpreter_op_initialise_2src_1dst_arith_variable_def_given(sdef[0], sdef[1], ddef[0], s0, s1, d);
    VERB_interpreter_op__generic(d, s0, s1, NOAHZK_variable_width_mul, NOAHZK_variable_width_mul, &, *, *);
}

/// /// subtraction /// ///

static void VERB_interpreter_op_add_impl(VERB_variable_t** const restrict sdef, VERB_variable_t** const restrict ddef){
    VERB_interpreter_op_initialise_2src_1dst_arith_variable_def_given(sdef[0], sdef[1], ddef[0], s0, s1, d);
    VERB_interpreter_op__generic(d, s0, s1, NOAHZK_variable_width_add, NOAHZK_variable_width_add, |, +, +);
}

static void VERB_interpreter_op_add(VERB_code_t* const restrict code, VERB_rht_t* const restrict token_defs){

}

/// /// subtraction /// ///

static void VERB_interpreter_op_sub_impl(VERB_variable_t** const restrict sdef, VERB_variable_t** const restrict ddef){
    VERB_interpreter_op_initialise_2src_1dst_arith_variable_def_given(sdef[0], sdef[1], ddef[0], s0, s1, d);
    VERB_interpreter_op__generic(d, s0, s1, NOAHZK_variable_width_sub, NOAHZK_variable_width_sub, ^, -, -);
}

static void VERB_interpreter_op_sub(VERB_code_t* const restrict code, VERB_rht_t* const restrict token_defs){

}



#endif
