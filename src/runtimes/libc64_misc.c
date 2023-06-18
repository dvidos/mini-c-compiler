typedef unsigned long size_t;


int strlen(char *s) {
    int len = 0;
    while (*s != 0) {
        len++;
        s++;
    }
    return len;
}



int sys_write(unsigned int fd, const char *buffer, size_t length);
extern unsigned int stdout;

void print(char *msg) {
    sys_write(stdout, msg, strlen(msg));
}

