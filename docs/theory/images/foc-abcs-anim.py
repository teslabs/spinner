import argparse

import matplotlib.pyplot as plt
import matplotlib.lines as lines
import matplotlib.patches as mpatches
from matplotlib.animation import FuncAnimation
import numpy as np


def circle(ax, radius):
    ax.add_artist(mpatches.Circle((0, 0), radius, fill=False, alpha=0.3))


def vector(color):
    return plt.quiver(0, 0, 0, 0, color=color, angles="xy", scale_units="xy", scale=1)


# unit vectors
a_un = np.exp(1j * 0)
b_un = np.exp(1j * 2 * np.pi / 3)
c_un = np.exp(1j * 4 * np.pi / 3)

fig, ax = plt.subplots()
ax.set_aspect("equal")
ax.set_xlim([-1.75, 1.75])
ax.set_ylim([-1.75, 1.75])
ax.set_xticks([1, 1.5])
ax.set_yticks([1, 1.5])

# center axes (using left+bottom only)
ax.spines["left"].set_position("center")
ax.spines["bottom"].set_position("center")
ax.spines["right"].set_color("none")
ax.spines["top"].set_color("none")

ax.xaxis.set_ticks_position("bottom")
ax.yaxis.set_ticks_position("left")

title = ax.text(0.95, 0.95, "", transform=ax.transAxes, ha="right")

# enclosing circles
circle(ax, 1)
circle(ax, 3 / 2)

# vectors (u, v, w, beta, alpha, space-vector)
a_line = vector("red")
a_line_label = ax.text(0, 0, r"$\vec{i_a}$", color="red")

b_line = vector("green")
b_line_label = ax.text(0, 0, r"$\vec{i_b}$", color="green")

c_line = vector("blue")
c_line_label = ax.text(0, 0, r"$\vec{i_c}$", color="blue")

i_s_line = vector("black")
i_s_line_label = ax.text(0, 0, r"$\vec{i_s}$", color="black")


def update(angle):
    """Update plot with new angle."""

    i_a = np.cos(angle)
    i_b = np.cos(angle - 2 * np.pi / 3)
    i_c = np.cos(angle - 4 * np.pi / 3)

    title.set_text("Rotor angle: {:.0f} deg".format(np.rad2deg(angle)))

    a = a_un * i_a
    a_line.set_UVC(np.real(a), np.imag(a))
    a_line_label.set_position((np.real(a), np.imag(a)))

    b = b_un * i_b
    b_line.set_UVC(np.real(b), np.imag(b))
    b_line_label.set_position((np.real(b), np.imag(b)))

    c = c_un * i_c
    c_line.set_UVC(np.real(c), np.imag(c))
    c_line_label.set_position((1.1 * np.real(c), np.imag(c)))

    i_s = a + b + c
    i_s_line.set_UVC(np.real(i_s), np.imag(i_s))
    i_s_line_label.set_position((np.real(i_s), np.imag(i_s)))

    return [
        title,
        a_line,
        a_line_label,
        b_line,
        b_line_label,
        c_line,
        c_line_label,
        i_s_line,
        i_s_line_label,
    ]


ani = FuncAnimation(
    fig, update, interval=50, frames=np.arange(0, 2 * np.pi, 2 * np.pi / 180), blit=True
)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", help="Output file")
    args = parser.parse_args()

    if args.output:
        ani.save(args.output, fps=25, writer="imagemagick")
    else:
        plt.show()
