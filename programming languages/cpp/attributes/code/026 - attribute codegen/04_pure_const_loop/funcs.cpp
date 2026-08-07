int global = 0;

__attribute__((pure)) int pureRead(int x) {
    return x + global;
}

__attribute__((const)) int constCompute(int x) {
    return x * 2;
}