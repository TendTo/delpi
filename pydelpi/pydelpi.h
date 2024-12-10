/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#ifndef DELPI_PYTHON
#error DELPI_PYTHON is not defined. Ensure you are building with the option '--config=python'
#endif

#include <pybind11/pybind11.h>

#include <sstream>

#include "delpi/libs/gmp.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)
#define STR_LAMBDA(class_name) [](const class_name &o) { return (std::stringstream{} << o).str(); }
#define REPR_LAMBDA(class_name) \
  [](const class_name &o) { return (std::stringstream{} << "<" STRINGIFY(class_name) "'" << o << "'>").str(); }

// template <>
// struct pybind11::detail::type_caster<mpq_class>;

void init_util(pybind11::module_ &);
void init_symbolic(pybind11::module_ &);
void init_solver(pybind11::module_ &);
