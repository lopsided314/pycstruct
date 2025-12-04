#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

long ipow10(int e) {
    long ret = 1;
    if (e <= 0) {
        return ret;
    }
    while (e-- > 0) {
        ret *= 10;
    }
    return ret;
}

bool repeated_digits(long n) {
    int o = ceil(log10(n));
    if (o % 2) {
        return false;
    }
    long l = n % ipow10(o / 2);
    long u = n / ipow10(o / 2);
    return l == u;
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
                printf("%ld\n",  i);
                sum += i;
            }
        }
    }
    printf("sum = %ld\n", sum);
    fclose(fp);
}
