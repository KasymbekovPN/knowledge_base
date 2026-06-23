module math3;

int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b + helper(0);
}

int helper(int a) {
    return a * 2;
}
