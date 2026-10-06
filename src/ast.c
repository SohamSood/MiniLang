/*
 * ast.c
 * -----
 * Contains functions for creating and managing AST nodes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
static ASTNode* create_node(ASTNodeType type, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: out of memory allocating AST node\n");
        exit(1);
    }
    node->type = type;
    node->line = line;
    memset(&node->data, 0, sizeof(node->data));
    return node;
}

ASTNode* ast_new_number(double value, int line) {
    ASTNode* node = create_node(AST_NUMBER, line);
    node->data.number.value = value;
    return node;
}

ASTNode* ast_new_var(const char* name, int line) {
    ASTNode* node = create_node(AST_VAR, line);
    strncpy(node->data.var.name, name, sizeof(node->data.var.name) - 1);
    node->data.var.name[sizeof(node->data.var.name) - 1] = '\0';
    return node;
}

ASTNode* ast_new_binary_op(BinaryOpType op, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = create_node(AST_BINARY_OP, line);
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode* ast_new_print(ASTNode* expr, int line) {
    ASTNode* node = create_node(AST_PRINT, line);
    node->data.print_stmt.expr = expr;
    return node;
}

ASTNode* ast_new_var_decl(const char* name, ASTNode* init_expr, int line) {
    ASTNode* node = create_node(AST_VAR_DECL, line);
    strncpy(node->data.var_decl.name, name, sizeof(node->data.var_decl.name) - 1);
    node->data.var_decl.name[sizeof(node->data.var_decl.name) - 1] = '\0';
    node->data.var_decl.init_expr = init_expr;
    return node;
}

ASTNode* ast_new_assign(const char* name, ASTNode* expr, int line) {
    ASTNode* node = create_node(AST_ASSIGN, line);
    strncpy(node->data.assign.name, name, sizeof(node->data.assign.name) - 1);
    node->data.assign.name[sizeof(node->data.assign.name) - 1] = '\0';
    node->data.assign.expr = expr;
    return node;
}

ASTNode* ast_new_block(int line) {
    ASTNode* node = create_node(AST_BLOCK, line);
    node->data.block.capacity = 8;
    node->data.block.count = 0;
    node->data.block.statements = (ASTNode**)malloc(sizeof(ASTNode*) * node->data.block.capacity);
    return node;
}

void ast_block_add(ASTNode* block, ASTNode* stmt) {
    if (!block || !stmt) return;
    if (block->data.block.count >= block->data.block.capacity) {
        block->data.block.capacity *= 2;
        block->data.block.statements = (ASTNode**)realloc(
            block->data.block.statements,
            sizeof(ASTNode*) * block->data.block.capacity
        );
    }
    block->data.block.statements[block->data.block.count++] = stmt;
}

ASTNode* ast_new_program(void) {
    ASTNode* node = ast_new_block(1);
    node->type = AST_PROGRAM;
    return node;
}

ASTNode* ast_new_if(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line) {
    ASTNode* node = create_node(AST_IF, line);
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_new_while(ASTNode* cond, ASTNode* body, int line) {
    ASTNode* node = create_node(AST_WHILE, line);
    node->data.while_stmt.condition = cond;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode* ast_new_func_decl(const char* name, char** params, int param_count, ASTNode* body, int line) {
    ASTNode* node = create_node(AST_FUNC_DECL, line);
    strncpy(node->data.func_decl.name, name, sizeof(node->data.func_decl.name) - 1);
    node->data.func_decl.name[sizeof(node->data.func_decl.name) - 1] = '\0';
    node->data.func_decl.params = params;
    node->data.func_decl.param_count = param_count;
    node->data.func_decl.body = body;
    return node;
}

ASTNode* ast_new_call(const char* name, ASTNode** args, int arg_count, int line) {
    ASTNode* node = create_node(AST_CALL, line);
    strncpy(node->data.call.name, name, sizeof(node->data.call.name) - 1);
    node->data.call.name[sizeof(node->data.call.name) - 1] = '\0';
    node->data.call.args = args;
    node->data.call.arg_count = arg_count;
    return node;
}

ASTNode* ast_new_return(ASTNode* expr, int line) {
    ASTNode* node = create_node(AST_RETURN, line);
    node->data.return_stmt.expr = expr;
    return node;
}

const char* binary_op_to_string(BinaryOpType op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
        case OP_LT:  return "<";
        case OP_LTE: return "<=";
        case OP_GT:  return ">";
        case OP_GTE: return ">=";
        default: return "?";
    }
}

void ast_free(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_NUMBER:
        case AST_VAR:
            break;

        case AST_BINARY_OP:
            ast_free(node->data.binary_op.left);
            ast_free(node->data.binary_op.right);
            break;

        case AST_PRINT:
            ast_free(node->data.print_stmt.expr);
            break;

        case AST_VAR_DECL:
            ast_free(node->data.var_decl.init_expr);
            break;

        case AST_ASSIGN:
            ast_free(node->data.assign.expr);
            break;

        case AST_BLOCK:
        case AST_PROGRAM:
            for (int i = 0; i < node->data.block.count; i++) {
                ast_free(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;

        case AST_IF:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch) {
                ast_free(node->data.if_stmt.else_branch);
            }
            break;

        case AST_WHILE:
            ast_free(node->data.while_stmt.condition);
            ast_free(node->data.while_stmt.body);
            break;

        case AST_FUNC_DECL:
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                free(node->data.func_decl.params[i]);
            }
            free(node->data.func_decl.params);
            ast_free(node->data.func_decl.body);
            break;

        case AST_CALL:
            for (int i = 0; i < node->data.call.arg_count; i++) {
                ast_free(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;

        case AST_RETURN:
            if (node->data.return_stmt.expr) {
                ast_free(node->data.return_stmt.expr);
            }
            break;
    }

    free(node);
}

// Compact single-line printer for expressions (e.g. PRINT((10 + 20)))
void ast_print_compact(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_NUMBER:
            if ((long long)node->data.number.value == node->data.number.value) {
                printf("%lld", (long long)node->data.number.value);
            } else {
                printf("%g", node->data.number.value);
            }
            break;

        case AST_VAR:
            printf("%s", node->data.var.name);
            break;

        case AST_BINARY_OP:
            printf("(");
            ast_print_compact(node->data.binary_op.left);
            printf(" %s ", binary_op_to_string(node->data.binary_op.op));
            ast_print_compact(node->data.binary_op.right);
            printf(")");
            break;

        case AST_PRINT:
            printf("PRINT(");
            ast_print_compact(node->data.print_stmt.expr);
            printf(")");
            break;

        case AST_VAR_DECL:
            printf("LET %s = ", node->data.var_decl.name);
            ast_print_compact(node->data.var_decl.init_expr);
            break;

        case AST_ASSIGN:
            printf("%s = ", node->data.assign.name);
            ast_print_compact(node->data.assign.expr);
            break;

        case AST_CALL:
            printf("%s(", node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                ast_print_compact(node->data.call.args[i]);
                if (i + 1 < node->data.call.arg_count) printf(", ");
            }
            printf(")");
            break;

        case AST_RETURN:
            printf("RETURN ");
            if (node->data.return_stmt.expr) {
                ast_print_compact(node->data.return_stmt.expr);
            }
            break;

        default:
            printf("<node:%d>", node->type);
            break;
    }
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static void ast_print_indented(ASTNode* node, int indent) {
    if (!node) return;

    print_indent(indent);
    switch (node->type) {
        case AST_NUMBER:
            if ((long long)node->data.number.value == node->data.number.value) {
                printf("NUMBER: %lld\n", (long long)node->data.number.value);
            } else {
                printf("NUMBER: %g\n", node->data.number.value);
            }
            break;

        case AST_VAR:
            printf("VAR: %s\n", node->data.var.name);
            break;

        case AST_BINARY_OP:
            printf("BINARY_OP (%s):\n", binary_op_to_string(node->data.binary_op.op));
            ast_print_indented(node->data.binary_op.left, indent + 1);
            ast_print_indented(node->data.binary_op.right, indent + 1);
            break;

        case AST_PRINT:
            printf("PRINT:\n");
            ast_print_indented(node->data.print_stmt.expr, indent + 1);
            break;

        case AST_VAR_DECL:
            printf("VAR_DECL (let %s):\n", node->data.var_decl.name);
            ast_print_indented(node->data.var_decl.init_expr, indent + 1);
            break;

        case AST_ASSIGN:
            printf("ASSIGN (%s = ...):\n", node->data.assign.name);
            ast_print_indented(node->data.assign.expr, indent + 1);
            break;

        case AST_BLOCK:
            printf("BLOCK (%d statements):\n", node->data.block.count);
            for (int i = 0; i < node->data.block.count; i++) {
                ast_print_indented(node->data.block.statements[i], indent + 1);
            }
            break;

        case AST_PROGRAM:
            printf("PROGRAM:\n");
            for (int i = 0; i < node->data.block.count; i++) {
                ast_print_indented(node->data.block.statements[i], indent + 1);
            }
            break;

        case AST_IF:
            printf("IF:\n");
            print_indent(indent + 1); printf("CONDITION:\n");
            ast_print_indented(node->data.if_stmt.condition, indent + 2);
            print_indent(indent + 1); printf("THEN:\n");
            ast_print_indented(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                print_indent(indent + 1); printf("ELSE:\n");
                ast_print_indented(node->data.if_stmt.else_branch, indent + 2);
            }
            break;

        case AST_WHILE:
            printf("WHILE:\n");
            print_indent(indent + 1); printf("CONDITION:\n");
            ast_print_indented(node->data.while_stmt.condition, indent + 2);
            print_indent(indent + 1); printf("BODY:\n");
            ast_print_indented(node->data.while_stmt.body, indent + 2);
            break;

        case AST_FUNC_DECL:
            printf("FUNCTION_DECL (%s):\n", node->data.func_decl.name);
            print_indent(indent + 1);
            printf("PARAMS: (");
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                printf("%s%s", node->data.func_decl.params[i], (i + 1 < node->data.func_decl.param_count) ? ", " : "");
            }
            printf(")\n");
            print_indent(indent + 1); printf("BODY:\n");
            ast_print_indented(node->data.func_decl.body, indent + 2);
            break;

        case AST_CALL:
            printf("CALL (%s):\n", node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                ast_print_indented(node->data.call.args[i], indent + 1);
            }
            break;

        case AST_RETURN:
            printf("RETURN:\n");
            if (node->data.return_stmt.expr) {
                ast_print_indented(node->data.return_stmt.expr, indent + 1);
            }
            break;
    }
}

void ast_print(ASTNode* node) {
    ast_print_indented(node, 0);
}