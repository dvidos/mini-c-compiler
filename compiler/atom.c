#include <stdlib.h>

struct {
    char *buffer;
    int capacity;
    int length;
} atom;

void init_atom() {
    atom.capacity = 32;
    atom.buffer = malloc(atom.capacity);
    atom.length = 0;
    atom.buffer[atom.length] = '\0';
}

void clear_atom() {
    atom.length = 0;
    atom.buffer[atom.length] = '\0';
}

void extend_atom(char c) {
    if (atom.length + 1 >= atom.capacity) {
        atom.capacity *= 2;
        atom.buffer = realloc(atom.buffer, atom.capacity);
    }
    atom.buffer[atom.length++] = c;
    atom.buffer[atom.length] = '\0';
}

int atom_len() {
    return atom.length;
}

char *get_atom() {
    return atom.buffer;
}
