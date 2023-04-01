* ~~fix pre/post inc/dec~~
* ~~fix three address code assembly generation...~~
* ~~implement the IR RETURN command to return a temp reg, to abstract away calling conventions~~
* asm encoding using the new instruction structure
* make the first executable, damn it!
* convert AST to object-like ~~and implement print to file, file line numbers~~
* maybe convert assembler and allocator to real objects, instead of singletons



For when we nail a working v.1...

* Ability to work with many files (compile only or link)
* Bring file information (filename & line) all the way to assembly language
* C Pre-processor (include, define, ifdef, endif etc)
* Make a debugger? Where is the limit?

