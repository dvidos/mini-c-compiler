# $@ target, $< first dependency, $^ all dependencies


default: elf-diff asm-diff obj_elf32 obj_elf64 obj_asm32 obj_asm64

elf-diff: elf32 elf64
	diff -y $^ > $@; true

asm-diff: asm32 asm64
	diff -y $^ > $@; true

elf32: tiny32 
	readelf -a $< > $@

elf64: tiny64
	readelf -a $< > $@

asm32: tiny32
	objdump -d -M "intel" $< > $@

asm64: tiny64
	objdump -d -M "intel" $< > $@

obj_elf32: tiny32.o
	readelf -a $< > $@

obj_elf64: tiny64.o
	readelf -a $< > $@

obj_asm32: tiny32.o
	objdump -d -M "intel" $< > $@

obj_asm64: tiny64.o
	objdump -d -M "intel" $< > $@

tiny32: tiny.c
	gcc -O0 -m32 -std=c99 -o $@ $<

tiny64: tiny.c
	gcc -O0 -m64 -std=c99 -o $@ $<

tiny32.o: tiny.c
	gcc -O0 -m32 -std=c99 -c -o $@ $<

tiny64.o: tiny.c
	gcc -O0 -m64 -std=c99 -c -o $@ $<

clean:
	rm -f tiny32 tiny64 asm32 asm64 elf32 elf64 elf-diff asm-diff

