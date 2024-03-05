STM32
=====

Introduction
------------

The timer peripheral is the core part of the SV-PWM driver. It generates the PWM
signals that drive the inverter circuit and it also takes care of synchronizing
ADC measurements made by the current sampling driver.

The driver is designed to work using one of the *advanced control timers*,
usually ``TIM1`` and ``TIM8``. They include specific functionalities that are
crucial to have a performant and safe system :cite:`an4013`.
:numref:`stm32-timer-diagram` shows the diagram of an advanced control timer.

.. _stm32-timer-diagram:
.. figure:: images/stm32-timer-diagram.png

        Advanced control timer diagram :cite:`rm0365`.

PWM generation
--------------

The timer peripheral clock, :math:`\mathrm{f_{TIM}}`, is as fundamental variable
as it controls the timer counting rate. The timer clock is divided by the
prescaler, which is controlled by the ``PSC`` register (16-bit). The counting
rate, :math:`\mathrm{f_{CNT}}`, is defined as:

.. math::

        \mathrm{f_{CNT} = \frac{f_{TIM}}{PSC + 1}}

STM32 timers have multiple PWM modes. The most interesting mode when doing motor
control is the **center-aligned PWM mode**
(:numref:`stm32-timer-centeraligned`). In this mode the counting direction
(up/down) is automatically alternated by the timer. This mode provides an
interesting feature when multiple PWM waveforms are generated such as in a
3-phase inverter. Contrary to edge-aligned modes, in this mode the rising and
falling edges of the PWM signals are not synchronized with the counter
roll-over. Therefore, switching time varies with the duty cycle value and
switching noise is spread. This is a key feature for electric motor drives,
since it allows to double the frequency of the current ripple for a given
switching frequency. For instance, a 10 kHz PWM will generate inaudible 20 kHz
current ripple. This feature also minimizes the switching losses due to the PWM
frequency while guaranteeing a silent operation.

.. _stm32-timer-centeraligned:
.. figure:: images/stm32-timer-centeraligned.svg

        Timing diagram for a timer in center-aligned PWM mode.

Using the above diagram, we can see that the PWM frequency
(:math:`\mathrm{f_{PWM}}`) is given by:

.. math::

        \mathrm{f_{PWM} = \frac{f_{TIM}}{2 \cdot (ARR + 1) \cdot (PSC + 1)}}

where ``ARR`` is the auto-reload register, a 16-bit value. In order to maximize
PWM resolution ``PSC`` should be chosen so that ``ARR`` is maximized while
fitting into its 16-bit register.

.. topic:: ARR calculation example

        Given :math:`\mathrm{f_{TIM} = 72~MHz}` and :math:`\mathrm{f_{PWM} = 30~KHz}`,
        we start with :math:`\mathrm{PSC = 0}`, which leads to:

        .. math::

                \mathrm{ARR = \frac{72~MHz}{2 \cdot 30~KHz \cdot (0 + 1)} - 1 = 1199}.

        As 1199 fits into a 16-bit register, we stick to :math:`\mathrm{PSC = 0}`.

The PWM duty cycle is controlled via the ``CCRx`` registers (``x = 1, 2, ...``).
``CCRx`` value is compared against ``CNT`` so that the PWM signal is active when
``CNT < CCRx``. In case ``CCRx`` is set to zero, the PWM signal is always kept
inactive.

There is an important feature that has to be enabled for ``CCRx`` registers:
pre-load. When pre-load is enabled, the register value is only updated when the
timer update event occurs. This is particular useful for real-time control, as
the new register values are applied synchronously. However in center aligned
mode, we have two update events: overflow (at the end of up cycle) and underflow
(at the end of down cycle). Update event happening on overflow should be avoided
since it is the time when current sampling is likely going to occur, and so the
regulation loop. Repetition counter feature comes to the rescue to solve this
problem. In center-aligned mode, odd values of the repetition counter generate
the update event either on overfow or underflow depending on when the repetition
counter register ``RCR`` was written and the counter launched. If ``RCR`` was
written before starting the counter, the update event will occur on underflow
and on overflow if ``RCR`` was written after starting the counter.

.. figure:: images/stm32-timer-repcnt.png

        Example of repetition counter update event generation :cite:`rm0365`.

Up to now all the details on the signal generation have been given. The only
missing part is now how to expose these signals to the outside via the ``OCx``
pins. This is controlled by the output stage of the capture/compare channel as
seen on :numref:`stm32-cc-output`. In general it is necessary to control both
high and low sides of each inverter leg. For this purpose complementary outputs
can be enabled (``OCxN``). As described in the following section, it is also
possible to insert dead-time when using complementary outputs. Some integrated
drivers do not require complementary signals since they internally take care of
their generation including dead-time insertion.

.. _stm32-cc-output:
.. figure :: images/stm32-cc-output.png

        Output stage of capture/compare channel :cite:`rm0365`.

ADC synchronization
-------------------

As detailed in the :doc:`/theory/currsmp` page, it is crucial to synchronize the
current measurements with the PWM generation. For this purpose, the driver uses
its fourth channel compare unit (``OC4``) to trigger the ADC.  The value of the
``CCR4`` register controlling the signal duty cycle is updated every time phase
voltages are set so that currents are always sampled at an optimal point. The
``OC4`` output is connected to the ``TRGO`` output signal.  The ADC device
managed by the current sampling driver is responsible to connect to this signal
as a trigger source.

Break function
--------------

The break function is used to protect the power stage driven by the PWM signals.
There are two break inputs which are usually connected to fault signals
generated by the power stage circuit (e.g. over-current). When any of the input
is activated a hardware protection mechanism is triggered so that the PWM
outputs are disabled, leaving them in a pre-programmed state. The break
circuitry works asynchronously, that is, it does not depend on any system clock.
This feature makes sure that the circuitry does not suffer from any clock
propagation delay or system clock failures.

.. _stm32-timer-brk:
.. figure:: images/stm32-timer-brk.png

        Break circuitry :cite:`rm0365`.

As shown in :numref:`stm32-timer-brk` ``BRK`` is the result of either an
external event (``BKIN``) or an internal event (``BRK_ACTH``) such as a clock
failure event (refer to :cite:`rm0365` for more details). The first channel,
``BRK``, has priority over ``BRK2``. ``BRK`` can also be configured to either
disable (inactive) or force PWM outputs to a predefined safe state. Furthermore,
a dead-time can be programmed to avoid potential shoot-through when activating
the break functionality. This provides a dual-level protection scheme, where for
instance a low priority protection with all switches off can be overridden by a
higher priority protection with low-side switches active. Let’s consider for
instance that the fault occurs when the high-side PWM is ON, while the safe
state is programmed to have high-side switched OFF and low-side switched ON. At
the time the fault occurs the system will first disable the high-side PWM, and
insert a dead time before switching ON the low side.

.. figure:: images/stm32-timer-brkconf.png

        Typical break use case :cite:`rm0365`.
