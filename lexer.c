#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Token* tokens = NULL;
int tokenCount = 0;
int currentIdx = 0;

void addToken(TokenType type, const char* val, int line) {
    tokens = realloc(tokens, sizeof(Token) * (tokenCount + 1));
    tokens[tokenCount].type = type;
    strcpy(tokens[tokenCount].value, val);
    tokens[tokenCount].line = line;
    tokenCount++;
}

void initLexer(const char* src) {
    tokens = NULL; tokenCount = 0; currentIdx = 0;
    int i = 0, line = 1;
    
    while (src[i] != '\0') {
        if (isspace(src[i])) { if (src[i] == '\n') line++; i++; continue; }
        
        // Simbolos simples
        if (src[i] == ';') { addToken(TOK_PUNTO_COMA, ";", line); i++; continue; }
        if (src[i] == '=') { addToken(TOK_ASIGNAR, "=", line); i++; continue; }
        if (src[i] == '+') { addToken(TOK_SUMA, "+", line); i++; continue; }
        if (src[i] == '-') { addToken(TOK_RESTA, "-", line); i++; continue; }
        if (src[i] == '<') { addToken(TOK_MENOR, "<", line); i++; continue; }
        if (src[i] == '{') { addToken(TOK_LLAVE_IZQ, "{", line); i++; continue; }
        if (src[i] == '}') { addToken(TOK_LLAVE_DER, "}", line); i++; continue; }
        if (src[i] == '(') { addToken(TOK_PAR_IZQ, "(", line); i++; continue; }
        if (src[i] == ')') { addToken(TOK_PAR_DER, ")", line); i++; continue; }
        if (src[i] == '[') { addToken(TOK_COR_IZQ, "[", line); i++; continue; }
        if (src[i] == ']') { addToken(TOK_COR_DER, "]", line); i++; continue; }
        if (src[i] == ',') { addToken(TOK_COMA, ",", line); i++; continue; }

        // Cadenas "..."
        if (src[i] == '"') {
            i++; // saltar comilla inicio
            char buf[256]; int j=0;
            while(src[i] != '"' && src[i] != '\0') buf[j++] = src[i++];
            buf[j] = '\0';
            if(src[i] == '"') i++; // saltar comilla fin
            addToken(TOK_LIT_CADENA, buf, line);
            continue;
        }

        // Caracteres 'c'
        if (src[i] == '\'') {
            i++; char c = src[i++]; 
            if(src[i] == '\'') i++;
            char buf[2] = {c, '\0'};
            addToken(TOK_LIT_CHAR, buf, line);
            continue;
        }

        // Palabras clave y IDs
        if (isalpha(src[i])) {
            char buf[100]; int j=0;
            while(isalnum(src[i])) buf[j++] = src[i++]; buf[j] = '\0';
            
            if (!strcmp(buf, "programa")) addToken(TOK_INICIO, buf, line);
            else if (!strcmp(buf, "fin")) addToken(TOK_FIN, buf, line);
            else if (!strcmp(buf, "funcion")) addToken(TOK_FUNC, buf, line);
            else if (!strcmp(buf, "retornar")) addToken(TOK_RETORNAR, buf, line);
            else if (!strcmp(buf, "si")) addToken(TOK_SI, buf, line);
            else if (!strcmp(buf, "entonces")) addToken(TOK_ENTONCES, buf, line);
            else if (!strcmp(buf, "mientras")) addToken(TOK_MIENTRAS, buf, line);
            else if (!strcmp(buf, "hacer")) addToken(TOK_HACER, buf, line);
            else if (!strcmp(buf, "escribir")) addToken(TOK_ESCRIBIR, buf, line);
            else if (!strcmp(buf, "para")) addToken(TOK_PARA, buf, line);
            
            // TIPOS
            else if (!strcmp(buf, "entero")) addToken(TOK_ENTERO, buf, line);
            else if (!strcmp(buf, "char")) addToken(TOK_CHAR, buf, line);
            else if (!strcmp(buf, "cadena")) addToken(TOK_CADENA, buf, line);
            else if (!strcmp(buf, "lista")) addToken(TOK_LISTA, buf, line);
            
            else addToken(TOK_ID, buf, line);
            continue;
        }

        if (isdigit(src[i])) {
            char buf[100]; int j=0;
            while(isdigit(src[i])) buf[j++] = src[i++]; buf[j] = '\0';
            addToken(TOK_NUMERO, buf, line);
            continue;
        }
        i++;
    }
    addToken(TOK_EOF, "EOF", line);
}

Token getNextToken() { 
    if (currentIdx < tokenCount) return tokens[currentIdx++]; 
    return tokens[tokenCount-1]; // Devolver EOF si nos pasamos
}

Token peekToken() { 
    if (currentIdx < tokenCount) return tokens[currentIdx];
    return tokens[tokenCount-1];
}

void freeLexer() { 
    if(tokens) free(tokens); 
}