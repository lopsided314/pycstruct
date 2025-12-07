#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINES 5
#define MAX_LINE 5000
#define MAX_PROBLEMS 2000

int main() {
    char delim = '\n';
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    FILE *fp = fopen("sample.txt", "r");
    // FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    int n_lines = 0;
    char input[MAX_LINES][MAX_LINE] = {0};
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

    int n_col = strlen(input[n_lines - 1]);
    long total;
    long problem_total = 0;
    char problem_operand = 0;
    for (int iCol = 0; iCol < n_col; iCol++) {
        
        // start a new problem
        if (input[n_lines-1][iCol] == '+' || input[n_lines-1][iCol] == '*') {
            problem_operand = input[n_lines-1][iCol];
            if (problem_total > 0) {
                printf("%ld\n", problem_total);
                total += problem_total;

                if (problem_operand == '*') {
                    problem_total = 1;
                } else {
                    problem_total = 0;
                }
            }
            continue;
        }

        for (int iLine = n_lines - 2; iLine >= 0; iLine--) {
            if (input[iLine][iCol] == ' ') 
                continue;
            printf("%c\n", input[iLine][iCol]);
        }
        printf("\n");
    }
    return 0;
}
