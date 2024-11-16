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
    char *keyword;
    TokenType type;
} Keyword;

Keyword keywords[] = {
    {"and", AND},
    {"class", CLASS},
    {"else", ELSE},
    {"false", FALSE},
    {"for", FOR},
    {"fun", FUN},
    {"if", IF},
    {"nil", NIL},
    {"or", OR},
    {"print", PRINT},
    {"return", RETURN},
    {"super", SUPER},
    {"this", THIS},
    {"true", TRUE},
    {"var", VAR},
    {"while", WHILE}
};

#define NUM_KEYWORDS (sizeof(keywords) / sizeof(Keyword))

typedef struct {
    TokenType type;
    char *lexeme;
    void *literal;
    int line;
    //int error;
    //int comment;
} Token;

struct Element{
	void* info;
	struct Element* next;
};

typedef struct Element t_Element;

typedef struct {
	t_Element* firstelement;
	int count;
} t_List;

// ---------------
// FUNCTIONS
// ---------------
char *read_file_contents(char *filename);
char advance(char* filecontent, int* position);
int isAtEnd(int position, int len);
TokenType checkKeyword(char *lexeme);
Token* scanToken(char* filecontent, int* line, int* position, int* error, char** msgError);
int decimals_to_show(char* str);
t_List* instruction_interpreter(char* file_contents);
char* tokenTypeToString(TokenType tokenType);
int match(char *filecontent, int* position, char expectedCharacter);
t_List *listCreate();
int listAdd(t_List* list, void* info);
void listIterate(t_List* list, void(*myFunction)(void*));
void listRemoveAll(t_List* list, void (*freeElement)(void*));
void printTokens(void* element);
void freeToken(void* token);

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

            t_List* Tokens = instruction_interpreter(file_contents);
            //listIterate(Tokens, &printTokens);
            listRemoveAll(Tokens, &freeToken);

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

TokenType checkKeyword(char *lexeme) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(lexeme, keywords[i].keyword) == 0) {
            return keywords[i].type;
        }
    }
    return IDENTIFIER;
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

Token* scanToken(char* filecontent, int* line, int* position, int* error, char** msgError) {

    Token* token = malloc(sizeof(Token));
    char character = advance(filecontent, position);

    token->lexeme = malloc(2);
    token->lexeme[0] = character;
    token->lexeme[1] = '\0';
    
    token->line = *line;

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
            while (filecontent[*position] != '\n' && filecontent[*position] != '\0') {
                character = advance(filecontent, position);
            }
            if(character == '\n'){
                (*line)++;
            }
            free(token->lexeme);
            free(token);
            token = scanToken(filecontent, line, position, error, msgError);
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
      case ' ': case '\r': case '\t': case '\n':{
            if(token->lexeme){free(token->lexeme);} 
            if(token){free(token);}
            if(character == '\n'){ (*line)++;}

            while (!isAtEnd(*position, strlen(filecontent)) &&  
                (filecontent[*position] == ' ' || filecontent[*position] == '\r' || 
                filecontent[*position] == '\t' || filecontent[*position] == '\n')
            ){
                if (filecontent[*position] == '\n') { (*line)++;}
                (*position)++;
            }
            token = scanToken(filecontent, line, position, error, msgError);
            break;
        }
      case '\0': {
            if(token->lexeme){free(token->lexeme);} 
            if(token){free(token);}
            token = NULL;
            break;
        }
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
                        *error = 1;// To do
                        
                        size_t length = snprintf(NULL, 0, "[line %d] Error: Unterminated string.", *line) + 1;
                        *msgError = malloc(length);
                        snprintf(*msgError, length, "[line %d] Error: Unterminated string.", *line);

                        free(token->lexeme); free(token); position++; line++;
                        
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
      case '0' ... '9': {
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
      case '_': case 'a' ... 'z': case 'A' ... 'Z':{
            int len = strlen(token->lexeme);
            token->type = IDENTIFIER;
            while(!isAtEnd(*position, strlen(filecontent)) && ( (filecontent[*position] >= 'a' && filecontent[*position] <= 'z') ||
                    (filecontent[*position] >= 'A' && filecontent[*position] <= 'Z') || (filecontent[*position] == '_') ||
                    (filecontent[*position] >= '0' && filecontent[*position] <= '9') ) ){
                        character = advance(filecontent, position);
                        token->lexeme = (char*) realloc(token->lexeme, len + 1 + 1);
                        token->lexeme[len] = character;
                        len++;
                        token->lexeme[len] = '\0';
                        token->type = checkKeyword(token->lexeme);
                    }
            break;
        }
      default: {
            *error = 1;// To do

            size_t length = snprintf(NULL, 0, "[line %d] Error: Unexpected character: %s", *line, token->lexeme) + 1;
            *msgError = malloc(length);
            snprintf(*msgError, length, "[line %d] Error: Unexpected character: %s", *line, token->lexeme);

            free(token->lexeme); free(token); position++; line++;
                        
            break;
      }
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
      case IDENTIFIER: return "IDENTIFIER";
      case AND: return "AND";
      case CLASS: return "CLASS";
      case ELSE: return "ELSE"; 
      case FALSE: return "FALSE"; 
      case FUN: return "FUN";
      case FOR: return "FOR";
      case IF: return "IF"; 
      case NIL: return "NIL";
      case OR: return "OR";
      case PRINT: return "PRINT";
      case RETURN: return "RETURN";
      case SUPER: return "SUPER";
      case THIS: return "THIS";
      case TRUE: return "TRUE";
      case VAR: return "VAR";
      case WHILE: return "WHILE";
      default: return "Unexpected character";
    }
}

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

