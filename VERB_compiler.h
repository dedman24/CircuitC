#ifndef VERB_included
#define VERB_included

#include "lexer/lexer.h"


//  #      #        #
// # #    #  ##     #
//    #  #  ### ### ##
//    # #   #   #   ###
//     #     ## #   ##

//     ##            ##                
//   #####          ##               #
//  #    ##       ###                #
//       ##       ##                 #
//        ##     ##     ###          ###
//        ##    ##     ##  #   ####  #  ##
//        ##  ###      #####  #      #   #
//        ##  ##       #      #      #   #
//         ####        ##     #      #  ##
//          ##          ###   #      ####
//

// VERB is a compiled, low-level language similar to C or C++.
// the idea behind verb was to allow people to write less verbose code, achieved through:
//      complex types, which are basically structs with methods (NOT CLASSES as there's no inheritance currently).
//      operator & function overloading.
//          operator overloading is good, IDK why zig rejects it. -> because it's harder to debug? 
//          perhaps add a specific symbol to denote operator overloading, so control flow is still explicit? like @ or #; that would be ugly.
//          TODO: add a compiler option that removes all hidden control flow! 
//      more sensible type system.
//      more sensible variable declaration.
//      dynamic typing in a statically-typed language with the generic type 'gen_t' (AKA generics).
//      indentation to assume where scopes start & end, although scopes can be explicited.
//  and more!
//
// other features:
//      C-style preprocessor macros & constants.
//          why? because I originally was going to write a language much closer to C, so I wrote a C-style preprocessor, but then I scrapped the whole thing.
//      C++-style constexpr functions & values, done through const qualifier.
//      Rust-style immutability by default, with mutability specified through the mut qualifier.
//      Zig-style operators on which signed & unsigned integer overflow is invalid/undefined, and separate wrapping operators %+ and %-.
//      C-style memory unsafety :OO
//      Rust-style unportable{} instead of unsafe{} to reduce unportable code.
//      statements, expressions terminated with '.', which IMO is far cooler than ';', although it might be harder to spot when missing or not.

// CURRENT ARCHITECTURE:
// VERB file -> [tokeniser] -> unoptimised bytecode -> [frontend] -> optimised bytecode -> [backend] -> machine code
// tokeniser ~ performs preprocessing, translation to bytecode. does this all in one pass, because one-pass compilers are cool.
// frontend  ~ performs generic (which does not mean BAD/simple) optimisations on bytecode.
// backend   ~ performs implementation-specific optimisations, translates to machine code.
// everything from the backend below is implementation-specific.
// the frontend does some generic optimisations & the backend is supposed to perform platform-specific ones.
// the backend must also translate from the frontend's bytecode to the implementation's assembly language.

#endif
