Feedback
========

Introduction
------------

Feedback drivers are responsible of providing information of the rotor position
or speed. One of the core principles of FOC is the knowledge of the rotor
position, therefore, it is one of the core devices of the system. To be precise,
FOC depends on the knowledge of the electrical angle of the motor, which is
directly related to the rotor position via the number of pair poles.

There is a wide variety of feedback sensors. Their choice depends on the end
application or required control. For example, Halls may be a good choice for
speed control as detailed above. However, for accurate position control digital
encoders may be a better candidate.

Halls
*****

Hall sensors are a common type of feedback based on the `Hall effect`_. The
sensor is actually composed by three individual hall sensors equally distributed
in the distance of one electrical revolution. The combination of the three
output signals using the XOR function results in a square wave that provides an
electrical angle resolution of 60 degress (:numref:`feedback-halls`). Because of
their low resolution Hall sensors are frequently used for speed control. An
important characteristic of Hall sensors is that their position feedback is
absolute.

.. _feedback-halls:
.. figure:: images/halls.svg

   Hall sensors signals versus the motor electrical angle.

.. _Hall effect: https://en.wikipedia.org/wiki/Hall_effect_sensor

API
---

.. doxygengroup:: spinner_drivers_feedback

Implementations
---------------

.. toctree::
    :glob:

    impl/*
