/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#ifndef DELPI_PYTHON
#error DELPI_PYTHON is not defined. Ensure you are building with the option '--config=python'
#endif

#include "pydelpi.h"

#include "delpi/libs/gmp.h"
#include "delpi/version.h"

namespace py = pybind11;

PYBIND11_MODULE(_pydelpi, m) {
  init_symbolic(m);
  init_util(m);
  init_solver(m);

  m.doc() = DELPI_DESCRIPTION;
#ifdef DELPI_VERSION_STRING
  m.attr("__version__") = DELPI_VERSION_STRING;
#else
#error "DELPI_VERSION_STRING is not defined"
#endif
}
