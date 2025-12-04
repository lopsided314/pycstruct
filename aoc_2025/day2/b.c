#include <stdio.h>
#include <string.h>
#include <unistd.h>

int repeated_digits(long n) {
    int match = 0;
    char str[100] = {0};
    char key[100] = {0};

    sprintf(str, "%ld", n);
    // printf("str = %s\n", str);
    for (size_t i = 1; i < strlen(str); i++) {
        strncpy(key, str, i);
        match = 1;
        for (size_t j = 0; j < strlen(str); j += i) {
            // printf("%s %s %li\n", key, str+j, j);
            if (strncmp(str + j, key, i) != 0) {
                match = 0;
                break;
            }
        }
        if (match) {
            printf("Matched %ld\n", n);
            return 1;
        }
    }
    return 0;
}

int main() {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    // FILE *fp = fopen("sample.txt", "r");
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        perror("Fopen");
        return 1;
    }

    long start, stop, sum = 0;
    while ((read = getdelim(&line, &len, ',', fp)) != -1) {
        // printf("line = %s\n", line);
        sscanf(line, "%ld-%ld", &start, &stop);
        // printf("%ld %ld\n", start, stop);
        for (long i = start; i <= stop; i++) {
            if (repeated_digits(i)) {
                // printf("%ld\n",  i);
                sum += i;
            }
            // printf("\n");
        }
    }
    printf("sum = %ld\n", sum);
    fclose(fp);
}
