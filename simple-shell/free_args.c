#include <stdlib.h>

/**
 * free_args - Frees memory allocated for an array of strings.
 * @args: The array of strings to be freed.
 */
void free_args(char **args) {
    int i;

    for (i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}
