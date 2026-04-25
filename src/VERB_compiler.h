#ifndef VERB_included
#define VERB_included

#include "lexer/lexer.h"
#include "frontend/frontend.h"

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

// VERB is a functional, compiled, low-level language similar to C or C++.
// the idea behind verb was to allow people to write less verbose code, achieved through:
//      complex types, which are basically structs with methods (NOT CLASSES as there's no inheritance currently).
//      operator & (limited) function overloading.
//      more sensible type system.
//      more sensible variable declaration.
//  and more!
//
// other features:
//      C-style preprocessor macros & constants.
//          why? because I originally was going to write a language much closer to C, so I wrote a C-style preprocessor, but then I scrapped the whole thing. I didn't want to waste all that work.
//      C++-style constexpr functions & values, done through const qualifier.
//      Zig-style operators on which signed & unsigned integer overflow is invalid/undefined, and separate wrapping operators %+ and %-.
//      statements, expressions terminated with '.', which IMO is far cooler than ';', although it might be harder to spot when missing or not.

// CURRENT ARCHITECTURE:
// VERB file -> [tokeniser] -> unoptimised bytecode -> [frontend] -> optimised bytecode -> [backend] -> machine code
// tokeniser ~ performs preprocessing, translation to bytecode. does this all in one pass, because one-pass compilers are cool. one day, I might change this.
// frontend  ~ performs generic (which does not mean BAD/simple) optimisations on bytecode.
// backend   ~ performs implementation-specific optimisations, translates to machine code.
// everything from the backend below is implementation-specific.
// the frontend does some generic optimisations & the backend is supposed to perform platform-specific ones.
// the backend must also translate from the frontend's bytecode to the implementation's assembly language.
// 
// I plan on coding the tokeniser in C, then writing the frontend/backend in VERB and eventually reimplementing the tokeniser in VERB.
// I plan on keeping the C version to avoid Ken Thompson's compiler hack-style attacks & also because I love C <333.
//

#endif
