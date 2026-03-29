/*
   NOAHZK_bigint reference source code package - reference C implementations

   Copyright 2025, dedmanwalking <dedmanwalking@proton.me>.  You may use this under the
   terms of the CC0 1.0 Universal license, linked below:
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
*/

#ifndef NOAHZK_bigint_included
#define NOAHZK_bigint_included

#include "ops/definitions.h"
#include "ops/logarithms.h"
#include "ops/logic.h"
#include "ops/type.h"
#include "ops/add.h"
#include "ops/mul.h"
#include "ops/sub.h"

// NAMING SCHEME:
//      NOAHZK_variable_width_<op>
//      for arithmetic ops, the pointers to the source arguments are named:
// 1  arg  ~ src
// 2+ args ~ rs0, ..., rsn
//      multiple dsts are named as:
// 1  dst  ~ dst
// 2+ dsts ~ dst0, ..., dstn
//      all operations with the suffix _and_resize are NOT GUARANTEED TO BE constant-time.
//      all operations with the suffix _constant are NOT GUARANTEED TO BE constant-time if the value provided itself is not 'constant'.
//      all other operations do not guarantee correctness when the source operands are larger than the destination operand.
// CODING STYLE:
//      variables and pointers declared const when possible. one should want the source arguments to be 'const <type>* const' whenever possible, & dst to be '<type>* const'.
//      pointers declared restrict when possible.
//      const before restrict on pointers (not that it changes much, I just like it more this way).
//  foo(void* restrict const bar);  --> NO
//  foo(void* const restrict bar);  --> YES
//      code should follow the C99 standard (the best C standard).
//      ideally one should have a few primitive arithmetic functions, and all other functions that need to implement that operation relying on said primitives.

// why code my own bigint library?
//  - to learn how bigint libraries work.
//  - to have better control over the code.
//  - to be able to add my own functions, whose behaviour I understand, according to the code's necessities.
//  - to avoid relying on other libraries; I want to own all the code I write.
// however NOAHZK_bigint has shown to be much more prone to errors than I expected. I cannot put any of the blame on C; rather it has always shown to be my fault.
// NOAHZK_bigint heavily relies on bit manipulation. I hope it's portable.

#endif
