#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CHOOSE 10
static int combo_ints[MAX_CHOOSE] = {0};
// return 1 if there are more combinations, 0 if all
// have been generated
int combo_generator(int N, int r) {
    static int counters[MAX_CHOOSE] = {-1};
    if (counters[0] == -1) {
        for (int i = 0; i < r; i++) {
            counters[i] = i;
        }
    }

    if (counters[0] >= N) {
        for (int i = 0; i < MAX_CHOOSE; i++) {
            counters[i] = -1;
            combo_ints[i] = -1;
        }
        return 0;
    }

    for (int i = 1; i < r - 1; i++) {
        if (counters[i] >= N) {
            counters[i - 1]++;
            for (int j = i; j < r; j++) {
                counters[j] = counters[j - 1] + 1;
            }
        }
    }

    if (counters[r - 1] >= N) {
        counters[r - 2]++;
        counters[r - 1] = counters[r - 2] + 1;
        return combo_generator(N, r);
    }
    counters[r - 1]++;

    for (int i = 0; i < MAX_CHOOSE; i++) {
        combo_ints[i] = counters[i];
    }
    combo_ints[r - 1]--;

    return 1;
}

//
// Notes:
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
//
#define MAX_LIGHTS 10
#define MAX_BUTTONS 15

int run_machine(char *machine_def) {
    printf("Machine = %s\n", machine_def);

    int lights = 0;
    int buttons[MAX_BUTTONS] = {0};
    // int joltages[MAX_LIGHTS] = {0};
    int n_lights = 0, n_buttons = 0;

    char *word, *sword, *word_end, *sword_end;

    word = strtok_r(machine_def, " ", &word_end);
    for (size_t i = 1; i < strlen(word) - 1; i++) {
        if (word[i] == '#') {
            lights += (1 << (i - 1));
        }
        n_lights++;
    }
    printf("lights = 0x%x\n", lights);
    while (1) {

        word = strtok_r(NULL, " ", &word_end);
        if (word == NULL) {
            break;
        } else if (word[0] == '(') {
            sword = strtok_r(word + 1, ",", &sword_end);
            // printf("b = %d\n", atoi(sb));
            buttons[n_buttons] += (1 << atoi(sword));
            while (1) {
                sword = strtok_r(NULL, ",", &sword_end);
                if (sword == NULL) {
                    break;
                }
                // printf("b = %d\n", atoi(sb));
                buttons[n_buttons] += (1 << atoi(sword));
            }
            // printf("buton %d = 0x%x\n", n_buttons, buttons[n_buttons]);
            n_buttons++;
        } else if (word[0] == '{') {
            // part 2...
            continue;
        }
    }

    for (int r = 1; r < n_buttons; r++) {
        while (combo_generator(n_buttons, r)) {

            int light_sum = 0;
            for (int i = 0; i < r; i++) {
                light_sum ^= buttons[combo_ints[i]];
                printf("%d-%x %x ", combo_ints[i], buttons[combo_ints[i]], light_sum);
                if (light_sum == lights) {
                    printf("Equal with %d\n", i+1);
                    return i+1;
                }
            }
            printf("\n");
        }
    }
    printf("Failed to find sequence\n");
    return 0;
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

    long total = 0;
    while ((read = getdelim(&line, &len, delim, fp)) != -1) {
        if (line[strlen(line) - 1] == delim)
            line[strlen(line) - 1] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        total += run_machine(line);
    }

    printf("Total = %ld\n", total);
    fclose(fp);
    return 0;
}
