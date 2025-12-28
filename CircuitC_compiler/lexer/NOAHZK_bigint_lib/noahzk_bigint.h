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
//      all operations ending in _resize are NOT GUARANTEED TO BE constant-time.
//      all other operations do not guarantee correctness when the source operands are larger in width than the destination operand.
// CODING STYLE:
//      variables and pointers declared const when possible
//      pointers declared restrict when possible
//      const before restrict on pointers (not that it changes much, I just like it more this way)
//  foo(void* restrict const bar);  --> NO
//  foo(void* const restrict bar);  --> YES
//      code should be C99-compatible (the best C standard)

#endif
