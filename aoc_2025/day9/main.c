#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_TILES 500
#define ARRAY_MAX 100000

typedef struct Corner_t {
    int x0, y0;
    int x1, y1;
    enum Directions { Unset = 0, X_POS = 1, X_NEG = 2, Y_POS = 4, Y_NEG = 8 } dir;
} Corner;

int in_bounds(int val, int b1, int b2) {
    return (b1 >= val && b2 <= val) || (b1 <= val && b2 >= val);
}

long corner2_area(Corner *c1, Corner *c2) {
    return (1 + labs(c1->x0 - c2->x0)) * (1 + labs(c1->y0 - c2->y0));
}

int corner2_contains(Corner *c1, Corner *c2, Corner *t) {
    // check if the test point is inside the rectangle
    int inside_x = (c1->x0 > t->x0 && c2->x0 < t->x0) || (c1->x0 < t->x0 && c2->x0 > t->x0);
    int inside_y = (c1->y0 > t->y0 && c2->y0 < t->y0) || (c1->y0 < t->y0 && c2->y0 > t->y0);

    if (inside_x && inside_y) {
        return 1;
    }

    // check if the arms of the test point intersect
    // with the rectangle
    else if (inside_x) {
        if (t->y0 > c1->y0 && t->y0 > c2->y0) {
            if (t->y1 < c1->y0 || t->y1 < c2->y0) {
                return 1;
            }
        } else {
            if (t->y1 > c1->y0 || t->y1 > c2->y0) {
                return 1;
            }
        }
    } else if (inside_y) {
        if (t->x0 > c1->x0 && t->x0 > c2->x0) {
            if (t->x1 < c1->x0 || t->x1 < c2->x0) {
                return 1;
            }
        } else {
            if (t->x1 > c1->x0 || t->x1 > c2->x0) {
                return 1;
            }
        }
    }
    return 0;
}

int n_tiles = 0;

Corner corners[MAX_TILES] = {0};

void make_edges() {
    // connect all the corners with corners
    for (int i = 0; i < n_tiles; i++) {
        int i_next = (i == n_tiles - 1) ? 0 : i + 1;
        int i_prev = (i == 0) ? n_tiles - 1 : i - 1;

        if (corners[i].x0 == corners[i_next].x0) {
            corners[i].y1 = corners[i_next].y0;
            corners[i].x1 = corners[i_prev].x0;
        } else {
            corners[i].x1 = corners[i_next].x0;
            corners[i].y1 = corners[i_prev].y0;
        }
    }

    for (int i = 0; i < n_tiles; i++) {
        int x0 = corners[i].x0;
        int y0 = corners[i].y0;
        for (int j = 0; j < n_tiles; j++) {
            if (i == j)
                continue;

            if (in_bounds(x0, corners[j].x0, corners[j].x1)) {
                if (y0 < corners[j].y0) {
                    corners[i].dir |= Y_POS;
                } else if (y0 > corners[j].y0) {
                    corners[i].dir |= Y_NEG;
                }
            }
            if (in_bounds(y0, corners[j].y0, corners[j].y1)) {
                if (x0 < corners[j].x0) {
                    corners[i].dir |= X_POS;
                } else if (x0 > corners[j].x0) {
                    corners[i].dir |= X_NEG;
                }
            }
        }
    }
}

int main() {

    char delim = '\n';
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    // FILE *fp = fopen("sample.txt", "r");
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        }
        // printf("%s\n", line);

        sscanf(line, "%d,%d", &corners[n_tiles].x0, &corners[n_tiles].y0);
        n_tiles++;
    }
    fclose(fp);

    make_edges();

    long area;
    long max_area = 0;
    for (int i = 0; i < n_tiles; i++) {
        for (int j = i + 1; j < n_tiles; j++) {
            printf("\n");
            printf("Checking (%d, %d) x (%d, %d):\n", corners[i].x0, corners[i].y0, corners[j].x0,
                   corners[j].y0);

            // check that the corners point at each other
            int ok;
            if (corners[i].x0 < corners[j].x0) {
                ok = (corners[i].dir & X_POS) && (corners[j].dir & X_NEG);
            } else {
                ok = (corners[i].dir & X_NEG) && (corners[j].dir & X_POS);
            }
            if (!ok) {
                printf("  skipping, x dir\n");
                continue;
            }

            if (corners[i].y0 < corners[j].y0) {
                ok = (corners[i].dir & Y_POS) && (corners[j].dir & Y_NEG);
            } else {
                ok = (corners[i].dir & Y_NEG) && (corners[j].dir & Y_POS);
            }
            if (!ok) {
                printf("  skipping, y dir\n");
                continue;
            }

            // check that the rectangle defined by the corners
            // doesn't contain any other corners
            int invalid = 0;
            for (int n = 0; n < n_tiles; n++) {
                if (n == i || n == j) {
                    continue;
                }
                if (corner2_contains(&corners[i], &corners[j], &corners[n])) {
                    printf("  skipping, contains (%d, %d)\n", corners[n].x0, corners[n].y0);
                    invalid = 1;
                    break;
                }
            }
            if (invalid) {
                continue;
            }

            area = corner2_area(&corners[i], &corners[j]);
            printf("(%d, %d) x (%d, %d) = %ld\n", corners[i].x0, corners[i].y0, corners[j].x0,
                   corners[j].y0, area);
            if (area > max_area)
                max_area = area;
        }
    }
    printf("Max area = %ld\n", max_area);

    return 0;
}
