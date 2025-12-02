#include <stdio.h>
#include <unistd.h>

enum LeftRight { Left, Right };

void spin_dial(int *dial, int *zero_crosses, int turns, enum LeftRight lr) {

    for (; turns > 0; turns--) {
        if (lr == Left) {
            *dial -= 1;
            if (*dial == 0) {
                *zero_crosses += 1;
            } else if (*dial < 0) {
                *dial = 99;
            }
        } else {
            *dial += 1;
            if (*dial == 100) {
                *zero_crosses += 1;
                *dial = 0;
            }
        }
        // printf("turns = %d, dial = %d, z = %d\n", turns, *dial, *zero_crosses);
    }
    printf("dial = %d, z = %d\n", *dial, *zero_crosses);
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

    enum LeftRight lr;
    int turns = 0, dial = 50, zero_count = 0;

    while ((read = getline(&line, &len, fp)) != -1) {
        // printf("line = %s", line);
        if (line[0] == 'L') {
            sscanf(line, "L%i", &turns);
            lr = Left;
        } else if (line[0] == 'R') {
            sscanf(line, "R%i", &turns);
            lr = Right;
        } else {
            continue;
        }
        spin_dial(&dial, &zero_count, turns, lr);
        // printf("dial = %i, crosses = %d\n", dial, zero_count);
    }
    printf("Total: %d\n", zero_count);

    fclose(fp);
}
