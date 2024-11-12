#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ---------------
// STRUCTURES
// ---------------
typedef enum {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    EOF_TOKEN
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    void *literal;
    int line;
} Token;

// ---------------
// FUNCTIONS
// ---------------
char *read_file_contents(const char *filename);
Token* scanToken(char character, int line, int position);
void instruction_interpreter(const char* file_contents);
char* tokenTypeToString(TokenType tokenType);

int main(int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "tokenize") == 0) {

        char *file_contents = read_file_contents(argv[2]);

        if (strlen(file_contents) > 0) {
            instruction_interpreter(file_contents);
            exit(1);
        } 
        
        free(file_contents);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}

char *read_file_contents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(file_contents, 1, file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Error reading file contents\n");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    file_contents[file_size] = '\0';
    fclose(file);

    return file_contents;
}

Token* scanToken(char character, int line, int position) {
    Token* token = malloc(sizeof(Token));

    token->literal = NULL;

    token->lexeme = malloc(2);
    token->lexeme[0] = character;
    token->lexeme[1] = '\0';

    token->line = line;

    switch (character) {
      case '(': token->type = LEFT_PAREN; break;
      case ')': token->type = RIGHT_PAREN; break;
      case '{': token->type = LEFT_BRACE; break;
      case '}': token->type = RIGHT_BRACE; break;
      case ',': token->type = COMMA; break;
      case '.': token->type = DOT; break;
      case '-': token->type = MINUS; break;
      case '+': token->type = PLUS; break;
      case ';': token->type = SEMICOLON; break;
      case '*': token->type = STAR; break;
    }

    return token;
}

char* tokenTypeToString(TokenType tokenType) {
    switch (tokenType) {
      case LEFT_PAREN: return "LEFT_PAREN";
      case RIGHT_PAREN: return "RIGHT_PAREN";
      case LEFT_BRACE: return "LEFT_BRACE";
      case RIGHT_BRACE: return "RIGHT_BRACE";
      case COMMA: return "COMMA";
      case DOT: return "DOT";
      case MINUS: return "MINUS";
      case PLUS: return "PLUS";
      case SEMICOLON: return "SEMICOLON";
      case STAR: return "STAR";
      default: return "UNKNOWN";
    }
}

void instruction_interpreter(const char* file_contents){
    int position = 0;
    int len = strlen(file_contents + 1);
    int line = 0;

    Token* myToken;

    while (position <= len) {
        if( file_contents[position] == '\n'){
            line++;
        }else{
            myToken = scanToken(file_contents[position], line, position);
            char* myTokenStringType =  tokenTypeToString(myToken->type);
            fprintf(stderr, "%s  %s null\n", myTokenStringType, myToken->lexeme);
            free(myToken->lexeme);
        }
        position++;
    }
    fprintf(stderr,"EOF null\n");
}

