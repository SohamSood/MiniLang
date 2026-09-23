/*
 * lexer.c
 * -------
 * Performs lexical analysis for MiniLang.
 * Converts source code into a sequence of tokens such as
 * numbers, identifiers, operators, and keywords.
 */
// Converts the current sequence of characters into a token.

#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->cursor = 0;
    lexer->line = 1;
}

Token lexer_next_token(Lexer* lexer) {

    while (isspace(lexer->source[lexer->cursor]))
        lexer->cursor++;

    if (lexer->source[lexer->cursor] == '\0') {
        Token t = {TOKEN_EOF, "EOF", 0, lexer->line};
        return t;
    }

    if (isdigit(lexer->source[lexer->cursor])) {

        int start = lexer->cursor;

        while (isdigit(lexer->source[lexer->cursor]))
            lexer->cursor++;

        int len = lexer->cursor - start;

        Token t;
        t.type = TOKEN_NUMBER;
        t.line = lexer->line;

        for (int i = 0; i < len; i++)
            t.text[i] = lexer->source[start + i];

        t.text[len] = '\0';
        t.number_val = atof(t.text);

        return t;
    }

    lexer->cursor++;

    Token t = {TOKEN_ERROR, "ERROR", 0, lexer->line};
    return t;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_EOF: return "EOF";
        default: return "ERROR";
    }
}

#Recognizes number tokens and detects the end of the source.