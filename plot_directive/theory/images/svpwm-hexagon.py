import numpy as np
import matplotlib.pyplot as plt
import matplotlib.lines as lines


def v_s(q_c, q_b, q_a):
    return q_a + q_b * np.exp(1j * 2 * np.pi / 3) + q_c * np.exp(1j * 4 * np.pi / 3)


def plot_vector(ax, v, label):
    r, i = np.real(v), np.imag(v)
    ax.quiver(0, 0, r, i, color="k", angles="xy", scale_units="xy", scale=1)
    ha = "left" if r > 0 else "right"
    va = "bottom" if i > 0 else "top"
    ax.annotate(label, (r, i), ha=ha, va=va)


fig, ax = plt.subplots()
ax.set_aspect("equal")
ax.set_axis_off()
ax.set_xlim([-1, 1])
ax.set_ylim([-1, 1])

# space vectors
plot_vector(ax, v_s(0, 0, 0), r"$\vec{v_0} (000)$")
plot_vector(ax, v_s(0, 0, 1), r"$\vec{v_1} (001)$")
plot_vector(ax, v_s(0, 1, 0), r"$\vec{v_2} (010)$")
plot_vector(ax, v_s(0, 1, 1), r"$\vec{v_3} (011)$")
plot_vector(ax, v_s(1, 0, 0), r"$\vec{v_4} (100)$")
plot_vector(ax, v_s(1, 0, 1), r"$\vec{v_5} (101)$")
plot_vector(ax, v_s(1, 1, 0), r"$\vec{v_6} (110)$")
plot_vector(ax, v_s(1, 1, 1), r"$\vec{v_7} (111)$")

# hexagon
angles = np.linspace(0, 2 * np.pi, 7)
for i in range(len(angles) - 1):
    ax.add_line(
        lines.Line2D(
            [np.cos(angles[i]), np.cos(angles[i + 1])],
            [np.sin(angles[i]), np.sin(angles[i + 1])],
            color="k",
        )
    )

plt.show()
