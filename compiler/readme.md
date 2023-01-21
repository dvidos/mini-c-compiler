# Writing a compiler

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

## Phases

1. parser. read the file(s) and parse all the items
2. ast. by going over the tokens, you build the abstract syntax tree, 
a data oriented representation of the code
3. generate. targeting a specific CPU, you emit either assembly
or machine code for it

We'll need some good data structures (dictionaries, list, trees) for this.

## Giants

Of course I'm relying on giants here, I'm here to discover and learn.

See https://compilers.iecc.com/crenshaw/tutorfinal.pdf
and https://www3.nd.edu/~dthain/compilerbook/compilerbook.pdf


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
int
identifier "d"
=
numeric_literal "2"
+
numeric_literal "3"
*
numeric_literal "4"
;
int
identifier "e"
=
(
numeric_literal "2"
+
numeric_literal "3"
)
*
numeric_literal "4"
;
int
identifier "f"
=
numeric_literal "1"
+
numeric_literal "2"
+
numeric_literal "3"
+
numeric_literal "4"
;
int
identifier "f"
=
numeric_literal "1"
*
numeric_literal "2"
+
numeric_literal "3"
*
numeric_literal "4"
;
int
identifier "f"
=
numeric_literal "1"
+
numeric_literal "2"
*
numeric_literal "3"
+
numeric_literal "4"
;
if
(
identifier "c"
>
numeric_literal "3"
)
{
return
numeric_literal "1"
;
}
else
{
return
identifier "c"
;
}
int
identifier "x"
=
numeric_literal "3"
;
while
(
identifier "x"
>
numeric_literal "0"
)
{
if
(
identifier "x"
>
numeric_literal "10"
)
continue
;
if
(
identifier "x"
<
numeric_literal "3"
)
break
;
}
}
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
