#ifndef LEXER_H //header guard (ifnotdefined)
#define LEXER_H

/*
 * lexer.h
 * -------
 * Token definitions and lexer function declarations.
 */

typedef enum { //constraints
    // End of file & error
    TOKEN_EOF,
    TOKEN_ERROR,

    // Literals & Identifiers
    TOKEN_NUMBER, //123
    TOKEN_IDENTIFIER, //abc

    // Keywords
    TOKEN_LET, //variable declaration
    TOKEN_PRINT, //print statement
    TOKEN_IF, //if conditiom
    TOKEN_ELSE, //else condition
    TOKEN_WHILE, //while loop
    TOKEN_FUNCTION, //function declaration
    TOKEN_RETURN, //return statement in fxn

    // Operators
    TOKEN_PLUS,          // +
    TOKEN_MINUS,         // -
    TOKEN_STAR,          // *
    TOKEN_SLASH,         // /
    TOKEN_ASSIGN,        // =
    TOKEN_EQUAL,         // ==
    TOKEN_NOT_EQUAL,     // !=
    TOKEN_LESS,          // <
    TOKEN_LESS_EQUAL,    // <=
    TOKEN_GREATER,       // >
    TOKEN_GREATER_EQUAL, // >=

    // Delimiters
    TOKEN_LPAREN,        // (
    TOKEN_RPAREN,        // )
    TOKEN_LBRACE,        // {
    TOKEN_RBRACE,        // }
    TOKEN_COMMA,         // ,
    TOKEN_SEMICOLON      // ;
} TokenType; //creates enumm nd call its type TokenType

typedef struct {
    TokenType type; //abc or 123 , number or identifier
    char text[64];       // stores the values
    double number_val;   // Parsed numeric value (if TOKEN_NUMBER)
    int line;            // Source line number for error reporting
} Token;

typedef struct {
    const char* source; // const -> doesntmodify og string and has the current source code being lexed
    int cursor;          // pointer to current source no 
    int line;            // current line number (1-based)
} Lexer;

// Initialize the lexer with source code
void lexer_init(Lexer* lexer, const char* source);

// Get the next token from the source code
Token lexer_next_token(Lexer* lexer);

// Helper to convert token type enum to a readable string
const char* token_type_to_string(TokenType type);

#endif // LEXER_H
