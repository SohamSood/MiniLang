#ifndef EVALUATOR_H
#define EVALUATOR_H

/*
 * evaluator.h
 * -----------
 * Declares runtime structures and functions for AST evaluation,
 * symbol table environments, and function registries.
 */

 
#include "ast.h"

// Value Types in MiniLang
typedef enum {
    VAL_NUMBER,
    VAL_STRING
} ValueType;

// Dynamic tagged runtime value
typedef struct {
    ValueType type;
    union {
        double number;
        char* string; // Heap-allocated string
    } as;
} Value;

// Result of evaluating an AST node
typedef struct {
    int is_return;   // Flag indicating a 'return' statement was encountered
    int is_break;    // Flag indicating a 'break' statement was encountered
    int is_continue; // Flag indicating a 'continue' statement was encountered
    Value value;     // Value associated with expression or return
} EvalResult;

// Variable entry in the symbol table
typedef struct {
    char name[64];
    Value value;
} Variable;

// Environment supporting nested scopes (e.g. for functions)
typedef struct Environment {
    Variable vars[256];
    int var_count;
    struct Environment* parent; // Pointer to enclosing/parent scope
} Environment;

// Function entry in the function table
typedef struct {
    char name[64];
    ASTNode* decl; // Points to the AST_FUNC_DECL node
} FunctionEntry;

typedef struct {
    FunctionEntry functions[128];
    int function_count;
} FunctionTable;

// Value constructors and helpers
Value        value_make_number(double num);
Value        value_make_string(const char* str);
Value        value_copy(Value val);
void         value_free(Value val);
int          value_is_truthy(Value val);

// Environment management
Environment* env_create(Environment* parent);
void         env_free(Environment* env);
int          env_set(Environment* env, const char* name, Value value);
int          env_get(Environment* env, const char* name, Value* out_value);
int          env_assign(Environment* env, const char* name, Value value);

// Function table management
void         function_table_init(FunctionTable* ft);
int          function_table_add(FunctionTable* ft, const char* name, ASTNode* decl);
ASTNode*     function_table_get(FunctionTable* ft, const char* name);

// Evaluation functions
EvalResult   eval_node(ASTNode* node, Environment* env, FunctionTable* ft);
void         evaluate(ASTNode* root);

#endif // EVALUATOR_H
