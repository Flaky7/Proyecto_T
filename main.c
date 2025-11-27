#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "generator.h"
#include "vm.h"
#include "symbols.h"

char* cargarArchivo(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: No se pudo abrir el archivo '%s'\n", filename);
        exit(1);
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(length + 1);
    if (!buffer) { printf("Error critico de memoria\n"); exit(1); }
    
    size_t leido = fread(buffer, 1, length, file);
    buffer[leido] = '\0';
    
    fclose(file);
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printf("Uso: ./interprete <archivo.alg>\n"); return 1; }

    char* source = cargarArchivo(argv[1]);
    
    // 1. Inicializar
    initLexer(source);
    initGenerator();
    initSymbolTable(); // Ahora sí funcionará porque incluimos symbols.h
    
    // 2. Compilar
    printf("Compilando...\n");
    parse(); 
    
    // 3. Debug (Opcional)
    printGeneratedCode(); 
    
    // 4. Ejecutar (SOLO UNA VEZ)
    runVM(getCode(), getCodeCount());
    
    // 5. Limpieza
    free(source);
    freeLexer();
    freeGenerator();
    
    return 0;
}