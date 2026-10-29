// 12_modulo_break_continue.ml
// Demonstration of modulo (%) operator, break, and continue statements

// 1. Modulo Operator (%)
print(10 % 3);  // 1
print(20 % 5);  // 0
print(17 % 7);  // 3

// 2. Break inside while loop (stops when i == 4)
let i = 1;
while (i <= 10) {
    if (i == 4) {
        break;
    }
    print(i);
    i++;
}

// 3. Continue inside while loop (skips 3)
let j = 0;
while (j < 5) {
    j++;
    if (j == 3) {
        continue;
    }
    print(j);
}

// 4. Break inside for loop (stops when k == 4)
for (let k = 1; k <= 10; k++) {
    if (k == 4) {
        break;
    }
    print(k);
}

// 5. Continue inside for loop (skips 3)
for (let m = 1; m <= 5; m++) {
    if (m == 3) {
        continue;
    }
    print(m);
}

// 6. Combining modulo and continue: print only odd numbers from 1 to 7
for (let n = 1; n <= 7; n++) {
    if (n % 2 == 0) {
        continue;
    }
    print(n);
}

