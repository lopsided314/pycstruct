import matplotlib.pyplot as plt

with open("./input.txt") as f:
    text = f.read()

x, y = [], []

for l in text.split("\n"):
    if len(l) == 0:
        continue

    xs, ys = l.split(",")

    x.append(int(xs))
    y.append(int(ys))


plt.plot(x, y, "-o")
plt.plot(x[0], y[0], "ro")

try:
    plt.show()
except KeyboardInterrupt:
    plt.close("all")
