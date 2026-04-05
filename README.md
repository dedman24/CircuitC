# VERB
VERB is a programming language I'm working on meant to fix all the issues C has, like all the other 100 C successor languages.\
I'm kidding I just want to code my own programming language & it is kind of reminiscent of C because I like C.\
A lot is missing; lexer architecture is almost finished, optimisation passes (generic & specific to an architecture) & codegen are still missing.\
May the Lord help me.

## COMPILER ARCHITECTURE
Preprocessing and code generation to bytecode happens in a single pass; this means the compiler does not generate any intermediate files in which it expands macros, or any AST, nor does it tokenise the source code (not anymore).\
I did this because I thought it would be cool. I also didn't need an AST or any intermediate result file (so far, might change since I'm still so early on in the development).
#### VERB BYTECODE
VERB uses its own bytecode, which is based on dataflow machines. The end result is SSA with different names for some instructions (phi -> pick).\
The dataflow machine is based on the work done by Arvind and David E. Culler in their publication "[Dataflow Architectures](https://dspace.mit.edu/handle/1721.1/149103)".
## LANGUAGE ITSELF
WIP.
