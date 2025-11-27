#ifndef GENERATOR_H
#define GENERATOR_H
#include "common.h"

void initGenerator();
int emit(OpCode op, int arg1, int arg2, int target);
void patch(int idx, int target);
Instruction* getCode();
int getCodeCount();
void freeGenerator();

// FUNCIONES DE STRINGS Y DEBUG
int addStringConstant(const char* str);
char* getStringConstant(int idx);
void printGeneratedCode(); 

#endif