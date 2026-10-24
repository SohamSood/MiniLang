// Stage 9: Pre and Post Increment / Decrement Demonstrations

// 1. Post-increment (a++) and Pre-increment (++a) as statements
let a = 10;
a++;
print(a); // 11
++a;
print(a); // 12

// 2. Post-decrement (a--) and Pre-decrement (--a) as statements
a--;
print(a); // 11
--a;
print(a); // 10

// 3. Post-increment in expressions (returns old value, then increments)
let x = 5;
print(x++); // prints 5
print(x);   // prints 6

// 4. Pre-increment in expressions (increments, then returns new value)
print(++x); // prints 7
print(x);   // prints 7

// 5. Post-decrement in expressions (returns old value, then decrements)
print(x--); // prints 7
print(x);   // prints 6

// 6. Pre-decrement in expressions (decrements, then returns new value)
print(--x); // prints 5
print(x);   // prints 5

// 7. Used in arithmetic expressions
let y = 10;
let z = y++ + 100;
print(z); // prints 110 (10 + 100)
print(y); // prints 11

let w = ++y + 100;
print(w); // prints 112 (12 + 100)
print(y); // prints 12
