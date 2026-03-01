// expected: 120

int factorial(int n) {
  if (n <= 1) {
    return 1;
  }
  // recursion：n * (n - 1)!
  return n * factorial(n - 1);
}

int main() {
  int input = 5;
  int result;
  result = factorial(input);
  return result;
}
