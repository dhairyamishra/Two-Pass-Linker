#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <cctype>
#include <vector>

int linenum = 0, lineoffset = 1;
int error_count = 0, warning_count = 0;
const int MACHINE_SIZE = 512;

std::map<std::string, int> symbolTable;
std::map<std::string, bool> symbolUsed;
std::map<std::string, int> symbolModule;
std::map<std::string, bool> symbolMultiplyDefined;
std::vector<std::string> symbolOrder;

char* getNextToken(FILE *file) {
    static char line[256];
    static char *token = nullptr, *currentLine = nullptr;
    while (true) {
        if (token == nullptr) {
            if (fgets(line, sizeof(line), file) == nullptr) return nullptr;
            linenum++;
            currentLine = line;
            token = strtok(currentLine, " \t\n");
            lineoffset = 1;
        } else {
            token = strtok(nullptr, " \t\n");
        }
        if (token != nullptr) {
            lineoffset = (token - currentLine) + 1;
            return token;
        }
    }
}

void __parseerror(int errcode) {
    static const char* errstr[] = {
        "NUM_EXPECTED", "SYM_EXPECTED", "ADDR_EXPECTED", "SYM_TOO_LONG",
        "TOO_MANY_DEF_IN_MODULE", "TOO_MANY_USE_IN_MODULE", "TOO_MANY_INSTR"
    };
    printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
    exit(1);
}


int readInt(FILE *file) {
    char *tok = getNextToken(file);
    if (!tok) __parseerror(0);
    for (int i=0; tok[i]; i++)
        if (!isdigit(tok[i])) __parseerror(0);
    return atoi(tok);
}

std::string readSymbol(FILE *file) {
    char *tok = getNextToken(file);
    if (!tok) __parseerror(1);
    if (!isalpha(tok[0])) __parseerror(1);
    for (int i=1; tok[i]; i++)
        if (!isalnum(tok[i])) __parseerror(1);
    if (strlen(tok) > 16) __parseerror(3);
    return std::string(tok);
}

char readMARIE(FILE *file) {
    char *tok = getNextToken(file);
    if (!tok) __parseerror(2);
    if (strchr("MARIE", tok[0]) == nullptr) __parseerror(2);
    return tok[0];
}

void firstPASS(FILE *file) {
    int baseAddr = 0, module = 0, totalInstructions = 0;
    while (true) {
        char *tok = getNextToken(file);
        if (!tok) break; // EOF
        int defcount = atoi(tok);
        if (defcount > 16) __parseerror(4);

        std::map<std::string, int> moduleDefs;
        for (int i=0; i<defcount; i++) {
            std::string sym = readSymbol(file);
            int relAddr = readInt(file);
            moduleDefs[sym] = relAddr;
        }

        int usecount = readInt(file);
        if (usecount > 16) __parseerror(5);
        for (int i=0; i<usecount; i++) readSymbol(file);

        int codecount = readInt(file);
        totalInstructions += codecount;
        if (totalInstructions > MACHINE_SIZE) {
            __parseerror(6); // 6 is the index for TOO_MANY_INSTR
        }

        for (int i=0; i<codecount; i++) {
            readMARIE(file);
            readInt(file);
        }

        for (auto &p : moduleDefs) {
            std::string sym = p.first;
            int relAddr = p.second;
            int absAddr = baseAddr + relAddr;

            if (relAddr >= codecount) {
                if(codecount-1 <= 0){
                    printf("Warning: Module %d: %s redefinition ignored\n", module, sym.c_str());
                }
                else{
                    printf("Warning: Module %d: %s=%d valid=[0..%d] assume zero relative\n", module, sym.c_str(), relAddr, codecount-1);
                }
                
                absAddr = baseAddr;
                warning_count++;
            }
            if (symbolTable.count(sym)) {
                symbolMultiplyDefined[sym] = true;
            } else {
                symbolTable[sym] = absAddr;
                symbolUsed[sym] = false;
                symbolModule[sym] = module;
                symbolOrder.push_back(sym);
            }
        }
        baseAddr += codecount;
        module++;
    }

    printf("SymTable:\n");
    for (auto &p : symbolOrder) {
        printf("%s=%d", p.c_str(), symbolTable[p]);
        if (symbolMultiplyDefined[p]) {
            printf(" Error: This variable is multiple times defined; first value used");
            error_count++;
        }
        printf("\n");
    }
}

void secondPASS(FILE *file) {
    int baseAddr = 0, module = 0, index = 0;
    printf("\nBinaryCode:\n");
    while (true) {
        char *tok = getNextToken(file);
        if (!tok) break;
        int defcount = atoi(tok);
        for (int i=0; i<defcount; i++) { readSymbol(file); readInt(file); }
        int usecount = readInt(file);
        char useList[16][20];
        bool used[16] = {false};
        for (int i=0; i<usecount; i++) {
            std::string sym = readSymbol(file);
            strcpy(useList[i], sym.c_str());
        }
        int codecount = readInt(file);
        for (int i=0; i<codecount; i++) {
            char mode = readMARIE(file);
            int instr = readInt(file);
            int opcode = instr / 1000, operand = instr % 1000;
            int resolved = instr;
            std::string error;

            if (instr >= 10000) { // too big
                resolved = 9999;
                error = " Error: Illegal opcode; treated as 9999";
                error_count++;
            } else if (mode == 'I') {
                if (instr >= 10000) {
                    resolved = 9999;
                    error = " Error: Illegal immediate operand; treated as 999";
                    error_count++;
                }
            } else if (mode == 'A') {
                if (operand >= MACHINE_SIZE) {
                    resolved = opcode * 1000;
                    error = " Error: Absolute address exceeds machine size; zero used";
                    error_count++;
                }
            } else if (mode == 'R') {
                if (operand >= codecount) {
                    resolved = opcode * 1000 + baseAddr;
                    error = " Error: Relative address exceeds module size; relative zero used";
                    error_count++;
                } else {
                    resolved = opcode * 1000 + operand + baseAddr;
                }
            } else if (mode == 'E') {
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
        for (int i=0; i<usecount; i++)
            if (!used[i]) {
                printf("Warning: Module %d: uselist[%d]=%s was not used\n", module, i, useList[i]);
                warning_count++;
            }
        baseAddr += codecount;
        module++;
    }
    for (auto &p : symbolUsed)
        if (!p.second) {
            printf("Warning: Module %d: %s was defined but never used\n",
                   symbolModule[p.first], p.first.c_str());
            warning_count++;
        }
    printf("\nSummary: Errors=%d Warnings=%d\n", error_count, warning_count);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <inputfile>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) { printf("Error opening file\n"); return 1; }
    firstPASS(file);
    fclose(file);

    file = fopen(argv[1], "r");
    if (!file) { printf("Error reopening file\n"); return 1; }
    secondPASS(file);
    fclose(file);

    return 0;
}
