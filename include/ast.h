#ifndef AST_H
#define AST_H

/*
 * ast.h
 * -----
 * Defines the data structures used to represent the
 * Abstract Syntax Tree (AST) of a MiniLang program.
 * Also provides declarations for creating and managing AST nodes.
 */

typedef enum {
    AST_NUMBER,
    AST_STRING,
    AST_VAR,
    AST_UNARY_OP,
    AST_BINARY_OP,
    AST_PRINT,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_BLOCK,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_INC_DEC,
    AST_FUNC_DECL,
    AST_CALL,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_PROGRAM
} ASTNodeType;

typedef enum {
    UNARY_NEGATE, // -x
    UNARY_NOT     // !x
} UnaryOpType;

typedef enum {
    OP_ADD,       // +
    OP_SUB,       // -
    OP_MUL,       // *
    OP_DIV,       // /
    OP_MOD,       // %
    OP_EQ,        // ==
    OP_NEQ,       // !=
    OP_LT,        // <
    OP_LTE,       // <=
    OP_GT,        // >
    OP_GTE,       // >=
    OP_AND,       // and, &, &&
    OP_OR,        // or, |, ||
    OP_XOR,       // xor, ^
    OP_NOR,       // nor
    OP_NAND       // nand
} BinaryOpType;

typedef enum {
    OP_PRE_INC,   // ++a
    OP_POST_INC,  // a++
    OP_PRE_DEC,   // --a
    OP_POST_DEC   // a--
} UpdateOpType;

struct ASTNode;

typedef struct ASTNode {
    ASTNodeType type;
    int line; // Source line for runtime error reporting

    union {
        // AST_NUMBER: Numeric literal
        struct {
            double value;
        } number;

        // AST_STRING: String literal
        struct {
            char* value;
        } string;

        // AST_VAR: Variable access
        struct {
            char name[64];
        } var;

        // AST_UNARY_OP: Unary operations (-x, !x)
        struct {
            UnaryOpType op;
            struct ASTNode* operand;
        } unary_op;

        // AST_BINARY_OP: Binary operations (arithmetic and comparison)
        struct {
            BinaryOpType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        // AST_INC_DEC: ++a, a++, --a, a--
        struct {
            char name[64];
            UpdateOpType op;
        } inc_dec;

        // AST_PRINT: print(expr);
        struct {
            struct ASTNode* expr;
        } print_stmt;

        // AST_VAR_DECL: let x = expr;
        struct {
            char name[64];
            struct ASTNode* init_expr;
        } var_decl;

        // AST_ASSIGN: x = expr;
        struct {
            char name[64];
            struct ASTNode* expr;
        } assign;

        // AST_BLOCK or AST_PROGRAM: List of statements
        struct {
            struct ASTNode** statements;
            int count;
            int capacity;
        } block;

        // AST_IF: if (cond) { ... } else { ... }
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch; // AST_BLOCK
            struct ASTNode* else_branch; // AST_BLOCK (nullable)
        } if_stmt;

        // AST_WHILE: while (cond) { ... }
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;        // AST_BLOCK
        } while_stmt;

        // AST_FOR: for (init; condition; update) { ... }
        struct {
            struct ASTNode* init;        // nullable
            struct ASTNode* condition;   // nullable
            struct ASTNode* update;      // nullable
            struct ASTNode* body;        // AST_BLOCK
        } for_stmt;

        // AST_FUNC_DECL: function name(p1, p2) { ... }
        struct {
            char name[64];
            char** params;
            int param_count;
            struct ASTNode* body;        // AST_BLOCK
        } func_decl;

        // AST_CALL: name(arg1, arg2)
        struct {
            char name[64];
            struct ASTNode** args;
            int arg_count;
        } call;

        // AST_RETURN: return expr;
        struct {
            struct ASTNode* expr;        // nullable
        } return_stmt;
    } data;
} ASTNode;

// AST Node Constructors
ASTNode* ast_new_number(double value, int line);
ASTNode* ast_new_string(const char* value, int line);
ASTNode* ast_new_var(const char* name, int line);
ASTNode* ast_new_unary_op(UnaryOpType op, ASTNode* operand, int line);
ASTNode* ast_new_binary_op(BinaryOpType op, ASTNode* left, ASTNode* right, int line);
ASTNode* ast_new_inc_dec(UpdateOpType op, const char* name, int line);
ASTNode* ast_new_print(ASTNode* expr, int line);
ASTNode* ast_new_var_decl(const char* name, ASTNode* init_expr, int line);
ASTNode* ast_new_assign(const char* name, ASTNode* expr, int line);
ASTNode* ast_new_block(int line);
void     ast_block_add(ASTNode* block, ASTNode* stmt);
ASTNode* ast_new_program(void);
ASTNode* ast_new_if(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line);
ASTNode* ast_new_while(ASTNode* cond, ASTNode* body, int line);
ASTNode* ast_new_for(ASTNode* init, ASTNode* condition, ASTNode* update, ASTNode* body, int line);
ASTNode* ast_new_func_decl(const char* name, char** params, int param_count, ASTNode* body, int line);
ASTNode* ast_new_call(const char* name, ASTNode** args, int arg_count, int line);
ASTNode* ast_new_return(ASTNode* expr, int line);
ASTNode* ast_new_break(int line);
ASTNode* ast_new_continue(int line);

// Memory cleanup
void ast_free(ASTNode* node);

// Visual debug printing
void ast_print(ASTNode* node);
void ast_print_compact(ASTNode* node);

// Helpers for operator strings
const char* unary_op_to_string(UnaryOpType op);
const char* binary_op_to_string(BinaryOpType op);
const char* update_op_to_string(UpdateOpType op);

#endif // AST_H
