// Test 1: String literals & print
let greeting = "Hello, MiniLang!";
print(greeting);

// Test 2: String concatenation (+)
let first = "Compiler ";
let second = "Engineering";
let full = first + second;
print(full);

// Test 3: String equality (== and !=)
let is_match = (full == "Compiler Engineering");
print(is_match);

let not_match = (full != "Wrong Text");
print(not_match);

// Test 4: Unary Negation (-)
let pos = 42;
let neg = -pos;
print(neg);
print(-(-100));

// Test 5: Unary Logical NOT (!)
let is_valid = 1;
print(!is_valid);
print(!0);
print(!"");
print(!"Has Content");

// Test 6: Strings in conditions and functions
function greetUser(name) {
    return "Welcome, " + name + "!";
}

let msg = greetUser("Soham");
print(msg);

// Test 7: Type Error verification (Number + String)
print("--- Type Error Handling Tests ---");
let a = 10;
let b = " apples";
let bad = a + b;

