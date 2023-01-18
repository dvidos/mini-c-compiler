# simple lang

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

## phases

1. parser. read the file(s) and parse all the items
2. ast. by going over the tokens, you build the abstract syntax tree, 
a data oriented representation of the code
3. generate. targeting a specific CPU, you emit either assembly
or machine code for it

We'll need some good data structures (dictionaries, list, trees) for this.

## giants

Of course I'm relying on giants here, I'm here to discover and learn.

See https://compilers.iecc.com/crenshaw/tutorfinal.pdf
and https://www3.nd.edu/~dthain/compilerbook/compilerbook.pdf

