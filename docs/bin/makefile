# $@ target, $< first dependency, $^ all dependencies
BASE = tiny
STAT = $(BASE)-stat
DYN = $(BASE)-dyn
OBJ = $(BASE)-obj

STAT32 = $(STAT)32
STAT64 = $(STAT)64
DYN32 = $(DYN)32
DYN64 = $(DYN)64
OBJ32 = $(OBJ)32
OBJ64 = $(OBJ)64







default: \
	$(STAT32).elf $(STAT32).asm \
	$(STAT64).elf $(STAT64).asm \
	$(DYN32).elf $(DYN32).asm \
	$(DYN64).elf $(DYN64).asm \
	$(OBJ32).elf $(OBJ32).asm \
	$(OBJ64).elf $(OBJ64).asm

%.elf: %
	readelf -a $< > $@

%.asm: %
	objdump -d -M "intel" $< > $@

tiny-dyn32: tiny.c
	gcc -O0 -m32 -std=c99 -o $@ $<

tiny-dyn64: tiny.c
	gcc -O0 -m64 -std=c99 -o $@ $<

tiny-stat32: tiny.c
	gcc -m32 -static -o $@ $<

tiny-stat64: tiny.c
	gcc -m64 -static -o $@ $<

tiny-obj32: tiny.c
	gcc -O0 -m32 -std=c99 -c -o $@ $<

tiny-obj64: tiny.c
	gcc -O0 -m64 -std=c99 -c -o $@ $<

clean:
	rm -f tiny-dyn32 tiny-dyn64 tiny-stat32 tiny-stat64 tiny-obj32 tiny-obj64 \
		asm32 asm64 elf32 elf64 elf-diff asm-diff \
		obj_* *.elf *.asm


