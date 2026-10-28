// Test Suite: Binary Logic and Bitwise Operations (and, or, xor, nor, nand)

// 1. Truth table for AND (both keywords and symbols)
print(1 and 1);  // 1
print(1 and 0);  // 0
print(0 and 1);  // 0
print(0 and 0);  // 0
print(1 & 1);    // 1
print(1 && 0);   // 0

// 2. Truth table for OR
print(1 or 1);   // 1
print(1 or 0);   // 1
print(0 or 1);   // 1
print(0 or 0);   // 0
print(1 | 0);    // 1
print(0 || 0);   // 0

// 3. Truth table for XOR
print(1 xor 1);  // 0
print(1 xor 0);  // 1
print(0 xor 1);  // 1
print(0 xor 0);  // 0
print(1 ^ 0);    // 1
print(1 ^ 1);    // 0

// 4. Truth table for NOR
print(1 nor 1);  // 0
print(1 nor 0);  // 0
print(0 nor 1);  // 0
print(0 nor 0);  // 1

// 5. Truth table for NAND
print(1 nand 1); // 0
print(1 nand 0); // 1
print(0 nand 1); // 1
print(0 nand 0); // 1

// 6. Bitwise operations on integers
let p = 5; // 101 in binary
let q = 3; // 011 in binary
print(p and q); // 5 & 3 = 1 (001)
print(p or q);  // 5 | 3 = 7 (111)
print(p xor q); // 5 ^ 3 = 6 (110)

// 7. Precedence with comparisons and arithmetic
let a = 10;
let b = 20;
if (a < 15 and b > 15) {
    print(100); // 100
} else {
    print(200);
}

if (a == 10 xor b == 20) {
    print(300);
} else {
    print(400); // 400 (both are true, so XOR is false)
}

if (a > 100 nor b > 100) {
    print(500); // 500 (neither is > 100, so NOR is true)
}

