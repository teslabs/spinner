.. _theory-svpwm:

SV-PWM
======

In a FOC based system we usually synthesize the voltage Space Vector
:math:`\vec{v_s}` using an inverter. When using an inverter we can not generate
all the voltage levels we want but only a discrete set. With a 2-level 3-phase
inverter, the most common one, we can generate :math:`2^3 = 8` voltage levels,
given by:

.. math::

        \vec{v_s} = V_d (q_a e^{j0} + q_b e^{j \frac{2\pi}{3}} + q_c e^{j \frac{4\pi}{3}})

where :math:`V_d` is the line voltage and :math:`q_a, q_b, q_c \in (0 =
\text{OFF}, 1 = \text{ON})` correspond to the the *switch* state of each phase.

.. _table-space-vectors:
.. table:: Synthesizable space vectors with a 2-level 3-phase inverter.
        :align: center

        ================================================ =========== =========== ===========
        Space Vector                                     :math:`q_c` :math:`q_b` :math:`q_a`
        ================================================ =========== =========== ===========
        :math:`\vec{v_0} = 0`                            0           0           0
        :math:`\vec{v_1} = V_d`                          0           0           1
        :math:`\vec{v_2} = V_d e^{j \frac{2 \pi}{3}}`    0           1           0
        :math:`\vec{v_3} = V_d e^{j \frac{\pi}{3}}`      0           1           1
        :math:`\vec{v_4} = V_d e^{-j \frac{2 \pi}{3}}`   1           0           0
        :math:`\vec{v_5} = V_d e^{-j \frac{\pi}{3}}`     1           0           1
        :math:`\vec{v_6} = -V_d`                         1           1           0
        :math:`\vec{v_7} = 0`                            1           1           1
        ================================================ =========== =========== ===========

If we draw lines that go from one vector edge to the other we can observe that
these lines form an hexagon as shown below.

.. _svpwm-hexagon:
.. plot:: theory/images/svpwm-hexagon.py

        SV-PWM synthesizable state vectors.

In order to synthesize an arbitrary voltage, there is a rather simple technique:
given the sector in which the vector to be synthesized falls, we can quickly
alternate between the two adjacent vectors. Taking a voltage falling in the
first sector, i.e. :math:`\theta \in \left( 0, \frac{\pi}{3} \right)`, we have
that the average space-vector :math:`\vec{v^a_s}` is given by:

.. math::

        \left.\vec{v^a_s}\right|_{\text{sector} = 1}
        = \frac{1}{T} \left( x T \vec{v_1} + y T \vec{v_3} + z T \vec{0} \right)
        = x \vec{v_1} + y \vec{v_3}

where :math:`T` is the averaging period and :math:`x + y + z = 1`. Note that
:math:`z` is the fraction of time where the actual voltage is zero (this happens
for :math:`\vec{v_0}` and :math:`\vec{v_7}`). Replacing with values from
:numref:`table-space-vectors` we have:

.. math::
        \hat{V_s} e^{j \theta_s} = x V_d e^{j 0}  + y V_d e^{j \frac{\pi}{3}}

and by equaling both real and imaginary components, we obtain:

.. math::
        x &= \frac{\hat{V_s}}{V_d} \left( \cos(\theta_s) - \frac{1}{\sqrt{3}} \sin(\theta_s) \right) \\
        y &= \frac{\hat{V_s}}{V_d} \frac{2}{\sqrt{3}} \sin(\theta_s).

In terms of the :math:`\alpha` and :math:`\beta` components, we have:

.. math::
        x &= \frac{1}{V_d} \left( \hat{v_{\alpha}} - \frac{1}{\sqrt{3}} \hat{v_{\beta}} \right) \\
        y &= \frac{1}{V_d} \frac{2}{\sqrt{3}} \hat{v_{\beta}}.

If we repeat the previous calculation for each sector we get similar results. If
we take:

.. math::
        :label: eq-abc

        a &= x \\
        b &= y \\
        c &= -(x + y)

being :math:`x, y` the values obtained for the first sector, we can express the
other values as a function of :math:`a, b, c`.

.. _table-svpwm-XY:
.. table:: SV-PWM X-Y
        :align: center

        ====== ========== ==========
        Sector :math:`x`  :math:`y`
        ====== ========== ==========
        1      a          b
        2      -c         -a
        3      b          c
        4      -a         -b
        5      c          a
        6      -b         -c
        ====== ========== ==========

Sector determination
--------------------

