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
int error_count = 0;
int warning_count = 0;
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
            token = strtok(currentLine, " \t\n");
            // reset line offset when token is null
            lineoffset = 1;
        } 
        else {
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
            error_count++;
            return;
        }
        if (defcount > 16) {
            printf("Parse Error line %d offset %d: TOO_MANY_DEF_IN_MODULE\n", linenum, lineoffset);
            error_count++;
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
                warning_count++;
            } else {
                symbolTable[symbol] = abs_addr;
            }
        }

        // Read use list
        usecount = atoi(getNextToken(file));
        for (int i = 0; i < usecount; i++) {
            char *sym = getNextToken(file);
            strcpy(useList[i], sym);
        }

        // Read program text
        int codecount = atoi(getNextToken(file));
        for (int i = 0; i < codecount; i++) {
            getNextToken(file); // skip addrmode
            getNextToken(file); // skip instr
            //skipping these values since first pass does not need to store them
            
        }
        base_address += codecount;
        module++;
    }

    // Print symbol table from the map
    printf("SymTable:\n");
    for (const auto &entry : symbolTable) {
        printf("%s=%d\n", entry.first.c_str(), entry.second);
    }
}

void secondPASS(FILE *file) {
    int base_address = 0;
    char *token;
    int module = 0;
    int index = 0;

    printf("\nBinaryCode:\n");

    while ((token = getNextToken(file)) != NULL) {
        int defcount = atoi(token);
        // Skip definitions
        for (int i = 0; i < defcount; i++) {
            getNextToken(file); // symbol
            getNextToken(file); // relative address
        }

        // Read use list
        int usecount = atoi(getNextToken(file));
        char useList[16][20];
        bool used[16] = {false};

        for (int i = 0; i < usecount; i++) {
            char *sym = getNextToken(file);
            strcpy(useList[i], sym);
        }

        // Read program text
        int codecount = atoi(getNextToken(file));
        for (int i = 0; i < codecount; i++) {
            char *addrmode = getNextToken(file);
            char *instr_str = getNextToken(file);
            int instr = atoi(instr_str);
            int opcode = instr / 1000;
            int operand = instr % 1000;
            int resolved = instr;
            std::string error = "";

            if (opcode >= 10) {
                resolved = 9999;
                error = " Error: Illegal opcode; treated as 9999";
                error_count++;

            } else if (addrmode[0] == 'I') {
                // Immediate: Leave as is
            } else if (addrmode[0] == 'A') {
                if (operand >= 512) {
                    resolved = opcode * 1000;
                    error = " Error: Absolute address exceeds machine size; zero used";
                    error_count++;

                }
            } else if (addrmode[0] == 'R') {
                if (operand >= codecount) {
                    resolved = opcode * 1000 + base_address;
                    error = " Error: Relative address exceeds module size; zero used";
                    error_count++;

                } else {
                    resolved = opcode * 1000 + operand + base_address;
                }
            } else if (addrmode[0] == 'E') {
                if (operand >= usecount) {
                    resolved = opcode * 1000;
                    error = " Error: External operand exceeds length of uselist; treated as relative=0";
                    error_count++;

                } else {
                    std::string sym(useList[operand]);
                    used[operand] = true;
                    if (symbolTable.count(sym)) {
                        resolved = opcode * 1000 + symbolTable[sym];
                        // TODO: mark symbol as used in a separate tracking map for Rule 4
                    } else {
                        resolved = opcode * 1000;
                        error = " Error: " + sym + " is not defined; zero used";
                        error_count++;

                    }
                }
            }

            printf("%03d: %04d%s\n", index++, resolved, error.c_str());
        }

        // After processing instructions, print warnings for unused use list entries (Rule 7)
        for (int i = 0; i < usecount; i++) {
            if (!used[i]) {
                printf("Warning: Module %d: %s appeared in the uselist but was not used\n", module, useList[i]);
                warning_count++;
            }
        }

        base_address += codecount;
        module++;
    }

    printf("\nSummary: Errors=%d Warnings=%d\n", error_count ,warning_count);
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
    // Reopen file for pass 2
    file = fopen(filepath, "r");
    if (!file) {
        printf("Error reopening file %s\n", filepath);
        return 1;
    }
    secondPASS(file);
    return 0;
}
