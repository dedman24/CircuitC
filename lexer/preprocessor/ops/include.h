#ifndef VERB_preprocessor_ops_include_included
#define VERB_preprocessor_ops_include_included

#include "define.h"
#include "stdio.h"              // file ops
#include "_includes.h"          // includes

// returns file size
size_t VERB_fsize(FILE* const fd){
    fseek(fd, 0, SEEK_END);
    size_t fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return fsize;
}

// reads an entire file 'fd' to the memory buffer 'ptr'
void VERB_fread_all(void* restrict ptr, const size_t size, const size_t nmemb, FILE* restrict stream){
    size_t bytes_read_so_far = 0;
    fseek(stream, 0, SEEK_SET);
// if fread returns an error, this may loop forever. TODO: FIX!
    do{
        bytes_read_so_far += fread(ptr, size, nmemb, stream);
    } while(bytes_read_so_far != size*nmemb);
}

size_t VERB_preprocessor_op_include_followed_by_as(char* restrict* const restrict string, char* restrict* const restrict namespace, VERB_tokeniser_t* const restrict tokeniser){
// ensures there are whitespaces.
    {
        const size_t whitespace_len = VERB_REGEX_whitespace_length(*string);
        if(!whitespace_len) return 0;
        *string += whitespace_len; tokeniser->offset += whitespace_len; 
    }
// ensures following statement is the string 'as'.
    {
        const size_t statement_len = VERB_REGEX_statement_length(*string);
        if(statement_len != strlen("as") || memcmp(*string, "as", strlen("as"))) return 0;
        *string += statement_len; tokeniser->offset += statement_len;
    }
// ensures there is some whitespace after (CONSIDER MAKING THIS A FUNCTION?).
    {
        const size_t whitespace_len = VERB_REGEX_whitespace_length(*string);
        if(!whitespace_len) return 0;
        *string += whitespace_len; tokeniser->offset += whitespace_len; 
    }
// namespace is what follows string. if no namespace is specified, this is currently not treated as an error, but perhaps it should be.
    {
        const size_t namespace_len = VERB_REGEX_statement_length(*string);
        *namespace = *string;
        *string += namespace_len; tokeniser->offset += namespace_len;
        return namespace_len;
    }
}

bool VERB_preprocessor_op_include_check_if_followed_by_module(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// ensures there are whitespaces.
    {
        const size_t whitespace_len = VERB_REGEX_whitespace_length(*string);
        if(!whitespace_len) return false;
        *string += whitespace_len; tokeniser->offset += whitespace_len; 
    }
// ensures following statement is the string 'as'.
    {
        const size_t statement_len = VERB_REGEX_statement_length(*string);
        if((statement_len != strlen("module")  || memcmp(*string, "module",  strlen("module")) ) &&
           (statement_len != strlen("modules") || memcmp(*string, "modules", strlen("modules")))) 
                return false;
        *string += statement_len; tokeniser->offset += statement_len;
    }
// ensures there is some whitespace after (CONSIDER MAKING THIS A FUNCTION?).
    {
        const size_t whitespace_len = VERB_REGEX_whitespace_length(*string);
        if(!whitespace_len) return false;
        *string += whitespace_len; tokeniser->offset += whitespace_len; 
    }
// *string now points to module statement
    return true;
}

void VERB_preprocessor_op_include_process_modules(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// algorithm:
//      get length of whitespace
//      get length of name & put module name in BST
//      get length of whitespace
//      check if there's a comma; if not, then break out.
    do{
        VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
        {
            const size_t modulename_len = VERB_REGEX_whitespace_length(*string);
            VERB_tree_boolean_put(&tokeniser->preprocessor->modules, *string, modulename_len, true);
            *string += modulename_len; tokeniser->offset += modulename_len;
        }
        VERB_preprocessor_op__skip_whitespaces(tokeniser, string);
    } while(**string == ',');
    if(**string == '.'){ 
        *string += 1;
    // skips whitespace to reach final \n.
        VERB_preprocessor_op__skip_whitespaces(tokeniser, string); 
    }
}

void VERB_preprocessor_op_include_tokenise_recursively(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, char* restrict pathname_start, const size_t pathname_length){
    char* const restrict pathname_end = pathname_start + pathname_length;   // points to the final '"'   
// ugly hack that assumes many things
    *pathname_end = 0;                                          // has to be done or else the whole program is interpreted as the file name 
    FILE* code_file = fopen(pathname_start, "r");               // as fopen takes a C string (terminated with \x00). we replace char with \x00 to terminate string..    
    if(!code_file){                                             // & then we replace it back
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_include_no_file, tokeniser->line, tokeniser->offset, 1, pathname_start);
        *pathname_end = '"';
        return;
    }
    *pathname_end = '"';
