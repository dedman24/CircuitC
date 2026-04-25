#ifndef VERB_preprocessor_ops__standardlib_included
#define VERB_preprocessor_ops__standardlib_included

// this file does NOT have all of VERB's standard libraries.
// each implementation is free to include a standard library implementation of their liking.
// the user should also be able to switch between different std library implementations with a compiler command pointing it to the correct folder to open.
// this file is meant to aide #include in std library stuff.

#include "string.h"                 // memcmp.
#include "../_includes.h"           // 

#define VERB_STD_LIBRARY_PREFIX "std/"

static bool VERB_preprocessor_op__isFileSTDLIB(char* const restrict pathname, const size_t pathlen){
    if(pathlen > strlen(VERB_STD_LIBRARY_PREFIX) && !memcmp(pathname, VERB_STD_LIBRARY_PREFIX, strlen(VERB_STD_LIBRARY_PREFIX))) return true;
    return false;
}

static char* VERB_preprocessor_op__resolveSTDLIBPath(VERB_tokeniser_t* const restrict tokeniser, char* const restrict oldpath, const size_t pathlen){
    char* const restrict newpath = VERB_tree_ptr_search(tokeniser->stdlibrary_file_paths, oldpath, pathlen);
    if(!newpath){
        VERB_error_report(tokeniser->specifics, VERB_error_invalid_statement, tokeniser->line, tokeniser->offset, 3, "NO SUCH STANDARD LIBRARY AS \"", oldpath, "\"");
        return NULL;
    }

    return newpath;
}

#endif
