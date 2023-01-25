void func() {
    p = &a;
    *p = 123;

    while (i++ < 10)
        a[i] = b[i + 1];
}
