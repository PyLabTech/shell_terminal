#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
extern char **environ; 
void free_args(char **args);

char *my_getline(void) {
    char *line = NULL;
    size_t bufsize = 0;
    if (getline(&line, &bufsize, stdin) == -1) {
        free(line);
        return NULL;
    }
    return line;
}

char *my_strtok(char *str, const char *delim) {
    static char *last_token = NULL;
    char *token_start;
    int in_delim = 1;

    if (str != NULL) {
        last_token = str;
    }

    while (*last_token != '\0') {
        if (strchr(delim, *last_token) == NULL) {
            in_delim = 0;
            break;
        }
        last_token++;
    }

    if (*last_token == '\0') {
        return NULL;
    }

    token_start = last_token;

    while (*last_token != '\0') {
        if (strchr(delim, *last_token) != NULL) {
            *last_token = '\0';
            last_token++;
            break;
        }
        last_token++;
    }

    if (in_delim) {
        return my_strtok(NULL, delim);
    }

    return token_start;
}

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
                // This is the child process
                execlp("true", "true", (char *)NULL);
                // If execlp fails
                perror("Error in execlp");
                exit(EXIT_FAILURE);
            } else {
                // This is the parent process
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

void free_args(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}

int execute_logical_op(char **args, const char *logical_op) {
    int result = execute_command(args);

    if (logical_op != NULL) {
        if (strcmp(logical_op, "&&") == 0 && result == 1) {
            return execute_logical_op(args + 2, my_strtok(NULL, " \t\n;"));
        } else if (strcmp(logical_op, "||") == 0 && result == 0) {
            return execute_logical_op(args + 2, my_strtok(NULL, " \t\n;"));
        }
    }

    return result;
}

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
