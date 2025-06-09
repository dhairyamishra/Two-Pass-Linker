#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const char* INPUT_DIR = "./sample_inputs/";
const char* OUTPUT_DIR = "./sample_outputs/";
char useList[16][20];  
int linenum = 0;       
int lineoffset = 1;    
int usecount = 0;
int symbolCount = 0;
std::map<std::string, int> symbolTable;


//using strtok() get pointer to char array for token
char* getNextToken(FILE *file) {
    static char line[256];
    static char *token = NULL;
    static char *currentLine = NULL;
    while (true) {
        if (token == NULL) {
            if (fgets(line, sizeof(line), file) == NULL) {
                return NULL;
            }
            // add new line for proper line tracking, was wrong first
            linenum++;            
            currentLine = line;   
            token = strtok(line, " \t\n");
            // reset line offset when token is null
            lineoffset = 1;
        } else {
            token = strtok(NULL, " \t\n");
        }
        if (token != NULL) {
            //need to add plus 1 since offest was adjusted before
            lineoffset = (token - currentLine) + 1;
            return token; 
        }
        // If not, the line was empty
    }
}

void firstPASS(FILE *file) {
    int base_address = 0;
    char *token;
    int module = 0;

    while ((token = getNextToken(file)) != NULL) {
        int defcount = atoi(token);
        if (defcount < 0) {
            printf("Error: Negative definition count in module %d\n", module);
            return;
        }
        if (defcount > 16) {
            printf("Parse Error line %d offset %d: TOO_MANY_DEF_IN_MODULE\n", linenum, lineoffset);
            exit(1);
        }
        // Read definitions
        for (int i = 0; i < defcount; i++) {
            char *sym = getNextToken(file);
            int rel_addr = atoi(getNextToken(file));
            int abs_addr = base_address + rel_addr;

            std::string symbol(sym);

            if (symbolTable.find(symbol) != symbolTable.end()) {
                printf("Warning: Module %d: %s redefinition ignored\n", module, sym);
            } else {
                symbolTable[symbol] = abs_addr;
                printf("Defined symbol: %s=%d (Module %d, relative %d)\n",
                       sym, abs_addr, module, rel_addr);
            }
        }

        // Read use list
        usecount = atoi(getNextToken(file));
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
            printf("Instruction: %c %s\n", addrmode[0], instr);
        }

        printf("Processed module %d -- base address %d, size %d\n", module, base_address, codecount);

        base_address += codecount;
        module++;
    }

    // Print symbol table from the map
    printf("\nSymbol Table\n");
    for (const auto &entry : symbolTable) {
        printf("%s=%d\n", entry.first.c_str(), entry.second);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s, ", argv[0]);
        return 1;
    }

    // Build full path to the input file
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./sample_inputs/%s", argv[1]);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Error opening file %s\n", filepath);
        return 1;
    }
    firstPASS(file);
    fclose(file);
    return 0;
}
