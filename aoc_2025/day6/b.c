#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 5000
#define MAX_PROBLEMS 2000
#define MAX_ARGS 5

char operands[MAX_PROBLEMS] = {0};
char problem_args[MAX_PROBLEMS][MAX_ARGS][6] = {0};
int n_problems = 0;

int main() {
    char delim = '\n';
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    // FILE *fp = fopen("sample.txt", "r");
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        perror("Fopen");
        return 1;
    }

    int n_lines = 0;
    char input[MAX_ARGS][MAX_LINE] = {0};
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (strlen(line) == 0) {
            continue;
        }

        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        strcpy(input[n_lines], line);
        n_lines++;
    }
    fclose(fp);

    int problem_starts[MAX_PROBLEMS] = {0};
    printf("%s\n", input[n_lines - 1]);
    for (int i = MAX_LINE - 1; i >= 0; i--) {
        if (input[n_lines - 1][i] == '+' || input[n_lines - 1][i] == '*') {
            operands[n_problems] = input[n_lines - 1][i];
            problem_starts[n_problems] = i;
            n_problems++;
            printf("%d\n", i);
        }
    }
    printf("%d\n", n_problems);

    // for (int i = 0; i < n_problems; i++) {
    //     for (int j = 0; j < n_lines - 1; j++) {
    //
    //     }
    // }

    return 0;
}
