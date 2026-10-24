let x = 30;
let y = 20;
print(30 + (x * y));

// For loop demonstration
for (let i = 1; i <= 3; i++) {
    print(i);
}

// Increment and decrement demonstration
let a = 10;
a++;
print(a); // 11
a--;
print(a); // 10

//gcc -Iinclude src/main.c src/lexer.c src/parser.c src/ast.c src/evaluator.c -o minilang.exe -Wall -Wextra -std=c99 -O2
// .\minilang.exe .\demo.ml