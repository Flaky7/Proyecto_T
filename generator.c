#include "generator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Instruction* code;
int codeCount = 0;
int codeCapacity = 200;

// TABLA DE STRINGS LITERALES
char stringTable[100][256];
int stringCount = 0;

// ARRAY DE NOMBRES PARA DEBUG
const char* OpNames[] = {
    "LITERAL", "CARGAR_VAR", "GUARDAR_VAR",
    "SUMAR", "RESTAR", "MENOR",
    "ESC_INT", "ESC_CHAR", "ESC_STR", "ESC_LIST", // Nombres cortos para imprimir
    "JMP_FALSE", "JMP",
    "CALL", "RET", "HALT",
    "NEW_LIST", "LIST_PUSH", "LIST_GET"
};

void initGenerator() {
    code = malloc(sizeof(Instruction) * codeCapacity);
    codeCount = 0;
    stringCount = 0;
}

int emit(OpCode op, int arg1, int arg2, int target) {
    if (codeCount >= codeCapacity) {
        codeCapacity *= 2; 
        code = realloc(code, sizeof(Instruction)*codeCapacity);
    }
    code[codeCount] = (Instruction){codeCount, op, arg1, arg2, target};
    return codeCount++;
}

void patch(int idx, int target) { 
    code[idx].target = target; 
}

// Implementacion de Strings
int addStringConstant(const char* str) {
    strcpy(stringTable[stringCount], str);
    return stringCount++;
}

char* getStringConstant(int idx) {
    return stringTable[idx];
}

void printGeneratedCode() {
    printf("\n=== CODIGO INTERMEDIO GENERADO (TUPLAS) ===\n");
    printf("ID\tOPCODE\t\tARG1\tARG2\tTARGET\n");
    printf("----------------------------------------------------\n");
    for(int i=0; i<codeCount; i++) {
        // Validación de seguridad para no desbordar el array de nombres
        const char* name = (code[i].op <= OP_LISTA_GET) ? OpNames[code[i].op] : "UNKNOWN";
        
        printf("%03d\t%-12s\t%d\t%d\t%d\n", 
               code[i].id, 
               name,
               code[i].arg1, 
               code[i].arg2, 
               code[i].target);
    }
    printf("====================================================\n");
}

Instruction* getCode() { return code; }
int getCodeCount() { return codeCount; }
void freeGenerator() { free(code); }