As we have already seen, knowledge of the sector is essential to compute the
:math:`x, y` values. If we are given the space vector in cartesian form, that
is, :math:`v_{\alpha}` and :math:`v_{\beta}`, we can easily determine its angle
by performing :math:`\arctan\left(\frac{v_{\beta}}{v_{\alpha}}\right)` and hence
the sector. However, :math:`\arctan` is an expensive trigonometric computation.
It turns out there is a faster way to determine the sector.

If we take a look at the plot of Eq. :eq:`eq-abc` for all sectors, we have that
each sector has a unique combination of signs:

.. plot:: theory/images/svpwm-abc-signs.py

        Eq. :eq:`eq-abc` signs.

.. table::
        :align: center

        ====== ====================== ====================== ======================
        Sector :math:`\text{sign}(a)` :math:`\text{sign}(b)` :math:`\text{sign}(c)`
        ====== ====================== ====================== ======================
        1      ``+``                  ``+``                  ``-``
        2      ``-``                  ``+``                  ``-``
        3      ``-``                  ``+``                  ``+``
        4      ``-``                  ``-``                  ``+``
        5      ``+``                  ``-``                  ``+``
        6      ``+``                  ``-``                  ``-``
        ====== ====================== ====================== ======================


Amplitude limitation
--------------------

By looking at the hexagon we can quickly observe that vectors falling in the
middle of a sector will not have the same average amplitude as the ones that can
be perfectly generated :math:`\vec{v_i}, i \in (0, ..., 7)`. The worst case
happens for the space vectors falling just in the middle of the sector as shown
on the figure below.

.. figure:: images/svpwm-limit.svg

        Maximum synthesizable amplitude without distortion.

Therefore, in order to avoid distortions the maximum average amplitude should be
limited to:

.. math::

        \hat{V_s}_{MAX} = V_d \cos(30^{\circ}) = V_d \frac{\sqrt{3}}{2}.

The previous value is actually the maximum line voltage we will be able to use
when using SV-PWM.

Duty cycles calculation
-----------------------

Finally, we need to compute the PWM duty cycles using the values calculated in
:numref:`table-svpwm-XY`. When using a center-aligned PWM we have that the
actual PWM output is "ON" when the control variable is over the trigger signal
(a saw-tooth) and "OFF" otherwise.

There is still one thing left: the zero or null vector. Right at the beginning
of this section we saw that in the formation of the space vector there is a
fraction of time, :math:`z`, where a *zero* vector is active. There are actually
a couple of zero vectors, :math:`\vec{v_0}` and :math:`\vec{v_7}`. Both vectors
are valid in order to produce the space vector, however, it is common to use the
null-vector that only requires a single switch state change with respect to the
previous or future state. This choice is also known as the **reverse-alternating
sequence**. For example, if the next state is :math:`\vec{v_1}` (001), then the
null-vector choice would be :math:`\vec{v_0}`. Below the waveforms on the first
sector are shown when using such sequence.

.. _svpwm-pwm-timing:
.. figure:: images/svpwm-pwm-timing.svg

        PWM waveforms in sector 1 (:math:`z = z_0 + z_7`).

By looking at the timing diagram in :numref:`svpwm-pwm-timing`, we have that the
duty cycles are for the first sector:

.. math::
        d_a &= x + y + \frac{z}{2} \\
        d_b &= y + \frac{z}{2} \\
        d_c &= \frac{z}{2}

We can do a similar calculation for each sector, leading to the duty cycles
listed in :numref:`table-svpwm-duties`.

.. _table-svpwm-duties:
.. table:: SV-PWM duty cycle equations
        :align: center

        ====== =========================== =========================== ===========================
        Sector :math:`d_a`                 :math:`d_b`                 :math:`d_c`
        ====== =========================== =========================== ===========================
        1      :math:`x + y + \frac{z}{2}` :math:`y + \frac{z}{2}`     :math:`\frac{z}{2}`
        2      :math:`x + \frac{z}{2}`     :math:`x + y + \frac{z}{2}` :math:`\frac{z}{2}`
        3      :math:`\frac{z}{2}`         :math:`x + y + \frac{z}{2}` :math:`y + \frac{z}{2}`
        4      :math:`\frac{z}{2}`         :math:`x + \frac{z}{2}`     :math:`x + y + \frac{z}{2}`
        5      :math:`y + \frac{z}{2}`     :math:`\frac{z}{2}`         :math:`x + y + \frac{z}{2}`
        6      :math:`x + y + \frac{z}{2}` :math:`\frac{z}{2}`         :math:`x + \frac{z}{2}`
        ====== =========================== =========================== ===========================

Using equations from :numref:`table-svpwm-XY` and :numref:`table-svpwm-duties`
we can plot the duty cycle waveforms:

.. _svpwm-modulation:
.. plot:: theory/images/svpwm-modulation.py

        SV-PWM duty cycle waveforms.
