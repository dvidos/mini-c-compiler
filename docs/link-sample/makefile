


target:
	gcc -c file1.c
	gcc -c file2.c
	ld -static \
        -L/usr/lib/gcc/x86_64-linux-gnu/11 \
        -L/usr/lib/x86_64-linux-gnu \
        -L/lib/x86_64-linux-gnu \
        -L/usr/lib \
        -L/lib \
        /usr/lib/x86_64-linux-gnu/crt1.o \
        /usr/lib/x86_64-linux-gnu/crti.o \
        /usr/lib/gcc/x86_64-linux-gnu/11/crtbeginT.o  \
        file1.o  \
        file2.o  \
        --start-group -lgcc -lgcc_eh -lc --end-group  \
        /usr/lib/gcc/x86_64-linux-gnu/11/crtend.o \
        /usr/lib/x86_64-linux-gnu/crtn.o

# -lc <= was my humble switch
# --start-group -lgcc -lgcc_eh -lc --end-group 


