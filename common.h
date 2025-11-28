#ifndef COMMON_H
#define COMMON_H

typedef enum {
    TOK_INICIO, TOK_FIN, TOK_FUNC, TOK_RETORNAR,
    // Tipos de datos
    TOK_ENTERO, TOK_CHAR, TOK_CADENA, TOK_LISTA,
    // Valores
    TOK_NUMERO, TOK_LIT_CHAR, TOK_LIT_CADENA, TOK_ID, 
    // Comandos
    TOK_LEER, TOK_ESCRIBIR, 
    TOK_SI, TOK_ENTONCES, TOK_MIENTRAS, TOK_HACER, TOK_PARA,
    // Operadores
    TOK_ASIGNAR, TOK_SUMA, TOK_RESTA, TOK_MENOR, 
    // Simbolos
    TOK_LLAVE_IZQ, TOK_LLAVE_DER, TOK_PAR_IZQ, TOK_PAR_DER, 
    TOK_COR_IZQ, TOK_COR_DER, TOK_COMA, TOK_PUNTO_COMA,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[256];//cadenas largas
    int line;
} Token;

typedef enum {
    OP_LITERAL, OP_CARGAR_VAR, OP_GUARDAR_VAR,
    OP_SUMAR, OP_RESTAR, OP_MENOR,
    // Impresion tipada
    OP_ESCRIBIR_INT, OP_ESCRIBIR_CHAR, OP_ESCRIBIR_STR, OP_ESCRIBIR_LIST,
    OP_SALTAR_FALSO, OP_SALTAR,
    OP_LLAMAR, OP_RETORNAR, OP_HALT,
    // Operaciones de Lista
    OP_NUEVA_LISTA, OP_LISTA_PUSH, OP_LISTA_GET
} OpCode;

typedef struct {
    int id;
    OpCode op;
    int arg1; 
    int arg2; 
    int target; 
} Instruction;

#endif