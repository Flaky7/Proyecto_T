#include "parser.h"
#include "lexer.h"
#include "symbols.h"
#include "generator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Contadores de memoria
int globalAddressCounter = 0;
int localOffsetCounter = 0;

// Prototipos internos
void block();
void statement();
void expression();
void parseListLiteral();

// Función auxiliar para validar y consumir tokens
void match(TokenType t) {
    if(peekToken().type == t) {
        getNextToken();
    } else {
        printf("Error Sintactico: Esperaba token tipo %d, pero obtuve '%s' en linea %d\n", 
               t, peekToken().value, peekToken().line);
        exit(1);
    }
}

// ---------------------------------------------------------
// PARSEO DE LISTAS LITERALES: [1, 2, 3]
// ---------------------------------------------------------
void parseListLiteral() {
    match(TOK_COR_IZQ); // [
    
    // 1. Crear una nueva lista vacía en el Heap de la VM
    emit(OP_NUEVA_LISTA, 0, 0, 0); 
    
    // Si no está vacía "[]", parseamos los elementos
    if (peekToken().type != TOK_COR_DER) {
        do {
            if (peekToken().type == TOK_COMA) match(TOK_COMA);
            
            expression(); // Calcula el valor del elemento (se pone en tope de pila)
            
            // OP_LISTA_PUSH espera: 
            // Tope: Valor a insertar
            // Tope-1: ID de la lista (creado por OP_NUEVA_LISTA)
            emit(OP_LISTA_PUSH, 0, 0, 0); 
            
        } while (peekToken().type == TOK_COMA);
    }
    match(TOK_COR_DER); // ]
}

// ---------------------------------------------------------
// EXPRESIONES (Matemáticas, Variables, Literales)
// ---------------------------------------------------------
void expression() {
    Token t = peekToken();
    
    // 1. Literales Numéricos
    if (t.type == TOK_NUMERO) {
        emit(OP_LITERAL, atoi(getNextToken().value), 0, 0);
    } 
    // 2. Literales Char ('A')
    else if (t.type == TOK_LIT_CHAR) {
        // Guardamos el char como su valor ASCII (int)
        char c = getNextToken().value[0];
        emit(OP_LITERAL, (int)c, 0, 0);
    }
    // 3. Literales Cadena ("Hola")
    else if (t.type == TOK_LIT_CADENA) {
        // Guardamos el string en la tabla estática y emitimos su índice
        int strId = addStringConstant(getNextToken().value);
        emit(OP_LITERAL, strId, 0, 0); 
    }
    // 4. Literales Lista ([...])
    else if (t.type == TOK_COR_IZQ) {
        parseListLiteral();
    }
    // 5. Identificadores (Variables o Funciones)
    else if (t.type == TOK_ID) {
        Token idToken = getNextToken();
        
        // A) ¿Es una LLAMADA A FUNCION? -> nombre(...)
        if (peekToken().type == TOK_PAR_IZQ) {
            match(TOK_PAR_IZQ);
            Symbol* s = findSymbol(idToken.value);
            if (!s || s->kind != SYM_FUNCION) {
                printf("Error: Funcion '%s' no definida o no es funcion.\n", idToken.value); exit(1);
            }
            
            int args = 0;
            if (peekToken().type != TOK_PAR_DER) {
                expression(); args++;
                while(peekToken().type == TOK_COMA) {
                    match(TOK_COMA); expression(); args++;
                }
            }
            match(TOK_PAR_DER);
            
            if (args != s->paramCount) {
                printf("Error: '%s' espera %d argumentos, recibio %d\n", idToken.value, s->paramCount, args); exit(1);
            }
            emit(OP_LLAMAR, s->address, args, 0);
        } 
        // B) Es una VARIABLE -> nombre
        else {
            Symbol* s = findSymbol(idToken.value);
            if (!s) { printf("Error: Variable '%s' no definida\n", idToken.value); exit(1); }
            
            emit(OP_CARGAR_VAR, s->address, (s->kind == SYM_VAR_LOCAL), 0);
            
            // C) ¿Es un ACCESO A LISTA? -> nombre[indice]
            if (peekToken().type == TOK_COR_IZQ) {
                match(TOK_COR_IZQ); // [
                expression();       // El índice (ej: 0)
                match(TOK_COR_DER); // ]
                
                // Stack antes: [ID_LISTA, INDICE]
                // Stack despues: [ELEMENTO]
                emit(OP_LISTA_GET, 0, 0, 0); 
            }
        }
    }
    // 6. Paréntesis agrupadores
    else if (t.type == TOK_PAR_IZQ) {
        match(TOK_PAR_IZQ); expression(); match(TOK_PAR_DER);
    }
    
    // --- OPERACIONES BINARIAS ---
    // (Implementación simple sin precedencia de operadores completa para brevedad)
    if (peekToken().type == TOK_SUMA) { getNextToken(); expression(); emit(OP_SUMAR, 0, 0, 0); }
    else if (peekToken().type == TOK_RESTA) { getNextToken(); expression(); emit(OP_RESTAR, 0, 0, 0); }
    else if (peekToken().type == TOK_MENOR) { getNextToken(); expression(); emit(OP_MENOR, 0, 0, 0); }
}

