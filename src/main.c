#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    int error;
    bool comment;
} Token;

// ---------------
// FUNCTIONS
// ---------------
char *read_file_contents(char *filename);
char advance(char* filecontent, int* position);
int isAtEnd(int position, int len);
Token* scanToken(char* filecontent, int line, int* position);
int decimals_to_show(char* str);
void instruction_interpreter(char* file_contents);
char* tokenTypeToString(TokenType tokenType);
int match(char *filecontent, int* position, char expectedCharacter);

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
        }else{
            printf("EOF  null\n");
            exit(0);
        }
        free(file_contents);

    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}

char *read_file_contents(char *filename) {
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

char advance(char* filecontent, int* position) {
    char myChar = filecontent[*position];
    (*position)++;
    return myChar;
}

int isAtEnd(int position, int len){
 return position >= len;
}

int match(char *filecontent, int* position, char expectedCharacter){
    if(isAtEnd(*position, strlen(filecontent)) || filecontent[*position] != expectedCharacter){
        return 0;
    }
    (*position)++;
    return 1;
}

Token* scanToken(char* filecontent, int line, int* position) {
    Token* token = malloc(sizeof(Token));
    char character = advance(filecontent, position);

    token->literal = NULL;
    token->comment = 0;
    token->error = 0;

    token->lexeme = malloc(2);
    token->lexeme[0] = character;
    token->lexeme[1] = '\0';
    
    token->line = line;
    token->literal = NULL;

    switch (character) {
       // Single-character tokens.
      case '(': token->type = LEFT_PAREN; break;
      case ')': token->type = RIGHT_PAREN; break;
      case '{': token->type = LEFT_BRACE; break;
      case '}': token->type = RIGHT_BRACE; break;
      case ',': token->type = COMMA; break;
      case '.': token->type = DOT; break;
      case '-': token->type = MINUS; break;
      case '+': token->type = PLUS; break;
      case ';': token->type = SEMICOLON; break;
      case '/':
        if (match(filecontent, position, '/')) {
            token->comment = 1;
            while (filecontent[*position] != '\n' && filecontent[*position] != '\0') {
                character = advance(filecontent, position);
            }
        } else {
            token->type = SLASH;
        }
      break;
      case '*': token->type = STAR; break;
      // One or two character tokens.
      case '!':
        token->type = match(filecontent, position, '=') ? BANG_EQUAL : BANG; 
        if (token->type == BANG_EQUAL) token->lexeme[1] = '=', token->lexeme[2] = '\0';
        break;
      case '=':
        token->type = match(filecontent, position, '=') ? EQUAL_EQUAL : EQUAL;
        if (token->type == EQUAL_EQUAL) token->lexeme[1] = '=', token->lexeme[2] = '\0';
        break;
      case '<':
        token->type = match(filecontent, position, '=') ? LESS_EQUAL : LESS;
        if (token->type == LESS_EQUAL) token->lexeme[1] = '=', token->lexeme[2] = '\0';
        break;
      case '>':
        token->type = match(filecontent, position, '=') ? GREATER_EQUAL : GREATER;
        if (token->type == GREATER_EQUAL) token->lexeme[1] = '=', token->lexeme[2] = '\0';
        break;
      case ' ': case '\r': case '\t': token->comment = 1; break;
      case '\n': (*position)++; break;
      case '"': {
                    token->type = STRING;
                    int len = strlen(token->lexeme);
                    
                    while(!isAtEnd(*position, strlen(filecontent)) && filecontent[*position] != '"'){
                        character = advance(filecontent, position);
                        token->lexeme = (char*)realloc(token->lexeme, len + 1 + 1);
                        token->lexeme[len] = character;
                        len++;
                        token->lexeme[len] = '\0';
                    }

                    if(filecontent[*position] == '\0'){
                        token->error = 2;
                        break;
                    }

                    if(filecontent[*position] == '"'){
                        character = advance(filecontent, position);
                        token->lexeme = (char*)realloc(token->lexeme, len + 1);
                        token->lexeme[len] = character;
                        len++;
                        token->lexeme[len] = '\0';
                    }

                    token->literal = malloc( (sizeof(char) * len) - (sizeof(char) * 2) );
                    for(int i=0; i <= (len - 2); i++ ){
                        if(i == len - 2){
                            ((char*) (token->literal)) [i] = '\0';
                            continue;
                        }
                        ((char*) (token->literal)) [i] = token->lexeme[i+1];
                    }
                    break;
                }
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            token->type = NUMBER;
            int len = strlen(token->lexeme);

            int isDecimal = 0;

            while (!isAtEnd(*position, strlen(filecontent)) && ( (filecontent[*position] >= '0' && filecontent[*position] <= '9') ||  filecontent[*position] == '.' ) ) {
                character = advance(filecontent, position);
                if(character == '.'){
                    isDecimal = 1;
                }
                token->lexeme = (char*)realloc(token->lexeme, len + 1 + 1);
                token->lexeme[len] = character;
                len++;
                token->lexeme[len] = '\0';
            }
            
            double myNumber = strtod(token->lexeme, NULL);
            token->literal = malloc(sizeof(double));
            *((double*) (token->literal)) = myNumber;
            
            break;
      }
      default: token->error = 1; break;
    }
    return token;
}

