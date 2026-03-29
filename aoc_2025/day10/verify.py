with open("output.txt") as f:
    output = f.read().lower()

sum: int = 0
solution: list[int] = []
buttons: list[int] = []
lights: int = 0
mlights: int = 0
machine: str = ""
for line in output.split("\n"):
    if "machine" in line:
        machine = line
        mlights = 0
        for i, c in enumerate(machine[machine.find("[") + 1 : machine.find("]")]):
            if c == "#":
                mlights |= 1 << i
    if "lights" in line:
        buttons = []
        solution = []
        lights = int(line.split(" ")[-1], 16)
        if lights != mlights:
            print(f"Wrong lights for {machine = }")
    if "button" in line:
        buttons.append(int(line.split(" ")[-1], 16))
    if "equal" in line:
        for s in line.split(" "):
            if "-" in s:
                solution.append(int(s[: s.find("-")]))
        if len(solution) != int(line[-1]):
            print("more steps than printed")
            exit()
        sum += len(solution)

        res = 0
        # print(buttons)
        for i in solution:
            res ^= buttons[i]

        if res != lights:
            print(f"error with {machine}")
            print(f"{solution = }")
            print(f"python = {res}, lights = {lights}")
            exit()
        # print(f"solution = {res:x}")


print(f"{sum = }")
