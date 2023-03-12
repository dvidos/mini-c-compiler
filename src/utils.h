#include <stdbool.h>


void print_16_hex(void *buffer, int size, int indent);
void print_pretty(char *str);


bool load_text(char *filemame, char **buffer);
bool save_text(char *filename, char *buffer);

unsigned long round_up(unsigned long value, unsigned threshold);
char *set_extension(char *path, char *extension);
