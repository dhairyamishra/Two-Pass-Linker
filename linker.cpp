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
    int linenum = 1;
    int token_counter = 0;
    char *token;

    while ((token = getNextToken(file)) != NULL) {
        printf("Token: %d:%d : %s\n", token_counter, linenum, token);
        token_counter++;

        // Check if a new line has been read in getNextToken()
        // We assume linenum should be incremented when a new line is read.
        // Here’s a typical approach (but not perfect for all cases):
        if (token_counter == 1) {
            linenum++;
        }
    }

    fclose(file);
    return 0;
}
