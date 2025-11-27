#include "symbols.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Scope {
    Symbol* symbols;
    struct Scope* parent;
    char name[50];
} Scope;

Scope* currentScope = NULL;
int indentLevel = 0;

void printIndent() {
    for(int i=0; i<indentLevel; i++) printf("  | ");
}

void initSymbolTable() { currentScope = NULL; indentLevel = 0; }

void pushScope(char* name) {
    Scope* s = malloc(sizeof(Scope));
    s->symbols = NULL; 
    s->parent = currentScope; 
    strcpy(s->name, name);
    currentScope = s;
    
    printIndent();
    printf("--> ABRIENDO SCOPE: '%s'\n", name);
    indentLevel++;
}

void popScope() {
    if(currentScope) {
        indentLevel--;
        printIndent();
        printf("<-- CERRANDO SCOPE: '%s'\n", currentScope->name);
        currentScope = currentScope->parent;
    }
}

// ESTA FUNCION CAUSABA EL ERROR (Faltaba 'DataType type')
void declareSymbol(char* name, SymbolKind kind, DataType type, int addr, int params) {
    Symbol* s = malloc(sizeof(Symbol));
    strcpy(s->name, name); 
    s->kind = kind; 
    s->type = type; // Asignar el tipo
    s->address = addr; 
    s->paramCount = params;
    
    s->next = currentScope->symbols; 
    currentScope->symbols = s;
    
    printIndent();
    char* kindStr = (kind == SYM_VAR_GLOBAL) ? "GLOBAL" : (kind == SYM_VAR_LOCAL) ? "LOCAL" : "FUNC";
    // Mapeo simple para log visual
    char* typeStr = "unknown";
    if (type == TIPO_ENTERO) typeStr = "int";
    else if (type == TIPO_CHAR) typeStr = "char";
    else if (type == TIPO_CADENA) typeStr = "string";
    else if (type == TIPO_LISTA) typeStr = "list";
    else if (type == TIPO_VOID) typeStr = "void";
    
    printf("    + DECLARADO: %s (%s : %s) @ Dir: %d\n", name, kindStr, typeStr, addr);
}

Symbol* findSymbol(char* name) {
    Scope* s = currentScope;
    while(s) {
        Symbol* sym = s->symbols;
        while(sym) { if(!strcmp(sym->name, name)) return sym; sym = sym->next; }
        s = s->parent;
    }
    return NULL;
}

int isGlobalScope() { 
    if (currentScope == NULL) return 1; 
    return currentScope->parent == NULL; 
}