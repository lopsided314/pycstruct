#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Range {
    long start;
    long stop;
};

struct Range ranges[200] = {0};
long available_ids[2000] = {0};

int n_ranges = 0;
int n_IDs = 0;

int part_one() {
    int count = 0;

    for (int iID = 0; iID < n_IDs; iID++) {
        for (int iRange = 0; iRange < n_ranges; iRange++) {
            if (available_ids[iID] <= ranges[iRange].stop &&
                available_ids[iID] >= ranges[iRange].start) {
                count++;
                break;
            }
        }
    }
    return count;
}

long part_two() {
    long count = 0;


    // combine as many ranges as possible
    int n_changed = 1;
    while (n_changed > 0) {
        n_changed = 0;
        for (int i = 0; i < n_ranges; i++) {
            for (int j = 0; j < n_ranges; j++) {
                if (i == j)
                    continue;
                if (ranges[i].start == 0 || ranges[j].start == 0)
                    continue;

                if (ranges[j].start >= ranges[i].start && ranges[j].start <= ranges[i].stop) {
                    if (ranges[i].stop < ranges[j].stop) {
                        // printf("%ld - %ld --> %ld - %ld\n", ranges[i].start, ranges[i].stop, ranges[i].start, ranges[j].stop);
                        ranges[i].stop = ranges[j].stop;
                        n_changed++;
                    }
                    ranges[j].start = 0;
                    ranges[j].stop = 0;
                }
            }
        }
    }

    int new_n_range = 0;
    for (int i = 0; i < n_ranges; i++) {
        if (ranges[i].start) {
            // printf("%ld-%ld\n", ranges[i].start, ranges[i].stop);
            new_n_range++;
            count += (ranges[i].stop - ranges[i].start) + 1;
        }
    }
    printf("%d %d\n", n_ranges, new_n_range);
    return count;
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

    enum what { Ranges, IDs } what = Ranges;

    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            what = IDs;
        }

        if (what == Ranges) {
            sscanf(line, "%ld-%ld", &ranges[n_ranges].start, &ranges[n_ranges].stop);
            printf("range %ld - %ld\n", ranges[n_ranges].start, ranges[n_ranges].stop);
            n_ranges++;
        } else {
            sscanf(line, "%ld", &available_ids[n_IDs]);
            printf("id %ld\n", available_ids[n_IDs]);
            n_IDs++;
        }
    }

    fclose(fp);

    // printf("total = %d\n", part_one());
    printf("total = %ld\n", part_two());
}
