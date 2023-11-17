// my_strtok.c
#include <string.h>

/**
 * my_strtok - Custom implementation of strtok.
 * @str: The string to be tokenized.
 * @delim: The delimiter characters.
 * Return: A pointer to the next token or NULL if no more tokens.
 */
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
