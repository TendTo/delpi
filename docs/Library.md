# Library

`delpi` comes with a shared C++ library that can be used to interact with the solver programmatically.  
See the [Installation](./Installation.md) page for instructions on how to install the library.

## Linking

When including the shared library in a project, you can include the main delpi header

```cpp
/* @file <my_file>.cpp */
#include <delpi/delpi.h>
```

and link the library with the flags `-ldelpi`. Add the `-lgmp` flag if your project needs to manipulate rational numbers.

```bash
g++ -std=c++20 <my_file>.cpp -ldelpi -o <out_file>
```

Finally, the executable can be run with

```bash
./<out_file>
```

## Example

The following example demonstrates how to use the library to check the satisfiability of a problem in SMT2 format:

```cpp
/* @file test.cpp */
#include <delpi.h>

```

The example can be compiled with

```bash
g++ -std=c++20 test.cpp -ldelpi -o test
```

and run with

```bash
./test
```
