int normalFunc(int x) {
    return x * 2;
}

__attribute__((pure)) int pureFunc(int x) {
    return x * 2;
}

__attribute__((const)) int constFunc(int x) {
    return x * 2;
}