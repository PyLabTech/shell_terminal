// replace_variables.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024

char *my_strtok(char *str, const char *delim);

/**
 * replace_variables - Replaces variables in a given line.
 * @line: The input line.
 * Return: A new string with variables replaced.
 */
char *replace_variables(char *line) {
    char *result = malloc(MAX_LINE);
    char *ptr = result;
    char *token = my_strtok(line, "$");

    while (token != NULL) {
        if (token[0] == '?' && token[1] == '\0') {
            int last_command_exit_status;
            pid_t child_pid = fork();

            if (child_pid == -1) {
                perror("Error forking");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) {
                execlp("true", "true", (char *)NULL);
                perror("Error in execlp");
                exit(EXIT_FAILURE);
            } else {
                waitpid(child_pid, &last_command_exit_status, 0);
                if (WIFEXITED(last_command_exit_status)) {
                    sprintf(ptr, "%d", WEXITSTATUS(last_command_exit_status));
                }
            }
        } else if (token[0] == '$' && token[1] == '\0') {
            sprintf(ptr, "%d", getpid());
        } else {
            sprintf(ptr, "%s", token);
        }

        ptr += strlen(ptr);
        token = my_strtok(NULL, "$");
    }

    *ptr = '\0';
    return result;
}