// if it fails to read the file, currently does nothing.

// gets file size
    const size_t filesize = VERB_fsize(code_file);
    char* restrict code = malloc(filesize);
    VERB_fread_all(code, filesize, 1, code_file);
    if(fclose(code_file) == EOF){
        free(code);
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_include_no_file, tokeniser->line, tokeniser->offset, 1, pathname_start);
        return;
    }

    const VERB_token_t terminating_characters[] = {
        VERB_TOKEN_special_EOF
    };

// can only be done this way; importantly HAS to be done b4 processing modules.
    VERB_tokeniser_persistent_data_t persistent; 
    VERB_tokeniser_persistent_data_populate(&persistent, tokeniser);
// checks for modules
    const bool processModules = VERB_preprocessor_op_include_check_if_followed_by_module(string, tokeniser);
    if(processModules){
        while(**string && **string != '\n') VERB_preprocessor_op_include_process_modules(string, tokeniser);
    }
// lexes file, handles final eof.
    while(*code) VERB_lexer_line(&code, tokeniser, false);
    VERB_bytecode_op_eof(&code, tokeniser);
// cleanup
    if(processModules) VERB_tree_boolean_destroy(tokeniser->preprocessor->modules, VERB_tree_keep_key);
    free(code);
    VERB_tokeniser_persistent_data_depopulate(tokeniser, &persistent);

}

const VERB_type_tok_t VERB_preprocessor_op_include__namespace_type_tok[] = {VERB_TYPE_type};
const char* const restrict VERB_preprocessor_op_include__namespace_type_str = "type";


// #include "..." as ... module ...
// returns ptr to error struct if something goes wrong, NULL otherwise 
VERB_bytecode_t VERB_preprocessor_op_include(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#include", tokeniser)) return VERB_TOKEN_special_IGNORE;

    if(**string != '"'){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 0);
// tries to go to end of #incude statement, either by going to the double quote, or to the newline, or to EOF.
// error handling is all implementation-defined; as long as the compiler conforms to the standard, it doesn't matter what the compiler does with them.
        char* pathname_end = strchr(*string, '"');
        if(!pathname_end) pathname_end = strchr(*string, '\n');
        if(!pathname_end) pathname_end = *string +strlen(*string);
        tokeniser->offset += (size_t)(pathname_end - *string);
        *string = pathname_end;
        return VERB_TOKEN_special_IGNORE;
    }
// TODO: MAKE THIS INTO SINGLE FUNCTION?
    *string += 1; tokeniser->offset += 1;
    
    char* pathname_start = *string;
    const size_t pathname_length = strcspn(pathname_start, "\"");
// +2 so it skips over '"' and points to character right after it.
    *string += pathname_length + 1; tokeniser->offset += pathname_length + 1;
    if(!pathname_length){
// NO FILE NAMED...
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_include_no_file, tokeniser->line, tokeniser->offset, 1, "IN #include STATEMENT AT ALL");
        return VERB_TOKEN_special_IGNORE;
    }
// namespace under which to add op
    char* namespace;
    const size_t namespace_len = VERB_preprocessor_op_include_followed_by_as(string, &namespace, tokeniser);
// sets up namespace. 
    void* old = namespace_len? NULL: VERB_tokeniser_backend_add_recently_added_rht(&tokeniser->backend);

    VERB_preprocessor_op_include_tokenise_recursively(string, tokeniser, pathname_start, pathname_length);;
    
    if(namespace_len){
        VERB_rht_t* recently_added = VERB_tokeniser_backend_extract_recently_added_rht(&tokeniser->backend);

        VERB_variable_type_t* const restrict namespace_data = VERB_variable_type_direct_init(recently_added);
        VERB_type_t* const restrict namespace_type = VERB_type_init(        // 0xded ARRESTED for type-casting to void*!
            (void*)VERB_preprocessor_op_include__namespace_type_tok, 
            sizeof(VERB_preprocessor_op_include__namespace_type_tok), 
            (void*)VERB_preprocessor_op_include__namespace_type_str, 
            strlen(VERB_preprocessor_op_include__namespace_type_str)
        );

        VERB_variable_definition_init(&tokeniser->backend, namespace, namespace_len, namespace_type, namespace_data);
    } 
    VERB_tokeniser_backend_set_recently_added_rht(&tokeniser->backend, old);
    
    return VERB_BC_special_IGNORE;
}

#endif
