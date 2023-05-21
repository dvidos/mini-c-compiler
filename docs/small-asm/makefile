

result: a.out
	objdump -xd $< > result.txt
	cat result.txt
	./$<; echo "$< returned" $$?
	#rm small.o


a.out: small.o
	ld $<

small.o: small.S
	as -o $@ $<


