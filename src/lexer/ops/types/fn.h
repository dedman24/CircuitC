#ifndef VERB_bytecode_op_fn_included
#define VERB_bytecode_op_fn_included

#include "../_includes.h"
#include "../_primitives/primitives.h"

// format for fn:
//      fn function_name(typed parameters) ~ (typed return args) = expression function evaluates to.
//      fn function_name(typed parameters) ~ (typed return args){ expression function evaluates to }
// in both cases, functions can be nameless (anonymous functions).
// the function type is fn(typed parameters) ~ (typed return args), stored as:
// VERB_TYPE_fn <types of parameters> VERB_TYPE_special_RETURNARGS <types of return args>

static bool VERB_bytecode_op_fn_inline(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(**string != '~') return false;
    VERB_tokeniser_skip_chars(string, tokeniser, 1);        // skips over '~'.
    VERB_tokeniser_skip_whitespace(string, tokeniser);


    const size_t fieldlen = VERB_REGEX_statement_length(*string);
    if(fieldlen == strlen("inline") && memcmp(*string, "inline", fieldlen)){
        VERB_tokeniser_skip_chars(string, tokeniser, fieldlen);
        return true;
    } 

    const char t = (*string)[fieldlen];
    (*string)[fieldlen] = '\0';
    VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 2, "UNKNOWN FUNCTION TYPE fn~", *string);
    (*string)[fieldlen] = t;
    VERB_tokeniser_skip_chars(string, tokeniser, fieldlen);

    return false;
}

// evaluates expressions inside round brackets (src or dst arguments).
static VERB_variable_t** const VERB_bytecode_op_fn__evaluate_expressions_inside_brackets(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, size_t* const restrict argcnt, const bool isHaving0ArgsValid){
    VERB_tokeniser_skip_whitespace(string, tokeniser);
    if(**string != '('){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION NOT FOLLOWED BY ROUND BRACKETS");
        return NULL;
    }
    VERB_tokeniser_skip_chars(string, tokeniser, 1);
    VERB_tokeniser_skip_whitespace(string, tokeniser);
// evaluates src expressions.
    const size_t initial_length = VERB_array_extract_length(&tokeniser->values);
    while(**string && **string != ')'){
        VERB_op__opcode(tokeniser, VERB_BC_special_OPENED_ROUND_BRACKET);               // makes compiler process what's inside the argument list.
        VERB_lexer_bracket_depth(string, tokeniser, ',');                               // I WANT TO CODE VERB STUFF I WANT TO FINISH MY OTHER PROJECTS I HATE PROGRAMMING A COMPILER.
        VERB_op__opcode(tokeniser, VERB_BC_special_CLOSED_ROUND_BRACKET);               // I need to test this too? hell nahhhh ;-;.

        if(**string == ',') VERB_tokeniser_skip_chars(string, tokeniser, 1);
    }
    const size_t final_length = VERB_array_extract_length(&tokeniser->values);

#ifdef VERB_DEBUG
    if(final_length < initial_length){
        VERB_error_report(tokeniser->specifics, VERB_error_debug, tokeniser->line, tokeniser->offset, 1, "IN FUNCTION HANDLING: final_length < initial_length");
        return NULL;
    }
#endif
    *argcnt = (final_length-initial_length)/sizeof(VERB_variable_token_t);              // neat trick.
    if(!**string){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "DECLARATION OF FUNCTION AT EOF");
        VERB_array_pop_offset_type(*argcnt, VERB_variable_token_t, &tokeniser->values);
        return NULL;
    }
    VERB_tokeniser_skip_chars(string, tokeniser, 1);        // skips over last bracket.
// in theory it's valid to write functions without any return arguments, like in fn foo(x: u64 !acq), but I chose to omit such things to keep syntax clean.
    if(!*argcnt && !isHaving0ArgsValid){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION CANNOT HAVE 0 RETURN ARGUMENTS");
        return NULL;
    }

    VERB_variable_t** const arg = malloc(*argcnt*sizeof(*arg));
    for(size_t i = 0; i < *argcnt; i++){
        arg[i] = VERB_variable_search_byToken(&tokeniser->backend, VERB_array_pop_type(VERB_variable_token_t, &tokeniser->values));
#ifdef VERB_DEBUG
        if(!arg[i]){
            VERB_error_report(tokeniser->specifics, VERB_error_debug, tokeniser->line, tokeniser->offset, 1, "IN FUNCTION HANDLING: UNKNOWN ", isHaving0ArgsValid? "SOURCE": "DEST", " ARGUMENT");
            return NULL;
        }
#endif
    }

    return arg;
}

