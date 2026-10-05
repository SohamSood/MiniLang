#ifndef EVALUATOR_H
#define EVALUATOR_H

/*
 * ast.h
 * -----
 * Defines the data structures used to represent the
 * Abstract Syntax Tree (AST) of a MiniLang program.
 * Also provides declarations for creating and managing AST nodes.
 */

 
#include "ast.h"

// Result of evaluating an AST node
typedef struct {
    int is_return; // Flag indicating a 'return' statement was encountered
    double value;  // Value associated with expression or return
} EvalResult;

// Variable entry in the symbol table
typedef struct {
    char name[64];
    double value;
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

// Environment management
Environment* env_create(Environment* parent);
void         env_free(Environment* env);
int          env_set(Environment* env, const char* name, double value);
int          env_get(Environment* env, const char* name, double* out_value);
int          env_assign(Environment* env, const char* name, double value);

// Function table management
void         function_table_init(FunctionTable* ft);
int          function_table_add(FunctionTable* ft, const char* name, ASTNode* decl);
ASTNode*     function_table_get(FunctionTable* ft, const char* name);

// Evaluation functions
EvalResult   eval_node(ASTNode* node, Environment* env, FunctionTable* ft);
void         evaluate(ASTNode* root);

#endif // EVALUATOR_H
