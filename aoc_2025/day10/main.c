#include <stdio.h>
#include <string.h>
#include <unistd.h>

//
// Notes:
//   the order of the button presses doesn't matter and can be
//   simulated with XOR operations
//
//   the result of the button press sequence is just XOR of all
//   the wiring diagrams for those buttons
//
//   a ^ a == 0
//   a ^ b ^ a == b
//
//   any sequence of button presses can only have each button
//   1 or 0 times, any more cancel each other out. This means
//   the maximum sequence length is the number of buttons
//
//   possibilities for 1 button press:
//     each button once - check the button wiring diagram bit pattern
//     against the solution
//   possibililties for 2 button presses:
//     0^1, 0^2, ..., 0^N, 1^2, 1^3, ..., 1^N, ..., N-1^N (itertools.combinations(..., 2))
//   possibililties for 3 button presses: itertools.combinations(..., 3)
//   etc.
//

#define MAX_CHOOSE 10

int combo_ints[MAX_CHOOSE] = {0};
void init_combo_generator(int N, int r) {
    (void)N;
    for (int i = 0; i < r; i++) {
        combo_ints[i] = i;
    }
}
// return 1 if there are more combinations, 0 if all
// have been generated
int combo_generator(int N, int r) {

    if (combo_ints[0] >= N) {
        for (int i = 0; i < MAX_CHOOSE; i++) {
            combo_ints[i] = -1;
        }
        return 0;
    }

    for (int i = 1; i < r - 1; i++) {
        if (combo_ints[i] >= N) {
            combo_ints[i - 1]++;
            for (int j = i; j < r; j++) {
                combo_ints[j] = combo_ints[j - 1] + 1;
            }
        }
    }
    if (combo_ints[r - 1] >= N) {
        combo_ints[r - 2]++;
        combo_ints[r - 1] = combo_ints[r - 2] + 1;
        return combo_generator(N, r);
    }
    combo_ints[r - 1]++;
    return 1;


    while (1) {
        if (combo_ints[0] >= N) {
            printf("done\n");
            return 0;
        }

        for (int i = 1; i < r - 1; i++) {
            if (combo_ints[i] >= N) {
                combo_ints[i - 1]++;
                for (int j = i; j < r; j++) {
                    combo_ints[j] = combo_ints[j - 1] + 1;
                }
            }
        }
        if (combo_ints[r - 1] >= N) {
            combo_ints[r - 2]++;
            combo_ints[r - 1] = combo_ints[r - 2] + 1;
            continue;
        }


        printf("(");
        for (int i = 0; i < r; i++) {
            printf("%d ", combo_ints[i]);
        }
        printf(")\n");

        combo_ints[r - 1]++;
    }
    return 0;
}

int run_machine(const char *machine_definitions) { return 0; }

int main() {

    int N = 5, r = 3;
    init_combo_generator(N, r);
    while (combo_generator(N, r)) {
        printf("(");
        for (int i = 0; i < r; i++) {
            printf("%d ", combo_ints[i]);
        }
        printf(")\n");
    }

    printf("\n");
    return 0;

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

        run_machine(line);
    }

    fclose(fp);
    return 0;
}
