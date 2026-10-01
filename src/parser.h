/*
 * parser.h
 * --------
 * Provides declarations for the MiniLang parser.
 * The parser consumes tokens produced by the lexer and builds
 * an Abstract Syntax Tree (AST).
 */
#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current_token;
    int has_error;
} Parser;

// Initialize parser with an active lexer instance
void parser_init(Parser* parser, Lexer* lexer);

// Parse full program and return root AST node (AST_PROGRAM)
ASTNode* parse_program(Parser* parser);

#endif // PARSER_H