#ifndef SYMBOLS_H
#define SYMBOLS_H

typedef enum { SYM_VAR_GLOBAL, SYM_VAR_LOCAL, SYM_FUNCION } SymbolKind;
// Definimos los 4 tipos soportados
typedef enum { TIPO_VOID, TIPO_ENTERO, TIPO_CHAR, TIPO_CADENA, TIPO_LISTA } DataType;

typedef struct Symbol {
    char name[50];
    SymbolKind kind;
    DataType type;
    int address; 
    int paramCount;
    struct Symbol* next;
} Symbol;

void initSymbolTable();
void pushScope(char* name);
void popScope();
void declareSymbol(char* name, SymbolKind kind, DataType type, int addr, int params);
Symbol* findSymbol(char* name);
int isGlobalScope();

#endif