* ~~fix pre/post inc/dec~~
* ~~fix three address code assembly generation...~~
* ~~implement the IR RETURN command to return a temp reg, to abstract away calling conventions~~
* asm encoding using the new instruction structure
* make the first executable, damn it!
* convert AST to object-like ~~and implement print to file, file line numbers~~
* maybe convert assembler and allocator to real objects, instead of singletons


* besides the generated files, we can generate files that show side-to-side, how the conversions are made
    * from AST into IR
    * from IR into ASM
    * from ASM into machine code, example follows

```
MOV  [BP-8] <- AX    >> --  --  --  --  --  89  01.000.101 --.---.--- f8.--.--.-- --.--.--.-- >>  89 45 f8
CALL math_demo       >> --  --  --  --  --  ff  00.010.101 --.---.--- 00.00.00.00 --.--.--.-- >>  ff 15 00 00 00 00
```


For when we nail a working v.1...

* Ability to work with many files (compile only or link)
* Bring file information (filename & line) all the way to assembly language
* C Pre-processor (include, define, ifdef, endif etc)
* Add specific processing at the start and end of functions, e.g. to release memory etc.
* Make a bunch of tests?
* Make a debugger? Where is the limit?

