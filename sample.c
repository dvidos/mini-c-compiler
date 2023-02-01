/// @brief 
/// @return

int func_no_args() {
    return 1;
}

int func_one_arg(int a) {
    return 1;
}

int func_two_args(int a, char b) {
    return 1;
}

int func_three_args(int a, char b, bool c) {
    return 1;
}

int func_four_args(int a, char b, bool c, int d) {
    return 1;
}

int main() {
    int x;
    x = func_no_args();
    x = func_one_arg(123);
    x = func_two_args(123, 456);
    x = func_three_args(123, 456, 789);
    x = func_four_args(123, 456, 0, 555);
}
