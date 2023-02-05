/*
    C file:
    int main() {
        return 0x12345678;
    }

    32 bits:
    0000118d <main>:
        118d:   55                      push   ebp
        118e:   89 e5                   mov    ebp,esp
        1190:   e8 0c 00 00 00          call   11a1 <__x86.get_pc_thunk.ax>
        1195:   05 47 2e 00 00          add    eax,0x2e47
        119a:   b8 78 56 34 12          mov    eax,0x12345678
        119f:   5d                      pop    ebp
        11a0:   c3                      ret

    64 bits:
    0000000000001129 <main>:
        1129:   f3 0f 1e fa             endbr64
        112d:   55                      push   rbp
        112e:   48 89 e5                mov    rbp,rsp
        1131:   b8 78 56 34 12          mov    eax,0x12345678
        1136:   5d                      pop    rbp
        1137:   c3                      ret
*/

int main() {
    return 0x12345678;
}
