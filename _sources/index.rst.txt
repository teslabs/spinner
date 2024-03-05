=====================
Spinner Documentation
=====================

Welcome to the Spinner's documentation! Spinner is a motor control firmware
based on the Field Oriented Control (FOC) principles. The firmware is built
on top of the `Zephyr RTOS`_, a modern multi-platform RTOS. Spinner is still
a proof of concept, so do not expect production grade stability or features.

Some of the offered features are:

- FOC based current control loop
- Driver APIs for:

  - Current sampling
  - SV-PWM
  - Feedbacks (e.g. Halls)

These documentation pages contain architecture details of the Spinner
firmware as well as some driver implementation details.

.. _Zephyr RTOS: https://zephyrproject.org

.. toctree::
    :caption: Theory
    :glob:
    :hidden:

    theory/*

.. toctree::
    :caption: Components
    :glob:
    :hidden:

    components/**/index

.. toctree::
    :caption: Reference
    :hidden:

    API <https://teslabs.github.io/spinner/doxygen/html/index.html>
    zbibliography