// ---------------------------------------------------------
// SENTENCIAS (Declaraciones, Logic, Print)
// ---------------------------------------------------------
void statement() {
    Token t = peekToken();
    
    // 1. DETECCION DE DECLARACION DE VARIABLES (Tipadas)
    DataType typeToDeclare = TIPO_VOID;
    if (t.type == TOK_ENTERO) typeToDeclare = TIPO_ENTERO;
    else if (t.type == TOK_CHAR) typeToDeclare = TIPO_CHAR;
    else if (t.type == TOK_CADENA) typeToDeclare = TIPO_CADENA;
    else if (t.type == TOK_LISTA) typeToDeclare = TIPO_LISTA;

    if (typeToDeclare != TIPO_VOID) {
        getNextToken(); // Consumir el tipo (ej: "entero")
        Token id = getNextToken(); // Nombre variable
        match(TOK_ASIGNAR); 
        expression(); // Valor inicial
        
        if (isGlobalScope()) {
            declareSymbol(id.value, SYM_VAR_GLOBAL, typeToDeclare, globalAddressCounter, 0);
            emit(OP_GUARDAR_VAR, globalAddressCounter++, 0, 0);
        } else {
            declareSymbol(id.value, SYM_VAR_LOCAL, typeToDeclare, localOffsetCounter, 0);
            emit(OP_GUARDAR_VAR, localOffsetCounter++, 1, 0);
        }
    }
    
    // 2. ASIGNACION A VARIABLE EXISTENTE (x = 5)
    else if (t.type == TOK_ID) {
        Token id = getNextToken();
        
        // Verificamos si es var[i] = val (Asignacion a lista) o var = val
        // Por simplicidad para este demo, solo soportamos var = val
        match(TOK_ASIGNAR); 
        expression();
        
        Symbol* s = findSymbol(id.value);
        if (!s) { printf("Error: Variable '%s' no declarada\n", id.value); exit(1); }
        emit(OP_GUARDAR_VAR, s->address, (s->kind == SYM_VAR_LOCAL), 0);
    }
    
    // 3. COMANDO ESCRIBIR (Inteligente)
    else if (t.type == TOK_ESCRIBIR) {
        getNextToken(); // Consumir 'escribir'
        
        // HEURISTICA: Miramos qué viene para decidir el OpCode de impresión correcto
        Token next = peekToken();
        OpCode printOp = OP_ESCRIBIR_INT; // Default
        
        if (next.type == TOK_LIT_CADENA) printOp = OP_ESCRIBIR_STR;
        else if (next.type == TOK_LIT_CHAR) printOp = OP_ESCRIBIR_CHAR;
        else if (next.type == TOK_ID) {
            Symbol* s = findSymbol(next.value);
            if(s) {
                if(s->type == TIPO_CADENA) printOp = OP_ESCRIBIR_STR;
                else if(s->type == TIPO_CHAR) printOp = OP_ESCRIBIR_CHAR;
                else if(s->type == TIPO_LISTA) printOp = OP_ESCRIBIR_LIST;
            }
        }
        
        expression();
        emit(printOp, 0, 0, 0);
    }
    
    // 4. RETORNO
    else if (t.type == TOK_RETORNAR) {
        getNextToken();
        expression();
        emit(OP_RETORNAR, 0, 0, 0);
    }
    
    // 5. IF / SI
    else if (t.type == TOK_SI) {
        getNextToken(); 
        expression(); 
        match(TOK_ENTONCES);
        int jmp = emit(OP_SALTAR_FALSO, 0, 0, -1);
        block();
        patch(jmp, getCodeCount());
    }
    
    // 6. WHILE / MIENTRAS
    else if (t.type == TOK_MIENTRAS) {
        getNextToken(); 
        int start = getCodeCount();
        expression(); 
        match(TOK_HACER);
        int jmp = emit(OP_SALTAR_FALSO, 0, 0, -1);
        block();
        emit(OP_SALTAR, 0, 0, start);
        patch(jmp, getCodeCount());
    }

    else if (t.type == TOK_PARA) {
        getNextToken(); // Consumir 'para'
        match(TOK_PAR_IZQ);
        statement(); 
        
        match(TOK_PUNTO_COMA); // ; Separador 1

        // 2. ETIQUETA DE CONDICION (Aqui volvemos en cada vuelta)
        int condLabel = getCodeCount();

        // 3. EVALUAR CONDICION (ej: i < 10)
        expression();
        
        match(TOK_PUNTO_COMA); // ; Separador 2

        // 4. SALTO DE SALIDA (Si la condicion es falsa, nos vamos al final)
        int jumpExit = emit(OP_SALTAR_FALSO, 0, 0, -1);

        // 5. SALTO AL CUERPO (Saltamos sobre el incremento, porque el incremento va DESPUES del cuerpo)
        int jumpBody = emit(OP_SALTAR, 0, 0, -1);

        // 6. ETIQUETA DE INCREMENTO (Aqui volvemos despues del cuerpo)
        int incLabel = getCodeCount();
        
        // Parseamos el incremento (ej: i = i + 1)
        // Usamos statement() porque maneja asignaciones 'x = ...'
        statement(); 
        
        // Despues de incrementar, volvemos a checar la condicion
        emit(OP_SALTAR, 0, 0, condLabel); 

        // 7. INICIO DEL CUERPO
        patch(jumpBody, getCodeCount()); // El salto del paso 5 aterriza aqui
        
        match(TOK_PAR_DER); // )
        block(); // { ... codigo ... }

        // 8. AL FINAL DEL CUERPO, SALTAMOS AL INCREMENTO
        emit(OP_SALTAR, 0, 0, incLabel);

        // 9. PUNTO DE SALIDA (El salto falso del paso 4 aterriza aqui)
        patch(jumpExit, getCodeCount());
    }
    
    // ERROR / BASURA
    else {
        printf("Error: Token inesperado '%s' en linea %d\n", t.value, t.line);
        getNextToken(); // Avanzar para evitar loop infinito
    }
}

