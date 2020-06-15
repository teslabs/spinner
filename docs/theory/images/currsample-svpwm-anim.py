import argparse

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


parser = argparse.ArgumentParser()
parser.add_argument("-o", "--output", help="Output file")
parser.add_argument("-s", "--sector", type=int, default=1, help="Sector")
args = parser.parse_args()


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


fig, axes = plt.subplots(nrows=7, sharex=True, figsize=(8, 6))

axes[0].set_ylim([0, 1])

for axis in axes[1:]:
    axis.set_xlim([0, 1])
    axis.set_ylim([-0.1, 1.1])
    axis.set_yticks([])
    axis.set_xticks([0, 0.5, 1])

axes[1].set_ylabel(r"$q_a$")
axes[2].set_ylabel(r"$\bar{q}_a$")
axes[3].set_ylabel(r"$q_b$")
axes[4].set_ylabel(r"$\bar{q}_b$")
axes[5].set_ylabel(r"$q_c$")
axes[6].set_ylabel(r"$\bar{q}_c$")

axes[6].set_xlabel("Regulation periods")

N = 200
t = np.linspace(0, 1, N)

# plot timer ramp
trig = np.zeros(N)
trig[: N // 2] = 0 + 2 * t[: N // 2]
trig[N // 2 :] = 2 - 2 * t[N // 2 :]
axes[0].plot(t, trig)

(da_line,) = axes[0].plot((0, 0), (0, 0), "r-")
(db_line,) = axes[0].plot((0, 0), (0, 0), "g-")
(dc_line,) = axes[0].plot((0, 0), (0, 0), "b-")

(qa_H_line,) = axes[1].step([], [], "r")
(qa_L_line,) = axes[2].step([], [], "r-")
(qb_H_line,) = axes[3].step([], [], "g")
(qb_L_line,) = axes[4].step([], [], "g")
(qc_H_line,) = axes[5].step([], [], "b")
(qc_L_line,) = axes[6].step([], [], "b")

qa_H_text = axes[1].text(0, 0, "")
qa_L_text = axes[2].text(0, 0, "")
qb_H_text = axes[3].text(0, 0, "")
qb_L_text = axes[4].text(0, 0, "")
qc_H_text = axes[5].text(0, 0, "")
qc_L_text = axes[6].text(0, 0, "")


def update(angle):
    """Update plot with new angle."""
    da, db, dc = calc(args.sector, angle)

    # adjust trigger lines
    da_line.set_data((0, 1), (da, da))
    db_line.set_data((0, 1), (db, db))
    dc_line.set_data((0, 1), (dc, dc))

    qa = np.zeros(N)
    qa[np.where(da > trig)] = 1

    qb = np.zeros(N)
    qb[np.where(db > trig)] = 1

    qc = np.zeros(N)
    qc[np.where(dc > trig)] = 1

    qa_H_line.set_data(t, qa)
    qa_L_line.set_data(t, 1 - qa)

    qb_H_line.set_data(t, qb)
    qb_L_line.set_data(t, 1 - qb)

    qc_H_line.set_data(t, qc)
    qc_L_line.set_data(t, 1 - qc)

    qa_H_text.set_text("{:.2f} %".format(100 * da))
    qa_L_text.set_text("{:.2f} %".format(100 * (1 - da)))
    qb_H_text.set_text("{:.2f} %".format(100 * db))
    qb_L_text.set_text("{:.2f} %".format(100 * (1 - db)))
    qc_H_text.set_text("{:.2f} %".format(100 * dc))
    qc_L_text.set_text("{:.2f} %".format(100 * (1 - dc)))

    return [
        da_line,
        db_line,
        dc_line,
        qa_H_line,
        qa_L_line,
        qb_H_line,
        qb_L_line,
        qc_H_line,
        qc_L_line,
        qa_H_text,
        qa_L_text,
        qb_H_text,
        qb_L_text,
        qc_H_text,
        qc_L_text,
    ]


ani = FuncAnimation(
    fig,
    update,
    interval=50,
    frames=np.arange(60 * (args.sector - 1), 60 * args.sector, 0.5),
    blit=True,
)


if args.output:
    ani.save(args.output, fps=25, writer="imagemagick")
else:
    plt.show()
