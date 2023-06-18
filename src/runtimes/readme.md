# runtimes

The idea is that, as a compiler, we want to allow the user to write just a "Hello World" program.

In order for this so compile and run, we need to know a few things about the runtime 
that the program is going to execute under, e.g. how the "sys_write" function is called.

This folder contains a minimum set of runtime code, that allows our programs to run in 
x86_64 linux. I wrote these to avoid the complexity and clutter of the real libc.a library.

User programs should be compiled against these objects and include these header files.