// ---------------------------------------------------------
// BLOQUES Y DEFINICIONES
// ---------------------------------------------------------
void block() {
    match(TOK_LLAVE_IZQ);
    while(peekToken().type != TOK_LLAVE_DER && peekToken().type != TOK_EOF) {
        statement();
    }
    match(TOK_LLAVE_DER);
}

void functionDef() {
    match(TOK_FUNC);
    Token name = getNextToken();
    match(TOK_PAR_IZQ);
    
    // Registramos la función
    declareSymbol(name.value, SYM_FUNCION, TIPO_VOID, getCodeCount(), 0);
    Symbol* funcSym = findSymbol(name.value);
    
    pushScope(name.value);
    localOffsetCounter = 0;
    
    int params = 0;
    if (peekToken().type != TOK_PAR_DER) {
        do {
            if (peekToken().type == TOK_COMA) match(TOK_COMA);
            
            // Exigimos tipo en parámetros
            DataType paramType = TIPO_ENTERO; // Default
            if (peekToken().type == TOK_ENTERO) { match(TOK_ENTERO); paramType = TIPO_ENTERO; }
            else if (peekToken().type == TOK_CHAR) { match(TOK_CHAR); paramType = TIPO_CHAR; }
            else if (peekToken().type == TOK_CADENA) { match(TOK_CADENA); paramType = TIPO_CADENA; }
            else if (peekToken().type == TOK_LISTA) { match(TOK_LISTA); paramType = TIPO_LISTA; }
            else { printf("Error: Se esperaba tipo en parametro\n"); exit(1); }

            Token paramName = getNextToken();
            declareSymbol(paramName.value, SYM_VAR_LOCAL, paramType, localOffsetCounter++, 0);
            params++;
        } while (peekToken().type == TOK_COMA);
    }
    match(TOK_PAR_DER);
    funcSym->paramCount = params;
    
    block();
    
    // Retorno de seguridad
    emit(OP_LITERAL, 0, 0, 0);
    emit(OP_RETORNAR, 0, 0, 0);
    popScope();
}

void parse() {
    initSymbolTable();
    pushScope("Global");
    
    match(TOK_INICIO);
    
    // Salto al main
    int jmpMain = emit(OP_SALTAR, 0, 0, -1);
    
    while(peekToken().type == TOK_FUNC) {
        functionDef();
    }
    
    patch(jmpMain, getCodeCount());
    
    while(peekToken().type != TOK_FIN && peekToken().type != TOK_EOF) {
        statement();
    }
    
    match(TOK_FIN);
    emit(OP_HALT, 0, 0, 0);
}