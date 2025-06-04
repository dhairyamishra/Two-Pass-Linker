#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("input-1", "r");
    if (!file) {
        printf("Error opening file\n");
        return 1;
    }
    char line[256];
    int linenum = 1;
    while (fgets(line, sizeof(line), file)) {
        printf("Line %d: %s", linenum, line);
        char *token = strtok(line, " \t\n");
        int token_counter = 0;
        while (token != NULL) {
            printf("Token: %d:%d : %s\n", token_counter, linenum, token);
            token = strtok(NULL, " \t\n");
            token_counter++;
        }
        linenum++;
    }

    fclose(file);
    return 0;
}
