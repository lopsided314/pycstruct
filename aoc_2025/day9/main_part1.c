#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Tile_t {
    int x, y;
} Tile;

typedef struct Rectangle_t {
    Tile *t1;
    Tile *t2;
} Rectangle;

long rect_area(Rectangle *rect) {
    return (1 + labs(rect->t1->x - rect->t2->x)) * (1 + labs(rect->t1->y - rect->t2->y));
}

#define MAX_TILES 500
Tile red_tiles[MAX_TILES] = {0};
int n_tiles = 0;

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

    int x, y;
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        sscanf(line, "%d,%d", &x, &y);
        printf("%s\n", line);
        red_tiles[n_tiles].x = x;
        red_tiles[n_tiles].y = y;
        n_tiles++;
    }

    fclose(fp);

    Rectangle rect = {0};
    long area;
    long max_area = 0;
    for (int i = 0; i < n_tiles; i++) {
        for (int j = i + 1; j < n_tiles; j++) {
            rect.t1 = &red_tiles[i];
            rect.t2 = &red_tiles[j];
            area = rect_area(&rect);
            printf("(%d, %d) x (%d, %d) = %ld\n", red_tiles[i].x, red_tiles[i].y, red_tiles[j].x,
                   red_tiles[j].y, area);
            if (area > max_area)
                max_area = area;
        }
    }
    printf("Max area = %ld\n", max_area);

    return 0;
}

int main() { return part_one(); }
