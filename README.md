<p align="center">
  <img width="256" src="./docs/_static/images/logo.svg" alt="Vertigo">
</p>
<p align="center">
  <a href="https://github.com/teslabs/spinner/actions/workflows/build.yml">
    <img src="https://github.com/teslabs/spinner/actions/workflows/build.yml/badge.svg" alt="CI status">
  </a>
  <a href="https://teslabs.github.io/spinner">
    <img src="https://img.shields.io/static/v1.svg?label=latest&message=documentation&color=blue)" alt="Documentation">
  </a>
</p>

# Introduction

Spinner is a motor control firmware based on the Field Oriented Control (FOC)
principles. The firmware is built on top of the [Zephyr RTOS](https://zephyrproject.org),
a modern multi-platform RTOS. Spinner is still a proof of concept, so do not
expect production grade stability or features.

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

The first step is to initialize the `spinner` workspace folder where the
source and all Zephyr modules will be cloned. You can do that by running:

```shell
# initialize spinner workspace
west init -m https://github.com/teslabs/spinner --mr main spinner
# update modules
cd spinner
west update
```

### Build & Run

The application can be built by running:

```shell
west build -b $BOARD spinner [-- -DSHIELD=$SHIELD]
```

where `$BOARD` is the target board (see `boards`) and `$SHIELD` an optional
shield (see `boards/shields`). Some other build configurations are also provided:

- `debug.conf`: Enable debug-friendly build
- `shell.conf`: Enable shell facilities

They can be enabled by setting `OVERLAY_CONFIG`, e.g.

```shell
west build -b $BOARD spinner -- -DOVERLAY_CONFIG=debug.conf
```

Once you have built the application you can flash it by running:

```shell
west flash
```

## Documentation

The documentation is based on Sphinx. Doxygen is used to extract the API
docstrings, but its HTML output can also be used if preferred. A simple CMake
script is provided in order to facilitate the documentation build process. In
order to configure CMake you need to run:

```shell
cmake -Sdocs -Bbuild_docs
```

In order to build the Doxygen documentation you need to run:

```shell
cmake --build build_docs -t doxygen
```

Note that Doxygen output is required by Sphinx, so every time you change your
API docstrings, remember to run the `doxygen` target. In order to build the
Sphinx HTML documentation you need to run:

```shell
cmake --build build_docs -t html
```
