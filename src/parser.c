/*
 * parser.c
 * --------
 * Converts the token stream produced by the lexer into
 * an Abstract Syntax Tree (AST).
 */
// Parses an arithmetic expression while respecting operator precedence.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Forward declarations of parser functions
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_block(Parser* parser);

static void advance(Parser* parser) {
    parser->current_token = lexer_next_token(parser->lexer);
}

static void syntax_error(Parser* parser, const char* message) {
    fprintf(stderr, "Syntax Error [Line %d]: %s (got '%s')\n",
            parser->current_token.line, message, parser->current_token.text);
    parser->has_error = 1;
}

static int match(Parser* parser, TokenType type) {
    return parser->current_token.type == type;
}

static int expect(Parser* parser, TokenType type, const char* message) {
    if (match(parser, type)) {
        advance(parser);
        return 1;
    }
    syntax_error(parser, message);
    return 0;
}

void parser_init(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->has_error = 0;
    advance(parser);
}

static ASTNode* parse_primary(Parser* parser) {
    int line = parser->current_token.line;

    if (match(parser, TOKEN_NUMBER)) {
        double val = parser->current_token.number_val;
        advance(parser);
        return ast_new_number(val, line);
    }

    if (match(parser, TOKEN_MINUS)) {
        advance(parser);
        ASTNode* right = parse_primary(parser);
        return ast_new_binary_op(OP_SUB, ast_new_number(0.0, line), right, line);
    }

    if (match(parser, TOKEN_LPAREN)) {
        advance(parser);
        ASTNode* expr = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "expected ')'");
        return expr;
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        char name[64];
        strncpy(name, parser->current_token.text, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        advance(parser);

        if (match(parser, TOKEN_LPAREN)) {
            advance(parser);
            int capacity = 4;
            int count = 0;
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
            if (!match(parser, TOKEN_RPAREN)) {
                while (1) {
                    if (count >= capacity) {
                        capacity *= 2;
                        args = (ASTNode**)realloc(args, sizeof(ASTNode*) * capacity);
                    }
                    args[count++] = parse_expression(parser);
                    if (match(parser, TOKEN_COMMA)) {
                        advance(parser);
                    } else {
                        break;
                    }
                }
            }
            expect(parser, TOKEN_RPAREN, "expected ')' after function arguments");
            return ast_new_call(name, args, count, line);
        }

        return ast_new_var(name, line);
    }

    syntax_error(parser, "unexpected token in expression");
    advance(parser);
    return NULL;
}

static ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* left = parse_primary(parser);

    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH)) {
        TokenType op_type = parser->current_token.type;
        int line = parser->current_token.line;
        advance(parser);
        ASTNode* right = parse_primary(parser);
        BinaryOpType op = (op_type == TOKEN_STAR) ? OP_MUL : OP_DIV;
        left = ast_new_binary_op(op, left, right, line);
    }

    return left;
}

static ASTNode* parse_additive(Parser* parser) {
    ASTNode* left = parse_multiplicative(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        TokenType op_type = parser->current_token.type;
        int line = parser->current_token.line;
        advance(parser);
        ASTNode* right = parse_multiplicative(parser);
        BinaryOpType op = (op_type == TOKEN_PLUS) ? OP_ADD : OP_SUB;
        left = ast_new_binary_op(op, left, right, line);
    }

    return left;
}

static ASTNode* parse_comparison(Parser* parser) {
    ASTNode* left = parse_additive(parser);

    while (match(parser, TOKEN_GREATER) || match(parser, TOKEN_LESS) ||
           match(parser, TOKEN_GREATER_EQUAL) || match(parser, TOKEN_LESS_EQUAL) ||
           match(parser, TOKEN_EQUAL) || match(parser, TOKEN_NOT_EQUAL)) {
        TokenType op_type = parser->current_token.type;
        int line = parser->current_token.line;
        advance(parser);
        ASTNode* right = parse_additive(parser);

        BinaryOpType op = OP_EQ;
        switch (op_type) {
            case TOKEN_GREATER:       op = OP_GT; break;
            case TOKEN_LESS:          op = OP_LT; break;
            case TOKEN_GREATER_EQUAL: op = OP_GTE; break;
            case TOKEN_LESS_EQUAL:    op = OP_LTE; break;
            case TOKEN_EQUAL:         op = OP_EQ; break;
            case TOKEN_NOT_EQUAL:     op = OP_NEQ; break;
            default: break;
        }

        left = ast_new_binary_op(op, left, right, line);
    }

    return left;
}

static ASTNode* parse_expression(Parser* parser) {
    return parse_comparison(parser);
}

static ASTNode* parse_block(Parser* parser) {
    int line = parser->current_token.line;
    if (!expect(parser, TOKEN_LBRACE, "expected '{' to start block")) {
        return NULL;
    }

    ASTNode* block = ast_new_block(line);

    while (!match(parser, TOKEN_RBRACE) && !match(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            ast_block_add(block, stmt);
        }
        if (parser->has_error) {
            break;
        }
    }

    expect(parser, TOKEN_RBRACE, "expected '}' to end block");
    return block;
}

