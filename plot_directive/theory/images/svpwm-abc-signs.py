import numpy as np
import matplotlib.pyplot as plt


fig, ax = plt.subplots()

ax.set_xlabel("State space vector angle (deg)")
ax.set_ylabel(r"$a, b, c$")

alphad = np.arange(0, 360, 1)
alpha = np.deg2rad(alphad)

# amplitude assumed sqrt(3) / 2 (maximum)
a = np.sqrt(3) / 2 * np.cos(alpha) - 1 / 2 * np.sin(alpha)
b = np.sin(alpha)
c = -(a + b)

ax.plot(alphad, a, color="r", label="a")
ax.plot(alphad, b, color="g", label="b")
ax.plot(alphad, c, color="b", label="c")

ax.set_xticks([0, 60, 120, 180, 240, 300, 360])
ax.set_xlim([0, 360])

ax.set_yticks([-1, 0, 1])
ax.set_ylim([-1, 1])

ax.axvline(0, ls="--")
ax.axvline(60, ls="--")
ax.axvline(120, ls="--")
ax.axvline(180, ls="--")
ax.axvline(240, ls="--")
ax.axvline(300, ls="--")
ax.axvline(360, ls="--")

ax.legend(loc="upper right")

plt.show()
