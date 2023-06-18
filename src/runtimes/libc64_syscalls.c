// list of x86_64 syscalls:
// http://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/


typedef unsigned long size_t;

unsigned int stdin  = 0;
unsigned int stdout = 1;
unsigned int stderr = 2;

extern long syscall(long sysno, long a1, long a2, long a3, long a4, long a5, long a6);

#define SYSCALL0(sysno)                           syscall(sysno, 0, 0, 0, 0, 0, 0)
#define SYSCALL1(sysno, a1)                       syscall(sysno, (long)a1, 0, 0, 0, 0, 0)
#define SYSCALL2(sysno, a1, a2)                   syscall(sysno, (long)a1, (long)a2, 0, 0, 0, 0)
#define SYSCALL3(sysno, a1, a2, a3)               syscall(sysno, (long)a1, (long)a2, (long)a3, 0, 0, 0)
#define SYSCALL4(sysno, a1, a2, a3, a4)           syscall(sysno, (long)a1, (long)a2, (long)a3, (long)a4, 0, 0)
#define SYSCALL5(sysno, a1, a2, a3, a4, a5)       syscall(sysno, (long)a1, (long)a2, (long)a3, (long)a4, (long)a5, 0)
#define SYSCALL6(sysno, a1, a2, a3, a4, a5, a6)   syscall(sysno, (long)a1, (long)a2, (long)a3, (long)a4, (long)a5, (long)a6)


int sys_read(unsigned int fd, char *buffer, size_t length) {
    return SYSCALL3(0, fd, buffer, length);
}

int sys_write(unsigned int fd, const char *buffer, size_t length) {
    return SYSCALL3(1, fd, buffer, length);
}

unsigned int sys_open(const char *filename, int flags, int mode) {
    return SYSCALL3(2, filename, flags, mode);
}

int sys_close(unsigned int fd) {
    return SYSCALL1(3, fd);
}

int sys_getpid() {
    return SYSCALL0(39);
}

int sys_fork() {
    return SYSCALL0(57);
}

int sys_execve(const char *filename, const char *const argv[], const char *const envp[]) {
    return SYSCALL3(59, filename, argv, envp);
}

