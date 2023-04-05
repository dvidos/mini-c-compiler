# mini-c-compiler

This project is a learning journey, 
to see what it takes to write a compiler,
to learn how they work, and to try my hand at 
creating machine code.

As a side effect, if this will be successful,
I am hoping it will be able to run on my other project, 
[handcrafted-os](https://github.com/dvidos/handcrafted-os).


## Writing a compiler

Let's say we want to write a single language. What shall it support?

* Expressions
* Variables
* One loop expression, with continue and break
* If / Else
* Functions
* Classes
* Modules
* Patterns? (observer, strategy, co-routines etc)
* Memory management / Garbage Collection?
* Exceptions?

How can we get out of the Von Neuman architecture,
with variables for storage, 
control statements for jump and test instructions,
and assignments for fetching, doing math and storing?

How can we avoid the imperative language, but go to 
declarative mathematical language?

Fun fact: I ended up implementing something C-like,
at least as a starting point, just to get something out
that works. Then I might add things to it 
(e.g. like the parallel routines in Go language)


## Phases

1. parser. read the file(s) and parse all the items
2. ast. by going over the tokens, you build the abstract syntax tree, 
a data oriented representation of the code
3. generate. targeting a specific CPU, you emit either assembly
or machine code for it

We'll need some good data structures (dictionaries, list, trees) for this.

## Giants

Of course I'm relying on giants here, I'm here to discover and learn.

* https://compilers.iecc.com/crenshaw/tutorfinal.pdf
* https://www3.nd.edu/~dthain/compilerbook/compilerbook.pdf
* https://web.stanford.edu/class/cs143/
* https://godbolt.org/



## Examples

Output from reading the file phase:

```
Read 660 bytes from file sample.c
---------------------
/*
    This is a sample comment.
    It exceeds in many lines.
    Thank you.
*/

int simple_var;
int var_with_value;

// this is a comment
int add_numbers(int a, int b) {
    int c = 2 * 3 + 4; // 10
    int d = 2 + 3 * 4; // 14
    int e = (2 + 3) * 4;    // 5 * 4
    int f = 1 + 2 + 3 + 4;  // 10
    int f = 1 * 2 + 3 * 4;  // 2 + 12
    int f = 1 + 2 * 3 + 4;  // 1 + 6 + 4

    if (c > 3) {
        return 1;
    } else {
        return c;
    }

    int x = 3;
    while (x > 0) {
        // something
        if (x > 10)
            continue;
        if (x < 3)
            break;
    }
}

// this is another comment
void main() {
    // func1(5);
}
```

Output from the lexer phase, grabbing tokens from the file contents

```
Parsed 132 tokens
---------------------
int
identifier "simple_var"
;
int
identifier "var_with_value"
;
int
identifier "add_numbers"
(
int
identifier "a"
,
int
identifier "b"
)
{
int
identifier "c"
=
numeric_literal "2"
*
numeric_literal "3"
+
numeric_literal "4"
;

[... snipped for brevity ...]

void
identifier "main"
(
)
{
}
eof
```

Output from the parser(s) converting tokens into the Abstract Syntax Tree

```
--- AST ----------------
int simple_var
int var_with_value
function: int add_numbers(int a, int b)
{
    int c=ADD(MUL(2,3),4)
    int d=ADD(2,MUL(3,4))
    int e=MUL(ADD(2,3),4)
    int f=ADD(1,ADD(2,ADD(3,4)))
    int f=ADD(MUL(1,2),MUL(3,4))
    int f=ADD(1,ADD(MUL(2,3),4))
    if(GT(c,3))
        {
            return (1)
        }
    else
        {
            return (c)
        }
    int x=3
    while(GT(x,0))
        {
            if(GT(x,10))
                continue
            if(LT(x,3))
                break
        }
}
function: void main()
{
}
```

## What did I learn?

That compilers are indeed interesting beasts!

The first phase is loading and parsing the file. We get rid of newlines,
empty space (where it can be ignored, as opposed say to Python), etc.
We end up with a stream of tokens, e.g. symbols, names, numbers etc.

The second phase is to parse these tokens into an AST, or *Abstract Syntax Tree*.
That means that the logic of the code is represented as data. 

For example, an `if` statement is represented as a node with three children,
one for the condition expression, one for the body if the condition evaluates
as true, and a final one for the body if the condition evaluates as false.
The expression can be a simple or nested expression to great depths,
while the bodies are sequences of statements, exactly like the `if` we are looking at.

The main parser I used for parsing the tokens is a *Recursive Descending parser*, 
which identifies parts of the program and calls the appropriate piece of code
to consume the tokens and generate the AST nodes. For expressions, since precedence
comes into play (e.g. multiplications must be done before additions, even if they 
appear to the right of the additions), I used a *Shunting Yard* algorithm, which 
uses stacks to make the highest priority operations pop last.

I also learned that the final code generated (even the machine code),
still needs a big piece of supporting data, formats, addresses etc. Enter the ELF 
format. This format, way more than just containing the code, describes
to the OS what to load, how, where etc, so that the program can be executed.
Similarly, it tells the Linker how to link a piece of object code, what the 
unsolved symbols are etc.
Therefore, I need to have code that supports ELF creation. 

### Machine code blues...

Turns out there's a very big number of instructions and not everything
supports everything. It also sounds like some constructs of C map directly to some assembly/cpu capabilities. 

So, I will try to also approach this from bottom-up perspective. Perhaps
building an assembler and knowing what it can support, then we can build 
the C compiler that will convert C to assembly.

### Light at the end of the tunnel

Taking the opposite route, from bottom up, i decided to make the smallest executable 
that I could, so that I could build off of that. 

I needed a way to represent instructions in a "parsed assembly" format, and an 
encoder that targets i386 Intel specifically, to make this work. Lots of 
investigation and back-and-forth, with online assemblers-deassemblers, and with
creating a minimum assembly program and comparing the dissassembled code sections.

Also needed to discover that the ELF file must store the sections in a way
that the loader can map them to pages. Otherwise a mysterious segmentation fault occured.

After a month or so, my first executable worked!

```
dimitris@mits-linux-laptop:~/code/mini-c-compiler (main *)$ ./mcc --asm-test && ./out.elf
mini-c-compiler, v0.01
Wrote 8393 bytes to out.elf file
Hello world!
```

### Some more notes


A high level language, could have the following characteristics:

* Built-in advanced data structures, like hashtables, lists, dictionaries, etc.
* Ability to initialize those in code, for example: { "name": "John", "age": 30 }
* Built-in algorithms for business processes, more than just qsort, allow various graph, Dynamic Programming, NP-complete problems to be executed
* Ability to run threads, or async tasks or other way of working with very shallow entry (think go routines)
* Ability to set reactive functions very easily, think cloud functions (arrival of a message, or a network packet etc)
* Ability to make restarting or rebooting very cheap, meaning, bring memory and things up to where they were in a previous snapshot or something.

Something similar to Python maybe...


How about we start from the bottom up?

* Given a machine code, make an elf file that can be read using readelf and objdump dissassembly
* Given specific encoding capabilities, allow encoding of specific listings into machine code.
* Find a way to convert IR code into those specific listings
* Find a way to encode high level constructs (e.g. for loops), using IR code
* Find a way to parse high level code (e.g. C) into these constructs
* Profit!

So the levels would be, along with their respective characteristics:

* Machine Code
	* Elf files, obj and executables
	* Segments and memory sections
	* Relocations & symbol tables
	* Representation: obj_code, mem_buffer, relocation_entry, elf file
* Assembly
	* Using tags and keywords for data / text segments
	* Representation: asm_listing, asm_instruction
* Intermediate Representation
	* Using Three Adress Code instructions, (un)conditional jumps, function calls
	* Representation: ir_listing, ir_entry
* Abstract Syntax Tree
	* Expressions (e.g. assignments, numeric operations, evaluations, func calls)
	* Statements (e.g. for loops, if statements, function declarations)
	* Representation: ast_node, ast_tree, ast_statement, ast_expression
* Source code
	* Syntax, parser
	* Representation: text file, string lines in memory

Then we need the following processes:

* Linker - to resolve relocations and create the elf files
* Encoder(?) - to convert assembly language into machine code
* Assembler(?) - to convert Intermediate Representation into assembly
* Code Generator - to convert Abstract Syntax Tree into Intermediate Representation code (e.g. for LLVM)
* Parser - to parse source code into Abstract Syntax Tree
* Editor - to write source code :-D

https://www.cs.princeton.edu/courses/archive/spr03/cs320/notes/IR-trans1.pdf
https://www.cs.princeton.edu/courses/archive/spr03/cs320/notes/IR-trans2.pdf

To produce a "hello world" executable we'll also need a small runtime in assembly,
e.g. the functions `exit()`, `read()`, `write()`, and their interface with the Operating System.

Graphical representation:

```
          [Lexer]           [Parser]              [Code Gen]                 [Assembler]             [Encoder]            [Linker]
  Source ---------> Tokens -----------> Abstract -------------> Intermediate -------------> Assembly -----------> Object ----------> Executable
   Code             Stream            Syntax Tree               Represenation                                      Code       ^         File
                                                                                                                              |
                                                                                                     [nasm]                   | 
                                                                                      runtime.asm  ---------> runtime.obj ----+
                                                                                      (hand crafted)
```