char* tokenTypeToString(TokenType tokenType) {
    switch (tokenType) {
      // Single-character tokens.
      case LEFT_PAREN: return "LEFT_PAREN";
      case RIGHT_PAREN: return "RIGHT_PAREN";
      case LEFT_BRACE: return "LEFT_BRACE";
      case RIGHT_BRACE: return "RIGHT_BRACE";
      case COMMA: return "COMMA";
      case DOT: return "DOT";
      case MINUS: return "MINUS";
      case PLUS: return "PLUS";
      case SEMICOLON: return "SEMICOLON";
      case SLASH: return "SLASH";
      case STAR: return "STAR";
      // One or two character tokens.
      case BANG: return "BANG";
      case BANG_EQUAL: return "BANG_EQUAL" ;
      case EQUAL: return "EQUAL";
      case EQUAL_EQUAL: return "EQUAL_EQUAL";
      case GREATER:  return "GREATER";
      case GREATER_EQUAL: return "GREATER_EQUAL";
      case LESS: return "LESS";
      case LESS_EQUAL: return "LESS_EQUAL";
      case STRING: return "STRING";
      case NUMBER: return "NUMBER";
      default: return "Unexpected character";
    }
}
// 11111.0001221200
int decimals_to_show(char* str){
    int len = strlen(str);
    int response = 0;
    int countDigit = 0;
    int isCero = 0;

    for(int i=0; i<len; i++){
        if(str[i] == '0' && countDigit){
            isCero++;
        }
        if(str[i] != '0' && countDigit){
            response = response + 1 + isCero;
            isCero = 0;
        }
        if(str[i] == '.'){
            countDigit = 1;
        }
    }
    if(!countDigit || isCero){
        response = 1;
    }
    
    return response;
}

void instruction_interpreter(char* file_contents){
    int position = 0;
    int len = strlen(file_contents);
    int line = 1;

    int error = 0;
    Token* myToken;

    Token** validTokens = NULL;
    int tokenCount = 0;

    while (!isAtEnd(position, len)) {
        if( file_contents[position] == '\n'){
            line++;
            position++;
        }else{
            myToken = scanToken(file_contents, line, &position);
            if(myToken->comment){
                continue;
            }
            if(!(myToken->error)){
                validTokens = realloc(validTokens, sizeof(Token*) * (tokenCount + 1));
                validTokens[tokenCount] = myToken;
                tokenCount++;
            }else{
                error = 1;
                if(myToken->error == 1){
                    fprintf(stderr, "[line %d] Error: Unexpected character: %s\n", line, myToken->lexeme);
                }
                if(myToken->error == 2){
                    fprintf(stderr, "[line %d] Error: Unterminated string.\n", line);
                }
                free(myToken->lexeme);
            }
        }
    }
    for (int i = 0; i < tokenCount; i++) {
        char* myTokenStringType = tokenTypeToString(validTokens[i]->type);
        if(!validTokens[i]->literal){
            printf("%s %s null\n", myTokenStringType, validTokens[i]->lexeme);
        }else{
            if(validTokens[i]->type == STRING){
                printf("%s %s %s\n", myTokenStringType, validTokens[i]->lexeme, ((char*) (validTokens[i]->literal)));
            }
            if(validTokens[i]->type == NUMBER){
                printf("%s %s %.*f\n", myTokenStringType, validTokens[i]->lexeme, decimals_to_show(validTokens[i]->lexeme) , *((double*) (validTokens[i]->literal)) );
            }
        }
        free(validTokens[i]->lexeme);
        free(validTokens[i]->literal);
        free(validTokens[i]);
    }
    printf("EOF  null\n");
    if(!(error)){
        exit(0);
    }else{
        exit(65);
    }
}

