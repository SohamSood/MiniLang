#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->cursor = 0;
    lexer->line = 1;
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->cursor];
}

static char peek_next(Lexer* lexer) {
    if (lexer->source[lexer->cursor] == '\0') return '\0';
    return lexer->source[lexer->cursor + 1];
}

static char advance(Lexer* lexer) {
    char c = lexer->source[lexer->cursor];
    if (c != '\0') {
        lexer->cursor++;
        if (c == '\n') {
            lexer->line++;
        }
    }
    return c;
}

static void skip_whitespace_and_comments(Lexer* lexer) {
    while (1) {
        char c = peek(lexer);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lexer);
        } else if (c == '/' && peek_next(lexer) == '/') {
            // Single-line comment: skip until newline or EOF
            advance(lexer);
            advance(lexer);
            while (peek(lexer) != '\n' && peek(lexer) != '\0') {
                advance(lexer);
            }
        } else {
            break;
        }
    }
}

static Token make_token(TokenType type, const char* text, int line) {
    Token token;
    token.type = type;
    token.line = line;
    token.number_val = 0.0;
    strncpy(token.text, text, sizeof(token.text) - 1);
    token.text[sizeof(token.text) - 1] = '\0';
    return token;
}

static TokenType check_keyword(const char* text) {
    if (strcmp(text, "let") == 0) return TOKEN_LET;
    if (strcmp(text, "print") == 0) return TOKEN_PRINT;
    if (strcmp(text, "if") == 0) return TOKEN_IF;
    if (strcmp(text, "else") == 0) return TOKEN_ELSE;
    if (strcmp(text, "while") == 0) return TOKEN_WHILE;
    if (strcmp(text, "function") == 0) return TOKEN_FUNCTION;
    if (strcmp(text, "return") == 0) return TOKEN_RETURN;
    return TOKEN_IDENTIFIER;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace_and_comments(lexer);

    int start_line = lexer->line;
    char c = peek(lexer);

    if (c == '\0') {
        return make_token(TOKEN_EOF, "EOF", start_line);
    }

    // Number literals (integers or decimals)
    if (isdigit(c)) {
        int start = lexer->cursor;
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
        if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
            advance(lexer); // consume '.'
            while (isdigit(peek(lexer))) {
                advance(lexer);
            }
        }
        int len = lexer->cursor - start;
        char num_str[64];
        if (len >= (int)sizeof(num_str)) len = sizeof(num_str) - 1;
        strncpy(num_str, lexer->source + start, len);
        num_str[len] = '\0';

        Token token = make_token(TOKEN_NUMBER, num_str, start_line);
        token.number_val = atof(num_str);
        return token;
    }

    // Identifiers and Keywords
    if (isalpha(c) || c == '_') {
        int start = lexer->cursor;
        while (isalnum(peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }
        int len = lexer->cursor - start;
        char id_str[64];
        if (len >= (int)sizeof(id_str)) len = sizeof(id_str) - 1;
        strncpy(id_str, lexer->source + start, len);
        id_str[len] = '\0';

        TokenType type = check_keyword(id_str);
        return make_token(type, id_str, start_line);
    }

    // Two-character operators
    if (c == '=' && peek_next(lexer) == '=') {
        advance(lexer); advance(lexer);
        return make_token(TOKEN_EQUAL, "==", start_line);
    }
    if (c == '!' && peek_next(lexer) == '=') {
        advance(lexer); advance(lexer);
        return make_token(TOKEN_NOT_EQUAL, "!=", start_line);
    }
    if (c == '<' && peek_next(lexer) == '=') {
        advance(lexer); advance(lexer);
        return make_token(TOKEN_LESS_EQUAL, "<=", start_line);
    }
    if (c == '>' && peek_next(lexer) == '=') {
        advance(lexer); advance(lexer);
        return make_token(TOKEN_GREATER_EQUAL, ">=", start_line);
    }

    // Single-character operators and delimiters
    advance(lexer);
    switch (c) {
        case '+': return make_token(TOKEN_PLUS, "+", start_line);
        case '-': return make_token(TOKEN_MINUS, "-", start_line);
        case '*': return make_token(TOKEN_STAR, "*", start_line);
        case '/': return make_token(TOKEN_SLASH, "/", start_line);
        case '=': return make_token(TOKEN_ASSIGN, "=", start_line);
        case '<': return make_token(TOKEN_LESS, "<", start_line);
        case '>': return make_token(TOKEN_GREATER, ">", start_line);
        case '(': return make_token(TOKEN_LPAREN, "(", start_line);
        case ')': return make_token(TOKEN_RPAREN, ")", start_line);
        case '{': return make_token(TOKEN_LBRACE, "{", start_line);
        case '}': return make_token(TOKEN_RBRACE, "}", start_line);
        case ',': return make_token(TOKEN_COMMA, ",", start_line);
        case ';': return make_token(TOKEN_SEMICOLON, ";", start_line);
        default: {
            char err_str[2] = { c, '\0' };
            return make_token(TOKEN_ERROR, err_str, start_line);
        }
    }
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_LET: return "let";
        case TOKEN_PRINT: return "print";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_FUNCTION: return "function";
        case TOKEN_RETURN: return "return";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_EQUAL: return "==";
        case TOKEN_NOT_EQUAL: return "!=";
        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_COMMA: return ",";
        case TOKEN_SEMICOLON: return ";";
        default: return "UNKNOWN";
    }
}
