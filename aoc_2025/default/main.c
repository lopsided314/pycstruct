#include <stdio.h>
#include <string.h>
#include <unistd.h>

int part_one() {
    char delim = '\n';
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    FILE *fp = fopen("sample.txt", "r");
    // FILE *fp = fopen("input.txt", "r");
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
    }

    fclose(fp);
    return 0;
}

int main() { return part_one(); }
