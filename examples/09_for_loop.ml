// Stage 8: For Loop Demonstrations

// 1. Basic for loop with post-increment (i++)
let sum = 0;
for (let i = 1; i <= 5; i++) {
    print(i);
    sum = sum + i;
}
print(sum);

// 2. For loop with pre-increment (++j)
for (let j = 1; j <= 3; ++j) {
    print(j * 10);
}

// 3. Reverse for loop with post-decrement (k--)
for (let k = 5; k >= 1; k--) {
    print(k);
}

// 4. Nested for loop
for (let r = 1; r <= 2; r++) {
    for (let c = 1; c <= 2; c++) {
        print(r * 100 + c);
    }
}

// 5. For loop inside function with return
function find_target() {
    for (let n = 1; n <= 10; n++) {
        if (n == 4) {
            return n;
        }
    }
    return 0;
}
print(find_target());
