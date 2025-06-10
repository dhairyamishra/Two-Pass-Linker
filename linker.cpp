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
int error_count = 0;
int warning_count = 0;

std::map<std::string, int> symbolTable;
std::map<std::string, bool> symbolUsed;
std::map<std::string, int> symbolModule; 
std::map<std::string, bool> symbolMultiplyDefined;

// Tokenizer
char* getNextToken(FILE *file) {
    static char line[256];
    static char *token = NULL;
    static char *currentLine = NULL;
    while (true) {
        if (token == NULL) {
            if (fgets(line, sizeof(line), file) == NULL) {
                return NULL;
            }
            linenum++;            
            currentLine = line;   
            token = strtok(currentLine, " \t\n");
            lineoffset = 1;
        } else {
            token = strtok(NULL, " \t\n");
        }
        if (token != NULL) {
            lineoffset = (token - currentLine) + 1;
            return token; 
        }
    }
}

char* checkedGetNextToken(FILE *file, const char* expected) {
    char* token = getNextToken(file);
    if (token == NULL) {
        printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, expected);
        exit(1);
    }
    return token;
}

void firstPASS(FILE *file) {
    int base_address = 0;
    char *token;
    int module = 0;

    while ((token = getNextToken(file)) != NULL) {
        int defcount = atoi(token);
        if (defcount > 16) {
            printf("Parse Error line %d offset %d: TOO_MANY_DEF_IN_MODULE\n", linenum, lineoffset);
            exit(1);
        }

        struct {
            std::string name;
            int rel_addr;
        } module_def[16];

        for (int i = 0; i < defcount; i++) {
            char *sym = checkedGetNextToken(file, "SYM_EXPECTED");
            int rel_addr = atoi(checkedGetNextToken(file, "NUM_EXPECTED"));
            module_def[i].name = sym;
            module_def[i].rel_addr = rel_addr;
        }

        usecount = atoi(checkedGetNextToken(file, "NUM_EXPECTED"));
        for (int i = 0; i < usecount; i++) {
            char *sym = checkedGetNextToken(file, "SYM_EXPECTED");
            strcpy(useList[i], sym);
        }

        int codecount = atoi(checkedGetNextToken(file, "NUM_EXPECTED"));
        for (int i = 0; i < codecount; i++) {
            checkedGetNextToken(file, "ADDR_EXPECTED");
            checkedGetNextToken(file, "NUM_EXPECTED");
        }

        for (int i = 0; i < defcount; i++) {
            int rel_addr = module_def[i].rel_addr;
            std::string sym = module_def[i].name;
            int abs_addr = base_address + rel_addr;

            if (rel_addr >= codecount) {
                if(codecount - 1 <= 0) {
                    printf("Warning: Module %d: %s redefinition ignored\n",
                       module, sym.c_str());
                }
                else{
                    printf("Warning: Module %d: %s=%d valid=[0..%d] assume zero relative\n",
                       module, sym.c_str(), rel_addr, codecount - 1);
                }
                abs_addr = base_address;
                warning_count++;
            }

            if (symbolTable.find(sym) != symbolTable.end()) {
                symbolMultiplyDefined[sym] = true;
            } else {
                symbolTable[sym] = abs_addr;
                symbolUsed[sym] = false;
                symbolModule[sym] = module;
            }
        }

        base_address += codecount;
        module++;
    }

    printf("SymTable:\n");
    for (const auto &entry : symbolTable) {
        printf("%s=%d", entry.first.c_str(), entry.second);
        if (symbolMultiplyDefined[entry.first]) {
            printf(" Error: This variable is multiple times defined; first value used");
            error_count++;
        }
        printf("\n");
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
        for (int i = 0; i < defcount; i++) {
            checkedGetNextToken(file, "SYM_EXPECTED");
            checkedGetNextToken(file, "NUM_EXPECTED");
        }

        int usecount = atoi(checkedGetNextToken(file, "NUM_EXPECTED"));
        char useList[16][20];
        bool used[16] = {false};

        for (int i = 0; i < usecount; i++) {
            char *sym = checkedGetNextToken(file, "SYM_EXPECTED");
            strcpy(useList[i], sym);
        }

        int codecount = atoi(checkedGetNextToken(file, "NUM_EXPECTED"));
        for (int i = 0; i < codecount; i++) {
            char *addrmode = checkedGetNextToken(file, "ADDR_EXPECTED");
            char *instr_str = checkedGetNextToken(file, "NUM_EXPECTED");
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
                // Immediate: leave as is
            } else if (addrmode[0] == 'A') {
                if (operand >= 512) {
                    resolved = opcode * 1000;
                    error = " Error: Absolute address exceeds machine size; zero used";
                    error_count++;
                }
            } else if (addrmode[0] == 'R') {
                if (operand >= codecount) {
                    resolved = opcode * 1000 + base_address;
                    error = " Error: Relative address exceeds module size; relative zero used";
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
                        symbolUsed[sym] = true; 
                    } else {
                        resolved = opcode * 1000;
                        error = " Error: " + sym + " is not defined; zero used";
                        error_count++;
                    }
                }
            }

            printf("%03d: %04d%s\n", index++, resolved, error.c_str());
        }

        for (int i = 0; i < usecount; i++) {
            if (!used[i]) {
                printf("Warning: Module %d: uselist[%d]=%s was not used\n", module, i, useList[i]);
                warning_count++;
            }
        }

        base_address += codecount;
        module++;
    }

    printf("\n");
    for (const auto &entry : symbolUsed) {
        if (!entry.second) {
            printf("Warning: Module %d: %s was defined but never used\n",
                   symbolModule[entry.first], entry.first.c_str());
            warning_count++;
        }
    }

    printf("\nSummary: Errors=%d Warnings=%d\n", error_count ,warning_count);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <inputfile>\n", argv[0]);
        return 1;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./sample_inputs/%s", argv[1]);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Error opening file %s\n", filepath);
        return 1;
    }
    firstPASS(file);
    fclose(file);

    file = fopen(filepath, "r");
    if (!file) {
        printf("Error reopening file %s\n", filepath);
        return 1;
    }
    secondPASS(file);
    fclose(file);

    return 0;
}
