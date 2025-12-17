import itertools


def stdlib_combinations(N, r):
    print("\n".join([str(v) for v in itertools.combinations(range(N), r)]))


def for_loops_3(N):
    for i in range(N):
        for j in range(i + 1, N):
            for k in range(j + 1, N):
                print(f"{i, j, k}")


def for_loops_4(N):
    if N >= 4:
        print(tuple(i for i in range(4)))
        return
    for i in range(N):
        for j in range(i + 1, N):
            for k in range(j + 1, N):
                for l in range(k + 1, N):
                    print(f"{i, j, k, l}")


def loop(N, r):

    ijk = list(range(r))
    while True:
        if ijk[0] >= N:
            break

        for i in range(1, r - 1):
            if ijk[i] >= N:
                ijk[i - 1] += 1
                for j in range(i, r):
                    ijk[j] = ijk[j - 1] + 1

        if ijk[-1] >= N:
            ijk[-2] += 1
            ijk[-1] = ijk[-2] + 1
            continue
        print(f"{tuple(ijk)}")
        ijk[-1] += 1


if __name__ == "__main__":
    N = 5
    r = 3

    loop(N, r)
    print()
    stdlib_combinations(N, r)
    exit()
    print()
    for_loops_4(4)
