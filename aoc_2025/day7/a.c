#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_COLS 200

int part_one() {
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

    char beams[MAX_COLS] = {0};
    memset(beams, '.', sizeof(beams));
    int split_count = 0;
    int line_count = 0;
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        } else {
            printf("%3d - ", line_count++);
            beams[strlen(line)] = '\0';
        }

        for (size_t i = 0; i < strlen(line); i++) {
            if (line[i] == 'S') {
                beams[i] = '|';
                continue;
            }

            if (line[i] == '^' && beams[i] == '|') {
                beams[i - 1] = '|';
                beams[i] = '.';
                beams[i + 1] = '|';
                split_count++;
            }
        }
        printf("'%s'\n", beams);
    }
    printf("total splits = %d\n", split_count);

    fclose(fp);
    return 0;
}
int part_two() {
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

    long path_counts[MAX_COLS] = {0};
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        for (size_t i = 0; i < strlen(line); i++) {
            if (line[i] == 'S') {
                path_counts[i] = 1;
                continue;
            }

            if (line[i] == '^') {
                path_counts[i - 1] += path_counts[i];
                path_counts[i + 1] += path_counts[i];
                path_counts[i] = 0;
            }
        }
        for (size_t i = 0; i < strlen(line); i++) {
            printf("%3ld ", path_counts[i]);
        }
        printf("\n");
    }

    long total = 0;
    for (size_t i = 0; i < MAX_COLS; i++) {
        total += path_counts[i];
    }
    printf("total splits = %ld\n", total);

    fclose(fp);
    return 0;
}
int main() { return part_two(); }