t_List* instruction_interpreter(char* file_contents){

    t_List* Tokens = listCreate();

    int position = 0;
    int len = strlen(file_contents);
    int line = 1;

    int error = 0; // No errors yet
    char* errorMessage = NULL; // No error messages yet

    while (!isAtEnd(position, len)) {
        
        Token* Token = scanToken(file_contents, &line, &position, &error , &errorMessage);
        if(!Token){break;} // Token is NULL => I'm at the EOF

        if(error == 0){

            listAdd(Tokens, Token);
            if(!Token->literal){
                printf("%s %s null \n", tokenTypeToString(Token->type), Token->lexeme);
            }else{

                if(Token->type == STRING){
                    printf("%s %s %s\n", tokenTypeToString(Token->type), Token->lexeme, ((char*) (Token->literal)));
                }
                if(Token->type == NUMBER){
                    printf("%s %s %.*f\n", tokenTypeToString(Token->type), Token->lexeme, decimals_to_show(Token->lexeme) , *((double*) (Token->literal)) );
                }
            }
        }else{

            fprintf(stderr, "%s\n", errorMessage);
            free(errorMessage);
            error = 0; // Error Reset
        }
    }

    // Create EOF Token
    Token* EndOfFile = malloc(sizeof(Token));
    EndOfFile->line = line; // Asegurarse de asignar la línea final
    EndOfFile->type = EOF_TOKEN;
    // Insert EOF Token
    listAdd(Tokens, EndOfFile);
    printf("EOF  null\n");

    return Tokens;
}

t_List *listCreate() {
	t_List *list = malloc(sizeof(t_List));
	list->firstelement = NULL;
	list->count = 0;
	return list;
}

int listAdd(t_List* list, void* info) {
    // Element
    t_Element* element = malloc(sizeof(t_Element));
	element->info = info;
    element->next = NULL;

    t_Element** elementReference = &list->firstelement;
    // Index
    for (int i = 0; i < list->count; ++i) {
		elementReference = &(*elementReference)->next;
	}
    // Insert Element
    element->next = *elementReference;
	*elementReference = element;

	list->count++;
    return list->count - 1;
}

void listIterate(t_List* list, void(*myFunction)(void*)) {
	t_Element* element = list->firstelement;
	while ( element != NULL) {
		myFunction(element->info);
		element = element->next;
	}
}

void listRemoveAll(t_List* list, void (*freeElement)(void*)) {

    while (list->count > 0) {
        t_Element* element = list->firstelement;
        list->firstelement = element->next;
        freeElement(element->info);
        free(element);
	    list->count--;
	}
    
    free(list);
}

void printTokens(void* element){
    Token myToken =  *((Token*) element) ;
    printf("Lexeme: %s\n", myToken.lexeme);
}

void freeToken(void* token){
    Token* myToken = (Token*) token;
    //printf("Liberando: %s\n", myToken->lexeme);
    free(myToken->lexeme);
    free(token);
}