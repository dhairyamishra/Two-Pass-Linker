#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

const char* INPUT_DIR = "./sample_inputs/";
const char* OUTPUT_DIR = "./sample_outputs/";
char useList[16][20];  
int usecount = 0;
struct SymbolEntry {
    char name[20];
    int address;
};

SymbolEntry symbolTable[256];
int symbolCount = 0;
//using strtok() get pointer to char array for token
char* getNextToken(FILE *file) {
    static char line[256];
    static char *token = NULL;
    while (true) {
        if (token == NULL) {
            if (fgets(line, sizeof(line), file) == NULL) {
                return NULL;  // No more lines in the file
            }
            token = strtok(line, " \t\n");
        } else {
            token = strtok(NULL, " \t\n");
        }

        if (token != NULL) {
            return token; 
        }
        // If not, the line was empty
    }
}

void firstPASS(FILE *file) {
    int base_address = 0;
    char *token;
    int module = 0;
    while((token = getNextToken(file)) != NULL) {
        //get the number of commands specified in the line
        int defcount = atoi(token);

        // Read definitions
        for (int i = 0; i < defcount; i++) {
            char *sym = getNextToken(file);
            int rel_addr = atoi(getNextToken(file));
            printf("Module %d for symbol %s --> relative address %d\n",module, sym, rel_addr);
            int abs_addr = base_address + rel_addr;
             // Store in the table
            strcpy(symbolTable[symbolCount].name, sym);
            symbolTable[symbolCount].address = abs_addr;
            symbolCount++;
            printf("Defined symbol: %s=%d\n", sym, abs_addr);
        }

        // Read use list
        int usecount = atoi(getNextToken(file));
        for (int i = 0; i < usecount; i++) {
            char *sym = getNextToken(file);
            strcpy(useList[i], sym);
            printf("Use list symbol: %s\n", sym);
        }

        // Read program text
        int codecount = atoi(getNextToken(file));
        for (int i = 0; i < codecount; i++) {
            char *addrmode = getNextToken(file);
            char *instr = getNextToken(file);
            // Pass 1: skip instructions
            printf("Instruction: %c %s\n", addrmode[0], instr);
        }

        printf("Processed module %d---> base address %d and size %d\n",
               module, base_address, codecount);
        base_address += codecount;
        module++;


    }
}

int main() {
    const char* filename = "input-1";
    char filepath[256];
    strcpy(filepath, INPUT_DIR);
    strcat(filepath, filename);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Error opening file");
        return 1;
    }
    // int linenum = 1;
    // int token_counter = 0;
    // char *token;
    firstPASS(file);

    fclose(file);
    return 0;
}
