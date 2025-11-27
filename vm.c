#include "vm.h"
#include "generator.h" // <--- OBLIGATORIO: Para ver getStringConstant y evitar crashes
#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 1024
int stack[STACK_SIZE];
int sp = -1, fp = 0;
int globals[100];
int returnStack[100], fpStack[100], callDepth = 0;

// === HEAP PARA LISTAS ===
typedef struct {
    int* data;
    int size;
    int capacity;
} ArrayList;

ArrayList listHeap[100];
int listHeapCount = 0;

int createList() {
    listHeap[listHeapCount].data = malloc(sizeof(int) * 4);
    listHeap[listHeapCount].size = 0;
    listHeap[listHeapCount].capacity = 4;
    return listHeapCount++;
}

void listPush(int listId, int val) {
    ArrayList* l = &listHeap[listId];
    if (l->size >= l->capacity) {
        l->capacity *= 2;
        l->data = realloc(l->data, sizeof(int) * l->capacity);
    }
    l->data[l->size++] = val;
}

int listGet(int listId, int index) {
    if (index < 0 || index >= listHeap[listId].size) {
        printf("Error Runtime: Indice fuera de rango\n"); exit(1);
    }
    return listHeap[listId].data[index];
}

// === UTILS VM ===
void push(int v) { 
    if(sp >= STACK_SIZE-1) { printf("Error: Stack Overflow\n"); exit(1); }
    stack[++sp] = v; 
}
int pop() { 
    if(sp < 0) { printf("Error: Stack Underflow\n"); exit(1); }
    return stack[sp--]; 
}

void runVM(Instruction* code, int count) {
    printf("\n=== EJECUCION VM (Con soporte de tipos) ===\n");
    int ip = 0;
    
    while(1) {
        // SEGURIDAD: Si ip se sale del código, paramos
        if (ip >= count) { printf("Error: IP fuera de rango\n"); return; }
        
        Instruction i = code[ip];
        int val1, val2;
        
        switch(i.op) {
            case OP_LITERAL: push(i.arg1); ip++; break;
            
            case OP_CARGAR_VAR:
                 if(i.arg2==0) push(globals[i.arg1]); 
                 else push(stack[fp+i.arg1]); 
                 ip++; break;
                 
            case OP_GUARDAR_VAR:
                 val1=pop(); 
                 if(i.arg2==0) globals[i.arg1]=val1; 
                 else stack[fp+i.arg1]=val1; 
                 ip++; break;

            case OP_SUMAR: val2=pop(); val1=pop(); push(val1+val2); ip++; break;
            case OP_RESTAR: val2=pop(); val1=pop(); push(val1-val2); ip++; break;
            case OP_MENOR: val2=pop(); val1=pop(); push(val1<val2); ip++; break;
            
            case OP_SALTAR_FALSO: val1=pop(); ip=(val1==0)?i.target:ip+1; break;
            case OP_SALTAR: ip=i.target; break;
            
            case OP_LLAMAR:
                returnStack[callDepth] = ip + 1;
                fpStack[callDepth] = fp;
                callDepth++;
                fp = sp - i.arg2 + 1; 
                ip = i.arg1;
                break;
                
            case OP_RETORNAR:
                val1 = pop(); 
                sp = fp - 1; 
                callDepth--;
                ip = returnStack[callDepth];
                fp = fpStack[callDepth];
                push(val1); 
                break;

            // --- IMPRESION ---
            case OP_ESCRIBIR_INT: 
                printf("OUT [INT]: %d\n", pop()); ip++; break;
                
            case OP_ESCRIBIR_CHAR: 
                printf("OUT [CHAR]: %c\n", (char)pop()); ip++; break;
                
            case OP_ESCRIBIR_STR:
                val1 = pop(); 
                // AQUI OCURRIA EL CRASH SI FALTABA generator.h
                printf("OUT [STR]: %s\n", getStringConstant(val1)); 
                ip++; break;
                
            case OP_ESCRIBIR_LIST:
                val1 = pop(); 
                printf("OUT [LIST]: [ ");
                for(int k=0; k<listHeap[val1].size; k++) 
                    printf("%d ", listHeap[val1].data[k]);
                printf("]\n");
                ip++; break;

            // --- LISTAS ---
            case OP_NUEVA_LISTA:
                val1 = createList();
                push(val1); 
                ip++; break;
                
            case OP_LISTA_PUSH:
                val1 = pop(); // Elemento
                val2 = pop(); // ID Lista
                listPush(val2, val1);
                push(val2); // Dejamos la lista en el stack
                ip++; break;
            
            case OP_LISTA_GET:
                val1 = pop(); // Indice
                val2 = pop(); // ID Lista
                push(listGet(val2, val1));
                ip++; break;
                
            case OP_HALT: 
                printf("=== FIN DEL PROGRAMA ===\n"); 
                return; // <--- IMPORTANTE: Rompe el bucle while(1)
            
            default:
                printf("Error: OpCode desconocido %d\n", i.op);
                return;
        }
    }
}