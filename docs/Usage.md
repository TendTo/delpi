# Usage

After [compilation](./Installation.md), the `delpi` binary will be located in the `bazel-bin/delpi` directory.
There are several options to run it, and any of the following methods can be used:

- run `./bazel-bin/delpi/delpi` in the terminal
- add the absolute path of the directory `bazel-bin/delpi` to the `PATH` environment variable
- move the binary from `./bazel-bin/delpi/delpi` to a directory already in the `PATH`, like `/usr/local/bin`
- create a symbolic link to the executable
- create an alias for the executable

From now on, the snippets will assume that the `delpi` binary has been added to the `PATH`.

```bash
# Invoke the delpi help tooltip
delpi -h
```

## File mode

By default, _delpi_ expects the path to the problem file as the only positional argument.
The file can be in the [MPS](<https://en.wikipedia.org/wiki/MPS_(format)>) format.
If left unspecified, the program will look at the file extension to determine the format.

```bash
# Invoke delpi with a problem in MPS format
delpi path/to/problem.mps
# Invoke delpi with a problem explicitally indicating the format
delpi path/to/problem --format MPS 
```

## Stdin mode

_delpi_ can be used in stdin mode, where the user can input is received from the standard input.
Note that in this case the format must be specified explicitly.

```bash
# Invoke delpi in stdin mode (MPS format)
delpi --in --format mps
```

When the stdin mode is used, the problem must be typed directly in the terminal.
To signal delpi the end of the input, press `Ctrl+D` two times.
This will start the solver.
