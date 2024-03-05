import numpy as np
import matplotlib.pyplot as plt


def calc(sector, angle):
    alpha = np.deg2rad(angle)

    # amplitude assumed sqrt(3) / 2 (maximum)
    a = np.sqrt(3) / 2 * np.cos(alpha) - 1 / 2 * np.sin(alpha)
    b = np.sin(alpha)
    c = -(a + b)

    if sector == 1:
        x = a
        y = b
        z = 1 - (x + y)

        da = x + y + 0.5 * z
        db = y + 0.5 * z
        dc = 0.5 * z
    elif sector == 2:
        x = -c
        y = -a
        z = 1 - (x + y)

        da = x + 0.5 * z
        db = x + y + 0.5 * z
        dc = 0.5 * z
    elif sector == 3:
        x = b
        y = c
        z = 1 - (x + y)

        da = 0.5 * z
        db = x + y + 0.5 * z
        dc = y + 0.5 * z
    elif sector == 4:
        x = -a
        y = -b
        z = 1 - (x + y)

        da = 0.5 * z
        db = x + 0.5 * z
        dc = x + y + 0.5 * z
    elif sector == 5:
        x = c
        y = a
        z = 1 - (x + y)

        da = y + 0.5 * z
        db = 0.5 * z
        dc = x + y + 0.5 * z
    elif sector == 6:
        x = -b
        y = -c
        z = 1 - (x + y)

        da = x + y + 0.5 * z
        db = 0.5 * z
        dc = x + 0.5 * z

    return da, db, dc


fig, ax = plt.subplots()

ax.set_xlabel("State space vector angle (deg)")
ax.set_ylabel("Duty cycle")

alpha = np.array([])
da = np.array([])
db = np.array([])
dc = np.array([])

for sector in range(1, 7):
    alpha_ = np.arange(60 * (sector - 1), 60 * sector, 1)
    da_, db_, dc_ = calc(sector, alpha_)

    alpha = np.append(alpha, alpha_)
    da = np.append(da, da_)
    db = np.append(db, db_)
    dc = np.append(dc, dc_)

ax.plot(alpha, da, color="r", label=r"$d_a$")
ax.plot(alpha, db, color="g", label=r"$d_b$")
ax.plot(alpha, dc, color="b", label=r"$d_c$")

ax.legend(loc="upper right")

plt.show()
