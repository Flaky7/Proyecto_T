#ifndef LEXER_H
#define LEXER_H
#include "common.h"

void initLexer(const char* source);
Token getNextToken();
Token peekToken();
void freeLexer();

#endif