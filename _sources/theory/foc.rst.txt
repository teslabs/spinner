Field Oriented Control
======================

Field Oriented Control (FOC) consists on controlling the stator currents
represented by a vector in a 2-D time-invariant space :math:`dq`. The :math:`dq`
space is an orthogonal space aligned with the rotor: flux is aligned with
:math:`d` and torque is aligned with :math:`q`. A set of projections is used to
transform from a three-phase speed and time dependent system to :math:`dq`. As
the transformations are just projections the controlled magnitudes are
instantaneous quantities, making the control structure valid for transient and
steady state.

It can be shown that in the :math:`dq` space we have:

.. math::

        T \propto \psi_R i_q

that is, by maintaining the rotor flux constant :math:`\psi_R` we have that the
generated torque :math:`T` is directly proportional to the :math:`i_q` stator
current. We can then perform torque control by changing the :math:`i_q` current
reference. Because the speed and time dependency is removed from the :math:`dq`
space, the control strategy is also simplified as constant references are being
controlled.

Space Vector
------------

We have that for three-phase AC motors, voltages, currents and fluxes can be
analyzed in terms of complex space vectors. First we define the :math:`abc`
space, given by the following three unit vectors in the complex space:

.. math::

        \hat{a} &= e^{j0} \\
        \hat{b} &= e^{j \frac{2 \pi}{3}} \\
        \hat{c} &= e^{j \frac{4 \pi}{3}}. \\

Then, the space vector for currents (same applies to other magnitudes) is
defined as:

.. math::
        \vec{i_s} = i_a \hat{a} + i_b \hat{b} + i_c \hat{c}
                  = \vec{i_a} + \vec{i_b} + \vec{i_c}.

The definition above may sound abstract, but with some more context it can be
better understood. Let us start by looking at the currents shape. Given a
three-phase balanced AC system, we have that phase currents are sinusoidal in
steady state, i.e.:

.. math::

        i_a(t) &= I \cos(\omega t + \phi_0) \\
        i_b(t) &= I \cos(\omega t - \frac{2 \pi}{3} + \phi_0) \\
        i_c(t) &= I \cos(\omega t - \frac{4 \pi}{3} + \phi_0) \\

where :math:`I` is the current magnitude, :math:`\omega` is the rotation speed
in rad/s and :math:`\phi_0` is an arbitrary initial phase. :math:`\omega t` is
the instantaneous position, :math:`\theta`. Note that both rotation speed and
instantaneous position are always in electrical terms. We can read from the
equations that :math:`i_b` lags :math:`i_a` by :math:`\frac{2 \pi}{3}` rad, and
:math:`i_c` lags :math:`i_b` by the same amount. We can also observe that the
following equality holds as the system is balanced:

.. math::

        i_a(t) + i_b(t) + i_c(t) = 0.

Using the above equations we can plot the space vector and its components as a
function of the rotor position (:numref:`foc-abcs-anim`). The space vector can
be seen as a CCW rotating vector with rotation speed :math:`\omega` in the
complex space.

.. _foc-abcs-anim:
.. figure:: images/foc-abcs-anim.gif

        Animated visualization of the space vector.

Clarke transform
----------------

Any non-orthogonal space indicates a redundancy in its axes. This is the case of
the :math:`abc` space, which can be reduced to the complex space. The complex
space is usually referred in the motor control literature as the :math:`\alpha
\beta` space.  In order to derive the transform from the :math:`abc` space to
the :math:`\alpha \beta` space, we can take the projection of the space vector
components into the :math:`\alpha \beta` axes, that is:

.. math::

        i_{\alpha} &= \Re(\vec{i_a} + \vec{i_b} + \vec{i_c})
                    = i_a + i_b \cos \left(\frac{2 \pi}{3}\right) + i_c \cos\left(\frac{4 \pi}{3}\right)
                    = i_a - \frac{1}{2} (i_b + i_c), \\
        i_{\beta}  &= \Im(\vec{i_a} + \vec{i_b} + \vec{i_c})
                    = i_b + \sin\left(\frac{2 \pi}{3}\right) + i_c \sin\left(\frac{4 \pi}{3}\right)
                    = \frac{\sqrt{3}}{2} (i_b - i_c).

Using the equality :math:`i_a + i_b + i_c = 0`, we can further simplify the
expressions:

.. math::

        i_{\alpha} &= i_a \\
        i_{\beta}  &= \left( \frac{1}{\sqrt{3}} i_a + \frac{2}{\sqrt{3}} i_b \right).

This transform is known as the **Clarke transform**, which in matrix form is:

.. math::

        (a, b) \rightarrow (\alpha, \beta): \mathbf{C} =
        \begin{bmatrix}
          1 && 0 \\
          \frac{1}{\sqrt{3}} && \frac{2}{\sqrt{3}}
        \end{bmatrix}.

It is important to note that :math:`\det{\mathbf{C}} \neq 1`, so the transform
is not power-invariant. Its inverse is defined as:

.. math::

        (\alpha, \beta) \rightarrow (a, b): \mathbf{C^{-1}} =
        \begin{bmatrix}
          1 && 0 \\
          -\frac{1}{2} && \frac{\sqrt{3}}{2}
        \end{bmatrix}.

Park transform
--------------

After the application of the Clarke transformation, we still have quantities
that are speed and time dependent. Assuming we have knowledge of the rotor
position, :math:`\theta = \omega t`, we can de-rotate the :math:`\alpha\beta`
space, therefore removing the speed and time dependency. The new frame will
actually be a **rotating frame** and it is known as the :math:`dq` space, the
space mentioned at the beginning.

In order to derive the transformation we need to again project the
:math:`\alpha\beta` components to the rotating frame. The transform is known as
the **Park transform** and it is actually the well-known 2-D rotation matrix
in its inverse form as we are de-rotating or moving clock-wise.

.. math::

        (\alpha, \beta) \rightarrow (d, q): \mathbf{P} =
        \begin{bmatrix}
          \cos(\theta) && \sin(\theta) \\
          -\sin(\theta) && \cos(\theta)
        \end{bmatrix}.

In its inverse form it is given by:

.. math::

        (\alpha, \beta) \rightarrow (d, q): \mathbf{P^{-1}} =
        \begin{bmatrix}
          \cos(\theta) && -\sin(\theta) \\
          \sin(\theta) && \cos(\theta)
        \end{bmatrix}.
