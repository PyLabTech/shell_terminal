// split_line.c
// split_line.c
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LINE 1024

/**
 * split_line - Splits a line into an array of strings.
 * @line: The input line.
 * Return: An array of strings.
 */
char **split_line(char *line) {
    int bufsize = MAX_LINE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        perror("Allocation error");
        exit(EXIT_FAILURE);
    }

    token = my_strtok(line, " \t\n;");
    while (token != NULL) {
        tokens[position++] = strdup(token);

        if (position >= bufsize) {
            bufsize += MAX_LINE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                perror("Allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = my_strtok(NULL, " \t\n;");
    }
    tokens[position] = NULL;
    return tokens;
}
