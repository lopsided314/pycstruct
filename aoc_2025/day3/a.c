#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int part_one(const char *line) {
    int j1 = 0, j2 = 0;
    size_t i1 = 0, i2 = 0;

    // find the index of the first number
    for (char n = '9'; n > '0'; n--) {
        for (i1 = 0; i1 < strlen(line) - 1; i1++) {
            if (line[i1] == n) {
                // printf("matched 1 %c %ld\n", n, i1);
                j1 = n - '0';
                break;
            }
        }
        if (j1) {
            break;
        }
    }

    // find the index of the second number, starting
    // after the first number
    for (char n = '9'; n > '0'; n--) {
        for (i2 = i1 + 1; i2 < strlen(line); i2++) {
            if (line[i2] == n) {
                // printf("matched 2 %c %ld\n", n, i2);
                j2 = n - '0';
                break;
            }
        }
        if (j2) {
            break;
        }
    }

    return 10 * j1 + j2;
}

long part_two(const char *line, int n_jolts) {
    char joltages[n_jolts + 1];
    memset(joltages, 0, sizeof(joltages));
    size_t iLast = 0;

    for (int j = 0; j < n_jolts; j++) {
        for (char n = '9'; n > '0'; n--) {
            for (size_t i = iLast; i < strlen(line) - (n_jolts - 1 - j); i++) {
                // printf("checked j=%d, n=%c i=%ld\n", j, n, i);
                if (line[i] == n) {
                    // printf("matched j=%d, n=%c i=%ld\n", j, n, i);
                    joltages[j] = n;
                    iLast = i + 1;
                    break;
                }
            }
            if (joltages[j]) {
                break;
            }
        }
    }

    printf("joltage = %s\n", joltages);
    return strtol(joltages, NULL, 10);
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

    long sum = 0;
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        sum += part_two(line, 12);
    }
    printf("sum = %ld\n", sum);
    fclose(fp);
}
