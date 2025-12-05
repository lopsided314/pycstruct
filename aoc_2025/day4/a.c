#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char arr[200][200] = {0};

int is_paper(int row, int col) { return arr[row][col] == '@' || arr[row][col] == 'X'; }

int part_one() {
    int roll_cutoff = 4;
    int forklift_accessible_roll_count = 0;
    int n_col = strlen(arr[0]);
    int n_row = 0;
    for (; arr[n_row][0] != '\0'; n_row++) {
    }

    // printf("n_col = %d, n_row = %d\n", n_col, n_row);
    int paper_count;

    // do all the locations not on the edges
    for (int row = 1; row < n_row - 1; row++) {
        for (int col = 1; col < n_col - 1; col++) {
            if (is_paper(row, col)) {
                paper_count = -1;
                for (int irow = -1; irow <= 1; irow++) {
                    for (int icol = -1; icol <= 1; icol++) {
                        if (is_paper(row + irow, col + icol)) {
                            paper_count++;
                        }
                    }
                }
                if (paper_count < roll_cutoff) {
                    arr[row][col] = 'X';
                    printf("row %d col %d = %d\n", row, col, paper_count);
                    forklift_accessible_roll_count++;
                }
            }
        }
    }

    // check all the edges
    for (int row = 0; row < n_row; row++) {
        // top and bottom rows
        if (row == 0) {
            // corners
            if (arr[row][0] == '@') {
                paper_count = 0;
                paper_count += is_paper(0, 1);
                paper_count += is_paper(1, 1);
                paper_count += is_paper(1, 0);
                if (paper_count < roll_cutoff) {
                    printf("row %d col %d = %d\n", row, 0, paper_count);
                    arr[row][0] = 'X';
                    forklift_accessible_roll_count++;
                }
            }
            if (arr[row][n_col - 1] == '@') {
                paper_count = 0;
                paper_count += is_paper(0, n_col - 2);
                paper_count += is_paper(1, n_col - 2);
                paper_count += is_paper(1, n_col - 1);
                if (paper_count < roll_cutoff) {
                    printf("row %d col %d = %d\n", row, n_col - 1, paper_count);
                    arr[row][n_col - 1] = 'X';
                    forklift_accessible_roll_count++;
                }
            }
            for (int col = 1; col < n_col - 1; col++) {
                if (arr[row][col] == '@') {
                    paper_count = -1;
                    for (int irow = 0; irow <= 1; irow++) {
                        for (int icol = -1; icol <= 1; icol++) {
                            if (is_paper(row + irow, col + icol))
                                paper_count++;
                        }
                    }
                    if (paper_count < roll_cutoff) {
                        printf("row %d col %d = %d\n", row, col, paper_count);
                        arr[row][col] = 'X';
                        forklift_accessible_roll_count++;
                    }
                }
            }
        } else if (row == n_row - 1) {
            if (arr[n_row - 1][0] == '@') {
                paper_count = 0;
                paper_count += is_paper(n_row - 2, 0);
                paper_count += is_paper(n_row - 2, 1);
                paper_count += is_paper(n_row - 1, 1);
                if (paper_count < roll_cutoff) {
                    printf("row %d col %d = %d\n", row, 0, paper_count);
                    arr[row][0] = 'X';
                    forklift_accessible_roll_count++;
                }
            }
            if (arr[n_row - 1][n_col - 1] == '@') {
                paper_count = 0;
                paper_count += is_paper(n_row - 2, n_col - 0);
                paper_count += is_paper(n_row - 2, n_col - 1);
                paper_count += is_paper(n_row - 1, n_col - 1);
                if (paper_count < roll_cutoff) {
                    printf("row %d col %d = %d\n", row, n_col - 1, paper_count);
                    arr[row][n_col - 1] = 'X';
                    forklift_accessible_roll_count++;
                }
            }

            for (int col = 1; col < n_col - 1; col++) {
                if (arr[n_row - 1][col] == '@') {
                    paper_count = -1;
                    for (int irow = -1; irow <= 0; irow++) {
                        for (int icol = -1; icol <= 1; icol++) {
                            if (is_paper(row + irow, col + icol)) {
                                paper_count++;
                            }
                        }
                    }
                    if (paper_count < roll_cutoff) {
                        printf("row %d col %d = %d\n", n_row - 1, col, paper_count);
                        arr[n_row - 1][col] = 'X';
                        forklift_accessible_roll_count++;
                    }
                }
            }
        } else {
            // edges of middle rows
            if (arr[row][0] == '@') {
                paper_count = -1;
                for (int irow = -1; irow <= 1; irow++) {
                    for (int icol = 0; icol <= 1; icol++) {
                        if (is_paper(row + irow, 0 + icol)) {
                            paper_count++;
                        }
                    }
                }
                if (paper_count < roll_cutoff) {
                    printf("row %d col %d = %d\n", row, 0, paper_count);
                    arr[row][0] = 'X';
                    forklift_accessible_roll_count++;
                }
            }
            if (arr[row][n_col - 1] == '@') {
                paper_count = -1;
                for (int irow = -1; irow <= 1; irow++) {
                    for (int icol = -1; icol <= 0; icol++) {
                        if (is_paper(row + irow, n_col - 1 + icol)) {
                            paper_count++;
                        }
                    }
                }
                if (paper_count < roll_cutoff) {
                    printf("row %d col %d = %d\n", row, n_col - 1, paper_count);
                    arr[row][n_col - 1] = 'X';
                    forklift_accessible_roll_count++;
                }
            }
        }
    }
    printf("Total = %d\n", forklift_accessible_roll_count);
    return forklift_accessible_roll_count;
}

int clear_rolls() {
    int count = 0;

    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            if (arr[i][j] == 'X') {
                count++;
                arr[i][j] = '.';
                // printf("clear row %d col %d\n", i, j);
            }
        }
    }
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

    size_t i = 0;
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        strcpy(arr[i++], line);
    }

    fclose(fp);

    int total = part_one();
    while (clear_rolls()) {
        total += part_one();
    }
    printf("Actual total = %d\n", total);
}
