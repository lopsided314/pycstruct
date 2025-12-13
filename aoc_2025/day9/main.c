#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum UDLR_t { Unset = 0, Up = 1, Down = 2, Left = 4, Right = 8 } UDLR;

typedef struct Corner_t {
    int x, y;
    UDLR open_directions;
} Corner;

typedef struct Edge_t {
    Corner *t1, *t2;
} Edge;

// typedef struct Span_t {
//     int start, stop;
// } Span;

long tile2_area(Corner *t1, Corner *t2) {
    return (1 + labs(t1->x - t2->x)) * (1 + labs(t1->y - t2->y));
}

// int on_edge(int x, int y, const Edge *edge) {
//     if (x == edge->t1->x && x == edge->t2->x) {
//         if (edge->t1->y > edge->t2->y) {
//             return (y <= edge->t1->y && y >= edge->t2->y);
//         } else {
//             return (y >= edge->t1->y && y <= edge->t2->y);
//         }
//     } else if (y == edge->t1->y && y == edge->t2->y) {
//         if (edge->t1->x > edge->t2->x) {
//             return (x <= edge->t1->x && x >= edge->t2->x);
//         } else {
//             return (x >= edge->t1->x && x <= edge->t2->x);
//         }
//     }
//     return 0;
// }

int edge_intersect(Edge *edge, Corner *c1, Corner *c2) {
    // if the edge and intersect line are in the same
    // direction, doesn't count as intersection...?
    int edge_vert = (edge->t1->x == edge->t2->x);
    int edge_horz = (edge->t1->y == edge->t2->y);
    int intersect_vert = (c1->x == c2->x);
    int intersect_horz = (c1->y == c2->y);

    if ((edge_vert && intersect_vert) || (edge_horz && intersect_horz)) {
        return 0;
    }

    if (edge_vert) {
        int in_vert_span = (c1->y > edge->t1->y && c1->y < edge->t2->y) || (c1->y < edge->t1->y && c1->y > edge->t2->y);
        if (!in_vert_span) {
            return 0;
        }
        int intersect_start, intersect_stop;
        if ((c2->x > edge->t1->x && c1->x < edge->t1->x) || (c1->x > edge->t1->x && c2->x < edge->t1->x)) {

        }
    }

    return 0;
}

#define MAX_TILES 500
int n_tiles = 0;
Corner corners[MAX_TILES] = {0};
Edge edges[MAX_TILES] = {0};
//
#define ARRAY_SIZE 100000
// // Span spans[ARRAY_SIZE] = {0};

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

    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        }
        // printf("%s\n", line);

        sscanf(line, "%d,%d", &corners[n_tiles].x, &corners[n_tiles].y);
        n_tiles++;
    }
    fclose(fp);

    for (int iCorner = 0; iCorner < n_tiles - 1; iCorner++) {
        edges[iCorner].t1 = &corners[iCorner];
        edges[iCorner].t2 = &corners[iCorner + 1];
    }
    edges[n_tiles - 1].t1 = &corners[n_tiles - 1];
    edges[n_tiles - 1].t2 = &corners[0];

    Corner *seed = NULL;
    int iSeedCorner;
    for (int y = 0; y < ARRAY_SIZE; y++) {
        for (int x = 0; x < ARRAY_SIZE; x++) {
            for (int iCorner = 0; iCorner < n_tiles; iCorner++) {
                if (corners[iCorner].x == x && corners[iCorner].y == y) {
                    seed = &corners[iCorner];
                    seed->open_directions = Down | Right;
                    iSeedCorner = iCorner;
                    printf("Seed = (%d %d)\n", x, y);
                }
            }
            if (seed) {
                break;
            }
        }
        if (seed) {
            break;
        }
    }

    long area;
    long max_area = 0;
    for (int i = 0; i < n_tiles; i++) {
        for (int j = i + 1; j < n_tiles; j++) {

            // make sure all the edges of the rectangle formed by
            // the rectangle are

            area = tile2_area(&corners[i], &corners[j]);
            printf("(%d, %d) x (%d, %d) = %ld\n", corners[i].x, corners[i].y, corners[j].x,
                   corners[j].y, area);
            if (area > max_area)
                max_area = area;
        }
    }
    printf("Max area = %ld\n", max_area);

    return 0;
}
