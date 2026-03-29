sln = [3, 5, 4, 7]
N = len(sln)
counts = [0] * N


class Button:
    def __init__(self, bits: list[int]) -> None:
        self.arr = [0] * N
        for b in bits:
            self.arr[b] = 1

    def mul(self, i):
        return [i * x for x in self.arr]


def sum(buttons: list[Button], coeffs: list[int]):
    sum = [0] * N
    for b, c in zip(buttons, coeffs):
        res = b.mul(c)
        for i in range(N):
            sum[i] += res[i]

    print(sum)


coeffs = [1, 3, 0, 3, 1, 2]
buttons = [
    Button([3]),
    Button([1, 3]),
    Button([2]),
    Button([2, 3]),
    Button([0, 2]),
    Button([0, 1]),
]
sum(buttons, coeffs)


