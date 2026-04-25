# VERB
VERB is a programming language I'm working on meant to fix all the issues C has, like all the other 100 C successor languages. It's also functional :OOO.\
I'm kidding I just want to code my own programming language.\
I like C, especially the low-level control one has. The lack of abstractions makes doing anything in C really rewardinglmfao.\
A lot is missing; the lexer architecture is almost finished, optimisation passes (generic & specific to an architecture) & codegen are still missing.\
I plan on writing those in VERB, with integration between the lexer (in C) and everything else (in VERB) through a FFI.\
May the Lord help me.

## COMPILER ARCHITECTURE
Preprocessing and code generation to bytecode happens in a single pass; this means the compiler does not generate any intermediate files in which it expands macros, or any AST, nor does it tokenise the source code.\
I did this because I thought it would be cool. I also didn't need an AST or any intermediate result file (so far, might change since I'm still so early on in the development).\
The lexer is an operator-precedence parser; this works in functional VERB (so far) because everything is an operator save for function declarations (handled as a special-case as syntax would be ugly otherwise).
#### VERB BYTECODE
VERB uses its own bytecode, which is based on dataflow machines. The end result is basically SSA with different names for some instructions.\
The dataflow machine is based on the work done by Arvind and David E. Culler in their publication "[Dataflow Architectures](https://dspace.mit.edu/handle/1721.1/149103)".
## LANGUAGE ITSELF
WIP.
