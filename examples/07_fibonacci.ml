// Stage 7: Comprehensive Project Test (Recursion, Control Flow, and Functions)
function fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

let i = 0;
while (i <= 7) {
    print(fib(i));
    i = i + 1;
}
