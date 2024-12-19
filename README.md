# <img alt="Icon" src="docs/_static/logo.svg" align="left" width="35" height="35"> delpi

[![delpi CI](https://github.com/TendTo/delpi/actions/workflows/delpi.yml/badge.svg)](https://github.com/TendTo/delpi/actions/workflows/delpi.yml)
[![pydelpi CI](https://github.com/TendTo/delpi/actions/workflows/pydelpi.yml/badge.svg)](https://github.com/TendTo/delpi/actions/workflows/pydelpi.yml)
[![docker CI](https://github.com/TendTo/delpi/actions/workflows/docker.yml/badge.svg)](https://github.com/TendTo/delpi/actions/workflows/docker.yml)
[![Docs CI](https://github.com/TendTo/delpi/actions/workflows/docs.yml/badge.svg)](https://github.com/TendTo/delpi/actions/workflows/docs.yml)

Exact LP solver with support for delta relaxation.

## Installation

There are multiple ways of installing `delpi`.
The recommended approach is to use the official Docker image or the python wrapper, `pydelpi`.

> [!Note]  
> Only Linux is supported, but using the Docker image circumvents this limitation.

```bash
# Docker
docker pull ghcr.io/tendto/delpi:main
# Run delpi
docker run -it --rm ghcr.io/tendto/delpi:main --help
```

```bash
# pydelpi
pip3 install pydelpi
# Run delpi
pydelpi --help
```

For more information about the setup, including installation from sources, refer to the [installation guide](docs/Installation.md) and [usage guide](docs/Usage.md).
