# CircuitC
CircuitC is an hardware description language based on C; the circuits specified by said code are then compiled to instances of the NP-complete satisfiability problem.  
CircuitC code can be thought of as a lot of preprocessor macros that aid in the specification of actual circuits.   
CircuitC code WILL BE (TODO: CHANGE) expressive enough to describe a simple RV64IM CPU core.

### Why C?
Because C is well-known, widely-used, well-defined (although CircuitC does stray from the C standard when convenient), it has been used as a hardware description language before (see: [Handel-C](https://en.wikipedia.org/wiki/Handel-C), which CircuitC is NOT based on).  
I also generally like C.

## WARNING
As of today, CircuitC is still incomplete.

## ROADMAP
 - lexer ~ complete
 - interpreter
     - error handling & everything the interpreter needs to do except interpreting 
     - basic operations
     - wires
     - structs & complex data types
     - enums & everything regarding pointers
