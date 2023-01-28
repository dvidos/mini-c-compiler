int global_a;

int func1() {
    int x;
    int y;
}

bool func2(int a, char b) {
    int local;
}

// we must run tests with function calling (0, 1, 2 arguments)
// and with array indexing, 1 or 2 dimensions

int main(int argc, char **argv) {
    func1();
    func2(5);
    return argc - 1;
}
