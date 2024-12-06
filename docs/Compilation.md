# Compilation

## Folder structure

The folder structure is as follows:

```bash
.
├── docs                    # Documentation
│   └── BUILD.bazel         # Documentation BUILD file
├── delpi                   # Main application
│   ├── ...                 # Submodules
│   │   └── ...
│   ├── BUILD.bazel         # Main application BUILD file
│   ├── delpi.h             # Main header
│   └── main.cpp            # Application entry point
├── third_party             # Third party libraries
├── tools                   # Tools and scripts
├── .bazelignore    # Bazel ignore file
├── .bazelrc        # Bazel configuration file
├── .bazelversion   # Bazel version lock file
├── .clang-format   # Clang format configuration file
├── CPPLINT.cfg     # cpplint configuration file
├── BUILD.bazel     # Root BUILD file
└── MODULE.bazel    # Root MODULE file
```

## Utility commands

```bash
# Build the main application.
# The executable can be found in the bazel-bin/delpi directory
bazel build //delpi
```

```bash
# Run the main application and pass an argument (e.g. 2)
bazel run //delpi -- 2
```

```bash
# Build the main application in optimised mode and run it passing it an argument (e.g. 2)
bazel run //delpi --config=opt -- 2
```

```bash
# Run all the tests
bazel test //tests/...
```

```bash
# Only run a specific tagged test
bazel test //tests/... --test_tag_filters=calculator
```

```bash
# Lint all the code
bazel test //delpi/...
```

```bash
# Build the documentation
# The documentation can be found in the bazel-bin/docs directory
bazel build //docs
```

## Configurations

Configurations are used to specify the build type, as they usually group a set of flags.
The following configurations are available:

- `--config=debug` for a debug build
- `--config=opt` for an optimized build
- `--config=bench` for an optimized build without logging and using full static linking
- `--config=python` to build the python bindings
- `--config=iwyu` to run the include-what-you-use check
- `--config=dwyu` to run the depend-on-what-you-use check

### Low-level compilation flags

It not advisable to use these flags directly, as the configurations already group them in a predefined way.
But if you need more control when compiling, the following flags are supported:

- `--enable_static_build` to use fully static linking between all the submodules and the binary. Default is `False`
- `--enable_dynamic_build` to use dynamic linking between all the submodules and the binary. Default is `False`
- `--enable_python_build` to build the python bindings. Default is `False`
- `--enable_fpic_build` to enable position independent code. Default is `False`
- `--static_boost` build boost statically. Default is `True`
- `--enable_qsoptex` to include the QSOptEx LP solver. Default is `True`
- `--enable_soplex` to include the SoPlex LP solver. Default is `True`

## DWYU

Depend on What You Use (DWYU) is an aspect of the Bazel build that checks the dependencies of a target and only includes the necessary ones, also distinguishing between implementation dependencies, which are not propagated, and standard dependencies which are.  
To run the check, use the following command:

```bash
bazel build --config=dwyu //delpi
```