static void VERB_bytecode_op_fn(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// parses name
    VERB_tokeniser_skip_whitespace(string, tokeniser);
    const bool isInline = VERB_bytecode_op_fn_inline(string, tokeniser);
// TODO: add parsing of fn qualifiers (like const etc) here.
    size_t namelen = VERB_REGEX_statement_length(*string);
    char* const name = *string;
// checks for operator overloads.
    const bool isOperatorOverload = namelen == 0 && VERB_REGEX_operator_length(*string) && **string != '(';
    VERB_bytecode_t opOverloaded;
    if(isOperatorOverload){
        namelen = VERB_REGEX_operator_length(*string);                      // opname MUST be separated by function with a space.
        if(!namelen){
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "UNKNOWN OPERATOR TO OVERLOAD");
            return;
        }
// yucky O(n) approach that's technically bad but who cares? I doubt enough overloads are defined where THIS matters over all else. added to todo list anyways.
        for(VERB_bytecode_t i = 0; i < VERB_BC_special_LAST; i++){
            if(!memcmp(VERB_bytecode_str[i], name, VERB_min(strlen(VERB_bytecode_str[i]), namelen))){
                opOverloaded = i; 
                break;
            }
            else{
                const char t = name[namelen]; name[namelen] = '\0';
                VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 3, "NO SUCH OPERATOR AS ", name, " IN OPERATOR OVERLOAD");
                name[namelen] = t;
                return;
            }
        }
    }

    VERB_tokeniser_skip_chars(string, tokeniser, namelen);
    VERB_tokeniser_backend_scope_new(&tokeniser->backend);

    size_t srccnt;
    VERB_variable_t** const src = VERB_bytecode_op_fn__evaluate_expressions_inside_brackets(string, tokeniser, &srccnt, true);
    if(!src) return;

    VERB_tokeniser_skip_whitespace(string, tokeniser);
    if(**string != '~'){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "FUNCTION CANNOT HAVE 0 RETURN ARGUMENTS");
        return;
    }
    VERB_tokeniser_skip_chars(string, tokeniser, 1);
    size_t dstcnt;
    VERB_variable_t** const dst = VERB_bytecode_op_fn__evaluate_expressions_inside_brackets(string, tokeniser, &dstcnt, false);
    if(!dst) return;
// parses function format.
    VERB_tokeniser_skip_whitespace(string, tokeniser);
    if(!**string || (**string != '=' && **string != '{' && namelen)){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID FUNCTION DECLARATION");
        return;
    }
    const bool isEquals = **string == '=';
// processes function code.
    char* const code_start = VERB_array_top_abs(tokeniser->code);
    if(!namelen){                                                       // anonymous function.
        VERB_op__opcode(tokeniser, VERB_BC_special_OPENED_ROUND_BRACKET);
        VERB_lexer_bracket_depth(string, tokeniser, '\0');
        VERB_op__opcode(tokeniser, VERB_BC_ret);
        VERB_op__opcode(tokeniser, VERB_BC_special_CLOSED_ROUND_BRACKET);
    } 
    else if(isEquals){                                                  // function of the format fn foo(...) ~ (...) = expr.  
        VERB_lexer(string, tokeniser, ".");
        VERB_op__opcode(tokeniser, VERB_BC_ret);                        // returns arguments.
    } 
    else{                                                               // function of the format fn foo(...) ~ (...) { exprs }
        VERB_lexer_scope_depth(string, tokeniser, '\0');
        if(**string) VERB_tokeniser_skip_chars(string, tokeniser, 1);
        else{
            VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "INVALID FUNCTION DECLARATION TERMINATES WITH EOF");
            return;
        } 
    } 
    VERB_tokeniser_backend_scope_del(&tokeniser->backend);
    char* const code_end = VERB_array_top_abs(tokeniser->code);
    const size_t codesize = (size_t)(code_end - code_start);
// defines function type.
    const VERB_variable__flags_t flags = VERB_VARIABLE_FLAG_ALL;        // todo: define ts!

    VERB_type_t* const restrict funtype = VERB_variable_function_type_init(src, srccnt, dst, dstcnt);
// VERB_variable_t* const restrict overload = VERB_tree_ptr_search(tokeniser->overloaded_operators, query, sumoflengths);
    if(isOperatorOverload){
        VERB_variable_t* const restrict fun = VERB_variable_definition_internal_init(&tokeniser->backend, funtype, flags);
        const size_t querylen = VERB_op__operator_overload_query_len(tokeniser, opOverloaded, srccnt, src);
        if(!querylen) return;
        char* const restrict query = VERB_op__operator_overload_query(tokeniser, opOverloaded, srccnt, src, querylen);
        VERB_tree_ptr_put(&tokeniser->overloaded_operators, query, querylen, fun);              // adds to tree of function overloads.
    }
    else 
        VERB_variable_definition_init(&tokeniser->backend, name, namelen, funtype, flags);
    
}

#endif
