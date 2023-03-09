INPUT=instab.dat
OUTPUT=instab.h
TOOL=mkinstab


$(OUTPUT): $(INPUT) $(TOOL).c
	./$(TOOL) $(INPUT) $(OUTPUT)

$(TOOL): $(TOOL).c makefile
	gcc -g -o $(TOOL) $(TOOL).c

