"""Provides a set of variables to the template engine."""

load("//tools:rules_cc.bzl", "DELPI_AUTHOR", "DELPI_AUTHOR_EMAIL", "DELPI_DESCRIPTION", "DELPI_HOMEPAGE", "DELPI_LICENSE", "DELPI_NAME", "DELPI_SOURCE", "DELPI_TRACKER", "DELPI_VERSION")

def _make_var_substitution_impl(ctx):
    vars = dict(ctx.attr.variables)

    # Python
    py_runtime = ctx.toolchains[ctx.attr._python_toolchain].py3_runtime
    major = py_runtime.interpreter_version_info.major
    minor = py_runtime.interpreter_version_info.minor
    implementation = py_runtime.implementation_name
    if implementation == "cpython":
        tag = "cp" + str(major) + str(minor)
        vars["PYTHON_ABI_TAG"] = tag
        vars["PYTHON_TAG"] = tag
    else:
        fail("This rule only supports CPython.")

    # delpi
    vars["DELPI_NAME"] = DELPI_NAME
    vars["DELPI_VERSION"] = DELPI_VERSION
    vars["DELPI_AUTHOR"] = DELPI_AUTHOR
    vars["DELPI_AUTHOR_EMAIL"] = DELPI_AUTHOR_EMAIL
    vars["DELPI_DESCRIPTION"] = DELPI_DESCRIPTION
    vars["DELPI_HOMEPAGE"] = DELPI_HOMEPAGE
    vars["DELPI_LICENSE"] = DELPI_LICENSE
    vars["DELPI_SOURCE"] = DELPI_SOURCE
    vars["DELPI_TRACKER"] = DELPI_TRACKER

    return [platform_common.TemplateVariableInfo(vars)]

make_var_substitution = rule(
    implementation = _make_var_substitution_impl,
    attrs = {
        "variables": attr.string_dict(),
        "_python_toolchain": attr.string(default = "@rules_python//python:toolchain_type"),
    },
    doc = """Provides a set of variables to the template engine.
Variables are passed as a dictionary of strings.
The keys are the variable names, and the values are the variable values.

It also comes with a set of default variables that are always available:
- DELPI_NAME: The name of the delpi library.
- DELPI_VERSION: The version of the delpi library.
- DELPI_AUTHOR: The author of the delpi library.
- DELPI_AUTHOR_EMAIL: The author email of the delpi library.
- DELPI_DESCRIPTION: The description of the delpi library.
- DELPI_HOMEPAGE: The homepage of the delpi library.
- DELPI_LICENSE: The license of the delpi library.
- PYTHON_ABI_TAG: The Python ABI tag (e.g., cp36, cp311).
- PYTHON_TAG: The Python tag (e.g., cp36, cp311).

Example:
```python
make_var_substitution(
    variables = {
        "MY_VARIABLE": "my_value",
    },
)
```

This will make the variable `MY_VARIABLE` available to the template engine.
""",
    toolchains = [
        "@rules_python//python:toolchain_type",
    ],
)
