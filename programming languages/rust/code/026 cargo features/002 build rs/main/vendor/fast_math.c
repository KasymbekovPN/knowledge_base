int fast_square(int x) { return x * x; }

long fast_factorial(int n) {
    long result = 1;
    for (int i = 2; i <= n; i++) result *= i;

    return result;
}
