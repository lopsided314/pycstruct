import itertools
from math import sqrt, prod


class JunctionBox:
    def __init__(self, i: int, x: int, y: int, z: int) -> None:
        self.i = i
        self.x = x
        self.y = y
        self.z = z

    def __str__(self):
        return f"({self.x},{self.y},{self.z})"
        # return f"{self.i} ({self.x},{self.y},{self.z})"


class Distance:
    def __init__(self, j1: JunctionBox, j2: JunctionBox) -> None:
        self.j1: JunctionBox = j1
        self.j2: JunctionBox = j2

        self.distance = sqrt(
            (j1.x - j2.x) ** 2 + (j1.y - j2.y) ** 2 + (j1.z - j2.z) ** 2
        )

    def __str__(self) -> str:
        return f"distance {self.j1} {self.j2} = {self.distance:.1f}"


class Circuit:
    def __init__(self, i, j: JunctionBox) -> None:
        self.i: int = i
        self.junctions: list[JunctionBox] = [j]


def main() -> int:

    # with open("sample.txt") as f:
    with open("input.txt") as f:
        text = f.read()

    junctions: list[JunctionBox] = []
    for i, line in enumerate(text.split("\n")):
        if len(line) == 0:
            continue

        x, y, z = [int(v) for v in line.split(",")]
        junctions.append(JunctionBox(i, x, y, z))

    distances: list[Distance] = []
    for i, j in itertools.combinations((j.i for j in junctions), 2):
        distances.append(Distance(junctions[i], junctions[j]))

    distances.sort(key=lambda d: d.distance)

    circuits: list[Circuit] = [Circuit(i, j) for i, j in enumerate(junctions)]

    for d in distances:
        print(f"{d}", end=" -- ")
        c1: Circuit | None = None
        c2: Circuit | None = None

        for c in circuits:
            if d.j1 in c.junctions:
                c1 = c

            if d.j2 in c.junctions:
                c2 = c

            if c1 and c2:
                break

        if not c1 or not c2:
            print(f"junction {d.j1.i} or {d.j2.i} has no circuit")
            return 1

        if c1.i == c2.i:
            print("same circuit")
        else:
            print(f"combining circuits {c1.i} and {c2.i}")
            c1.junctions += c2.junctions
            c1.junctions = list(set(c1.junctions))
            circuits.remove(c2)

        if len(circuits) == 1:
            print(f"Final junction between {d.j1} and {d.j2}")
            print(f"x^2 = {d.j1.x * d.j2.x}")
            break

    if len(circuits) > 1:
        print("Failed to combine all circuits")
        return 1

    # circuits.sort(key=lambda c: len(c.junctions), reverse=True)
    # for circuit in circuits[:3]:
    #     print(f"Circuit has {len(circuit.junctions)} junctions")
    #     for j in circuit.junctions:
    #         print(j)
    #
    #     print()

    # total = prod(len(c.junctions) for c in circuits[:3])
    # print(f"{total = }")
    return 0


if __name__ == "__main__":
    exit(main())
