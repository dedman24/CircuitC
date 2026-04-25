#ifndef VERB_preprocessor_ops_include_included
#define VERB_preprocessor_ops_include_included

#include "stdio.h"                                          // file ops.
#include "_includes.h"                                      // includes.
#include "standardlib/standardlib.h"                        // standard library handling.
#include "../../tokeniser/variables/_all_variables.h"

// returns file size
static size_t VERB_fsize(FILE* const fd){
    fseek(fd, 0, SEEK_END);
    size_t fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return fsize;
}

// reads an entire file 'fd' to the memory buffer 'ptr'
static void VERB_fread_all(void* restrict ptr, const size_t size, const size_t nmemb, FILE* restrict stream){
    size_t bytes_read_so_far = 0;
    fseek(stream, 0, SEEK_SET);
// if fread returns an error, this may loop forever. TODO: FIX!
    do{
        bytes_read_so_far += fread(ptr, size, nmemb, stream);
    } while(bytes_read_so_far != size*nmemb);
}

static size_t VERB_preprocessor_op_include__followedByAs(char* restrict* const restrict string, char* restrict* const restrict namespace, VERB_tokeniser_t* const restrict tokeniser){
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

static bool VERB_preprocessor_op_include__followedByModule(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
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

static void VERB_preprocessor_op_include__modules(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
// algorithm:
//      get length of whitespace
//      get length of name & put module name in BST
//      get length of whitespace
//      check if there's a comma; if not, then break out.
    do{
        VERB_tokeniser_skip_whitespace(string, tokeniser);
        {
            const size_t modulename_len = VERB_REGEX_whitespace_length(*string);
            VERB_tree_boolean_put(&tokeniser->preprocessor->modules, *string, modulename_len, true);
            *string += modulename_len; tokeniser->offset += modulename_len;
        }
        VERB_tokeniser_skip_whitespace(string, tokeniser);
    } while(**string == ',');
    if(**string == '.'){ 
        VERB_tokeniser_skip_chars(string, tokeniser, 1);
        VERB_tokeniser_skip_whitespace(string, tokeniser);      // skips whitespace to reach final \n. 
    }
}

static void VERB_preprocessor_op_include__recursive(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser, char* pathname, const size_t pathlength){
    const bool isFileSTDLIB = VERB_preprocessor_op__isFileSTDLIB(pathname, pathlength);
    if(isFileSTDLIB)
        pathname = VERB_preprocessor_op__resolveSTDLIBPath(tokeniser, pathname, pathlength);
    else{
        VERB_tokeniser_path_push(pathname, pathlength, tokeniser->path);
        pathname = VERB_tokeniser_path_get(tokeniser->path);
        VERB_tokeniser_path_pop(pathlength, tokeniser->path);                               // not unsafe I know what I'm doing.
    }

    FILE* const restrict code_file = fopen(pathname, "r");    
    if(!code_file){
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_include_no_file, tokeniser->line, tokeniser->offset, 1, pathname);
        return;
    }
// gets file size
    const size_t filesize = VERB_fsize(code_file);
    char* restrict code = malloc(filesize);
    VERB_fread_all(code, filesize, 1, code_file);
    if(fclose(code_file) == EOF){
        free(code);
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_include_no_file, tokeniser->line, tokeniser->offset, 1, pathname);
        return;
    }

    const VERB_token_t terminating_characters[] = {
        VERB_TOKEN_special_EOF
    };

// can only be done this way; importantly HAS to be done b4 processing modules.
    VERB_tokeniser_persistent_data_t persistent; 
    VERB_tokeniser_persistent_data_populate(&persistent, tokeniser);
// checks for modules
    const bool processModules = VERB_preprocessor_op_include__followedByModule(string, tokeniser);
    if(processModules){
        while(**string && **string != '\n') VERB_preprocessor_op_include__modules(string, tokeniser);
    }
// lexes file, handles final eof.
    while(*code) VERB_lexer(&code, tokeniser, "");
    VERB_bytecode_op_eof(&code, tokeniser);
// cleanup
    if(processModules) VERB_tree_boolean_destroy(tokeniser->preprocessor->modules, VERB_tree_keep_key);
    free(code);
    VERB_tokeniser_persistent_data_depopulate(tokeniser, &persistent);

}

// #include "..." as ... module ...
// returns ptr to error struct if something goes wrong, NULL otherwise 
static void VERB_preprocessor_op_include(char* restrict* const restrict string, VERB_tokeniser_t* const restrict tokeniser){
    if(!VERB_preprocessor_op__check_for_whitespace(string, "#include", tokeniser)) return;

    if(**string != '"'){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 1, "NO '\"' (DOUBLE QUOTES SYMBOL) AFTER #include STATEMENT");
// tries to go to end of #incude statement, either by going to the double quote, or to the newline, or to EOF.
// error handling is all implementation-defined; as long as the compiler conforms to the standard, it doesn't matter what the compiler does with them.
        char* pathname_end = strchr(*string, '"');
        if(!pathname_end) pathname_end = strchr(*string, '\n');
        if(!pathname_end) pathname_end = *string +strlen(*string);
        tokeniser->offset += (size_t)(pathname_end - *string);
        *string = pathname_end;
        return;
    }
    VERB_tokeniser_skip_chars(string, tokeniser, 1);
    
    char* const pathname_start = *string;
    const size_t pathname_length = strcspn(pathname_start, "\"");
    VERB_tokeniser_skip_chars(string, tokeniser, pathname_length + 1);
    if(!pathname_length){                           // NO FILE NAMED...
        VERB_error_report(tokeniser->specifics, VERB_error_preprocessor_include_no_file, tokeniser->line, tokeniser->offset, 1, "IN #include STATEMENT AT ALL");
        return;
    }
// namespace under which to add op
    char* namespace;
    const size_t namespace_len = VERB_preprocessor_op_include__followedByAs(string, &namespace, tokeniser);
// sets up namespace. 
    void* old = namespace_len? NULL: VERB_tokeniser_backend_add_recently_added(&tokeniser->backend);

    VERB_preprocessor_op_include__recursive(string, tokeniser, pathname_start, pathname_length);
    
    if(namespace_len){
        VERB_array_t* recently_added = VERB_tokeniser_backend_extract_recently_added(&tokeniser->backend);
        VERB_array_extract_all(recently_added, const size_t typelen, VERB_type_tok_t* const restrict type, false);

        VERB_type_signature_build(type, typelen, sig, siglen);
        VERB_type_t* const restrict namespace_type = VERB_type_copy(type, typelen, sig, siglen);

        VERB_variable_definition_init(&tokeniser->backend, namespace, namespace_len, namespace_type, VERB_VARIABLE_FLAG_const);
    } 
    VERB_tokeniser_backend_set_recently_added(&tokeniser->backend, old);
}

#endif
