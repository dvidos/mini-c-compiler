* ~~fix pre/post inc/dec~~
* ~~fix three address code assembly generation...~~
* ~~implement the IR RETURN command to return a temp reg, to abstract away calling conventions~~
* ~~asm encoding using the new instruction structure~~
* ~~make the first executable, damn it! <------ Done!~~
* ~~make x86_64 asm-to-machine-code encoder~~ Did it!
  * ~~start offering the smallest operations for just a hello world example.~~
* ~~rename the assembler into something else (backend IR converter?) and rename the Encoder into Assembler.~~
* ~~convert AST to object-like and implement print to file, file line numbers~~
~~* convert assembler and allocator to real objects, instead of singletons~~
~~* convert all objects into objects, with priv_data, with "v" for operations vtable, because we have so many different patterns~~

* ~~verify operations on R8 - R15 registers~~
* implement the x86_64 assembly of "hello world" listing, for end-to-end test

* simplify list operations even more
  * e.g. around ASM listing
  * list_contains() to use internal struct_info, no extra funcs to write
  * list_iterator_xxxx() to accept a pointer for local state storage.
  * implement `list_to_string(mp, separator)` for debugging


* Besides the generated files, we can generate files that show side-to-side, how the conversions are made
  * from AST into IR
  * from IR into ASM
  * from ASM into machine code, example follows

```
MOV  [BP-8] <- AX    >> --  --  --  --  --  89  01.000.101 --.---.--- f8.--.--.-- --.--.--.-- >>  89 45 f8
CALL math_demo       >> --  --  --  --  --  ff  00.010.101 --.---.--- 00.00.00.00 --.--.--.-- >>  ff 15 00 00 00 00
```

* Good refactoring for later: 
  * Identify the various stages
    * What they do
    * What is the processor and what runtime structures it uses
    * What are the input and output formats
  * Identify the data between each stage
    * Allow them to be saved and loaded from/to files

* Better error handling, logging etc
* Better testing... we do need this
 



For when we nail a working v.1...

* Ability to work with many files (compile only or link)
* Bring file information (filename & line) all the way to assembly language
* C Pre-processor (include, define, ifdef, endif etc)
* Add specific processing at the start and end of functions, e.g. to release memory etc.
* Make a bunch of tests?
* Make a debugger? Where is the limit?


Also:

* create solution similar to https://github.com/rxi/log.c
* i.e. 6 levels of logging,
* output stdout, stderr and optionally file
* can use a --gen-log to create an mcc.log file or something.


Objects to make:

* token
* ast_node
* declaration
* expression
* operator
* statement
* symbol
* ir_entry (expression, function, module, statement)
* ir_value
* asm_allocator
* asm_instruction
* asm_listing
* obj_code_module
* elf_contents

Another approach:

* Framework objects: mempool, str, binary, list, tree, hashtable, iterator, observer etc
* Steps to take: 
  * preprocess the file (preprocessor) -- string and file manipulation: includes, ifdefs, pattern substitution.
  * parse code into tokens (lexer) -- split into tokens with file/line/column information
  * parse tokens into AST (parser) -- recursive descend? something expandable... yacc, flex, bison?
  * generate IR from AST (generator?) -- maybe the easiest thing...
  * generate asm from IR (??) -- requires register allocator, function call ABI 
  * encode asm into machine code, generate obj file (assembler) -- encoding intel instructions
  * link obj files into executable (linker) -- resolve symbols and relocations
* So, how to proceed?

Resources

* https://tomassetti.me/resources-create-programming-languages/
