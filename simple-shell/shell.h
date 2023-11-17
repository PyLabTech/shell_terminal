#ifndef SHELL_H
#define SHELL_H

extern char **environ;

#define MAX_LINE 1024

char *my_getline(void);
char *my_strtok(char *str, const char *delim);
char *replace_variables(char *line);  // Add this line
char **split_line(char *line);
int execute_logical_op(char **args, const char *logical_op);
void free_args(char **args);

#endif // SHELL_H
