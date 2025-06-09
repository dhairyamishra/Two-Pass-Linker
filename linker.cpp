#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

const char* INPUT_DIR = "./sample_inputs/";
const char* OUTPUT_DIR = "./sample_outputs/";

//using strtok() get pointer to char array for token
char* getNextToken(FILE *file) {
    static char line[256];
    static char *token = NULL;
    while (1) {
        if (token == NULL) {
            if (fgets(line, sizeof(line), file) == NULL) {
                return NULL;  // No more lines in the file
            }
            token = strtok(line, " \t\n");
        } else {
            token = strtok(NULL, " \t\n");
        }

        if (token != NULL) {
            return token;  // Found a token!
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
        int def_number = atoi(token);
        if (def_number < 0) {
            printf("Error: Negative number of definitions\n");
            return;
        }
        for( int i = 0; i < def_number; i++) {
            char *sym = getNextToken(file);
            int rel_addr = atoi(getNextToken(file));
            printf("Module %d defines symbol %s with relative address %d\n",
                   module, sym, rel_addr);
        }
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
