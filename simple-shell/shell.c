// shell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"
#define MAX_LINE 1024

/**
 * execute_command - Executes a command with arguments.
 * @args: The array of arguments.
 * Return: 1 on success, 0 on failure.
 */
int execute_command(char **args) {
    pid_t pid;
    int status;
    char *line;

    if (args[0] == NULL || args[0][0] == '#') {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            int exit_status = atoi(args[1]);
            free_args(args);
            exit(exit_status);
        } else {
            free_args(args);
            exit(EXIT_SUCCESS);
        }
    } else if (strcmp(args[0], "env") == 0) {
        char *const *env = environ;
        while (*env) {
            printf("%s\n", *env);
            env++;
        }
    } else if (strcmp(args[0], "setenv") == 0) {
        if (args[1] != NULL && args[2] != NULL && args[3] == NULL) {
            if (setenv(args[1], args[2], 1) != 0) {
                fprintf(stderr, "setenv: Unable to set environment variable\n");
            }
        } else {
            fprintf(stderr, "setenv: Incorrect syntax. Usage: setenv VARIABLE VALUE\n");
        }
    } else if (strcmp(args[0], "unsetenv") == 0) {
        if (args[1] != NULL && args[2] == NULL) {
            if (unsetenv(args[1]) != 0) {
                fprintf(stderr, "unsetenv: Unable to unset environment variable\n");
            }
        } else {
            fprintf(stderr, "unsetenv: Incorrect syntax. Usage: unsetenv VARIABLE\n");
        }
    } else if (strcmp(args[0], "cd") == 0) {
        char *new_directory = args[1];

        if (new_directory == NULL || strcmp(new_directory, "~") == 0) {
            new_directory = getenv("HOME");
        } else if (strcmp(new_directory, "-") == 0) {
            new_directory = getenv("OLDPWD");
        }

        if (new_directory != NULL && chdir(new_directory) == 0) {
            char cwd[MAX_LINE];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                setenv("PWD", cwd, 1);
                setenv("OLDPWD", cwd, 1);
            } else {
                perror("getcwd");
            }
        } else {
            perror("cd");
        }
    } else if (strcmp(args[0], "alias") == 0) {
        fprintf(stderr, "Not implemented: Alias command\n");
    } else {
        line = replace_variables(args[0]);
        pid = fork();
        if (pid == 0) {
            if (execvp(line, args) == -1) {
                perror("Error");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            perror("Error");
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, &status, 0);
        }
        free(line);
    }

    return 1;
}
