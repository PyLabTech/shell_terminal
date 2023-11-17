// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"

/**
 * main - The main function of the shell.
 * @argc: The number of arguments.
 * @argv: An array of argument strings.
 * Return: 0 on success.
 */
int main(int argc, char *argv[]) {
    char *line;

    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        while (1) {
            line = my_getline();

            if (line == NULL) {
                break;
            }

            line[strcspn(line, "\n")] = '\0';

            char **commands = split_line(line);
            for (int i = 0; commands[i] != NULL; i++) {
                char **args = split_line(commands[i]);
                execute_logical_op(args, my_strtok(NULL, " \t\n;"));
                free_args(args);
            }

            free_args(commands);
            free(line);
        }

        fclose(file);
    } else {
        while (1) {
            printf("$ ");
            fflush(stdout);
            line = my_getline();

            if (line == NULL) {
                printf("\n");
                break;
            }

            line[strcspn(line, "\n")] = '\0';

            char **commands = split_line(line);
            for (int i = 0; commands[i] != NULL; i++) {
                char **args = split_line(commands[i]);
                execute_logical_op(args, my_strtok(NULL, " \t\n;"));
                free_args(args);
            }

            free_args(commands);
            free(line);
        }
    }

    return 0;
}
