SV-PWM
======

Introduction
------------

The SV-PWM driver is the responsible to control the power stage, that is, the
circuit that powers the motor. The power stage is formed by three half-bridge
circuits, one for each motor phase. Each half-bridge is composed by two
*switches*: :math:`\mathrm{q_i}` and :math:`\mathrm{\bar{q_i}}`, where
:math:`\mathrm{i} \in \mathrm{(a, b, c)}` (:numref:`ps-schematic`). These
switches are implemented using transistors (e.g. MOSFET, GaN...).

.. _ps-schematic:
.. figure:: images/ps-schematic.svg

   Schematic of the power stage *switches*

As the notation suggests, the switches are by complementary PWM signals,
sometimes with the insertion of dead-time. The modulation scheme implemented by
the driver is known as *Space Vector PWM*, hence its name. The theoretical
details of this modulation can be found at the :doc:`/theory/svpwm` page. When
using shunt resistors, current sampling needs to be synchronized with the PWM
generation. Because of this reason, the SV-PWM driver also takes care of current
sampling synchronization. The theoretical details can be found at the
:doc:`/theory/currsmp` page.

API
---

.. doxygengroup:: spinner_drivers_svpwm


Implementations
---------------

.. toctree::
    :glob:

    impl/*
