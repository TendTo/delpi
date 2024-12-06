# Python

`pydelpi` is a Python package that provides bindings to the `delpi` library.
The package can be used to create and solve linear programs from Python.

## From source

Clone the repository and install the package with pip.

### Requirements

- [python-dev](https://packages.ubuntu.com/bionic/python-dev)

### Install

```bash
pip install .
# For development
pip install -e .
```

## From PyPI

```bash
pip install pydelpi
```

## Usage

If the package has been installed, either locally or from PyPI, it can be invoked with the same options as the binary.

```bash
pydelpi --help
```

Furthermore, the `pydelpi` module can be imported and used as a library.

```python
import sys
import pydelpi

# TODO: Add example
```
