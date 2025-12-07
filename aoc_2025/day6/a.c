#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

long problem_args[1000][1000] = {0};
int n_problems = 0;
int n_args = 0;

long part_one() {
    long results[1000] = {0};
    long total = 0;
    int op_row = n_args - 1;
    n_args--;

    for (int iProb = 0; iProb < n_problems; iProb++) {
        char op = (char)problem_args[iProb][op_row];
        if (op == '*') {

            results[iProb] = 1;
            for (int iArg = 0; iArg < n_args; iArg++) {
                results[iProb] *= problem_args[iProb][iArg];
            }

        } else if (op == '+') {

            for (int iArg = 0; iArg < n_args; iArg++) {
                results[iProb] += problem_args[iProb][iArg];
            }
        }
        printf("problem %d = %ld\n", iProb, results[iProb]);
        total += results[iProb];
    }
    printf("total = %ld\n", total);
    return total;
}

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

    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        n_problems = 0;
        char *p = strtok(line, " ");
        while (p != NULL) {
            if (strlen(p) == 0)
                continue;

            if (isdigit(p[0])) {
                problem_args[n_problems][n_args] = atol(p);
                // printf("problem %d %ld\n", n_problems, problem_args[n_problems][n_args]);
            } else {
                problem_args[n_problems][n_args] = p[0];
                // printf("problem %d %c\n", n_problems, (char)problem_args[n_problems][n_args]);
            }

            n_problems++;
            p = strtok(NULL, " ");
        }
        n_args++;
        // printf("\n");
    }

    fclose(fp);
    part_one();
}