static ASTNode* parse_var_decl(Parser* parser) {
    int line = parser->current_token.line;
    advance(parser); // consume 'let'

    if (!match(parser, TOKEN_IDENTIFIER)) {
        syntax_error(parser, "expected variable name after 'let'");
        return NULL;
    }

    char name[64];
    strncpy(name, parser->current_token.text, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    advance(parser);

    expect(parser, TOKEN_ASSIGN, "expected '=' after variable name");
    ASTNode* init_expr = parse_expression(parser);
    expect(parser, TOKEN_SEMICOLON, "expected ';' after variable declaration");

    return ast_new_var_decl(name, init_expr, line);
}

static ASTNode* parse_print(Parser* parser) {
    int line = parser->current_token.line;
    advance(parser); // consume 'print'

    expect(parser, TOKEN_LPAREN, "expected '(' after 'print'");
    ASTNode* expr = parse_expression(parser);
    expect(parser, TOKEN_RPAREN, "expected ')' after print expression");
    expect(parser, TOKEN_SEMICOLON, "expected ';' after print statement");

    return ast_new_print(expr, line);
}

static ASTNode* parse_if(Parser* parser) {
    int line = parser->current_token.line;
    advance(parser); // consume 'if'

    expect(parser, TOKEN_LPAREN, "expected '(' after 'if'");
    ASTNode* condition = parse_expression(parser);
    expect(parser, TOKEN_RPAREN, "expected ')' after condition");

    ASTNode* then_branch = parse_block(parser);
    ASTNode* else_branch = NULL;

    if (match(parser, TOKEN_ELSE)) {
        advance(parser);
        if (match(parser, TOKEN_IF)) {
            ASTNode* nested_if = parse_if(parser);
            else_branch = ast_new_block(line);
            ast_block_add(else_branch, nested_if);
        } else {
            else_branch = parse_block(parser);
        }
    }

    return ast_new_if(condition, then_branch, else_branch, line);
}

static ASTNode* parse_while(Parser* parser) {
    int line = parser->current_token.line;
    advance(parser); // consume 'while'

    expect(parser, TOKEN_LPAREN, "expected '(' after 'while'");
    ASTNode* condition = parse_expression(parser);
    expect(parser, TOKEN_RPAREN, "expected ')' after while condition");

    ASTNode* body = parse_block(parser);

    return ast_new_while(condition, body, line);
}

static ASTNode* parse_func_decl(Parser* parser) {
    int line = parser->current_token.line;
    advance(parser); // consume 'function'

    if (!match(parser, TOKEN_IDENTIFIER)) {
        syntax_error(parser, "expected function name after 'function'");
        return NULL;
    }

    char name[64];
    strncpy(name, parser->current_token.text, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    advance(parser);

    expect(parser, TOKEN_LPAREN, "expected '(' after function name");

    int capacity = 4;
    int param_count = 0;
    char** params = (char**)malloc(sizeof(char*) * capacity);

    if (!match(parser, TOKEN_RPAREN)) {
        while (1) {
            if (!match(parser, TOKEN_IDENTIFIER)) {
                syntax_error(parser, "expected parameter name");
                break;
            }

            if (param_count >= capacity) {
                capacity *= 2;
                params = (char**)realloc(params, sizeof(char*) * capacity);
            }

            char* param_name = (char*)malloc(64);
            strncpy(param_name, parser->current_token.text, 63);
            param_name[63] = '\0';
            params[param_count++] = param_name;
            advance(parser);

            if (match(parser, TOKEN_COMMA)) {
                advance(parser);
            } else {
                break;
            }
        }
    }

    expect(parser, TOKEN_RPAREN, "expected ')' after parameters");
    ASTNode* body = parse_block(parser);

    return ast_new_func_decl(name, params, param_count, body, line);
}

static ASTNode* parse_return(Parser* parser) {
    int line = parser->current_token.line;
    advance(parser); // consume 'return'

    ASTNode* expr = NULL;
    if (!match(parser, TOKEN_SEMICOLON)) {
        expr = parse_expression(parser);
    }

    expect(parser, TOKEN_SEMICOLON, "expected ';' after return statement");

    return ast_new_return(expr, line);
}

static ASTNode* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_LET)) {
        return parse_var_decl(parser);
    }
    if (match(parser, TOKEN_PRINT)) {
        return parse_print(parser);
    }
    if (match(parser, TOKEN_IF)) {
        return parse_if(parser);
    }
    if (match(parser, TOKEN_WHILE)) {
        return parse_while(parser);
    }
    if (match(parser, TOKEN_FUNCTION)) {
        return parse_func_decl(parser);
    }
    if (match(parser, TOKEN_RETURN)) {
        return parse_return(parser);
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        char name[64];
        strncpy(name, parser->current_token.text, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        int line = parser->current_token.line;
        advance(parser);

        if (match(parser, TOKEN_ASSIGN)) {
            advance(parser);
            ASTNode* expr = parse_expression(parser);
            expect(parser, TOKEN_SEMICOLON, "expected ';' after assignment");
            return ast_new_assign(name, expr, line);
        } else if (match(parser, TOKEN_LPAREN)) {
            advance(parser);
            int capacity = 4;
            int count = 0;
            ASTNode** args = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
            if (!match(parser, TOKEN_RPAREN)) {
                while (1) {
                    if (count >= capacity) {
                        capacity *= 2;
                        args = (ASTNode**)realloc(args, sizeof(ASTNode*) * capacity);
                    }
                    args[count++] = parse_expression(parser);
                    if (match(parser, TOKEN_COMMA)) {
                        advance(parser);
                    } else {
                        break;
                    }
                }
            }
            expect(parser, TOKEN_RPAREN, "expected ')' after arguments");
            expect(parser, TOKEN_SEMICOLON, "expected ';' after function call");
            return ast_new_call(name, args, count, line);
        } else {
            syntax_error(parser, "expected '=' or '(' after identifier");
            return NULL;
        }
    }

    if (match(parser, TOKEN_SEMICOLON)) {
        advance(parser);
        return NULL;
    }

    syntax_error(parser, "unexpected token at start of statement");
    advance(parser);
    return NULL;
}

ASTNode* parse_program(Parser* parser) {
    ASTNode* program = ast_new_program();

    while (!match(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            ast_block_add(program, stmt);
        }
        if (parser->has_error) {
            break;
        }
    }

    return program;
}