/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <Python.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace delpi {

void py_check_signals() {
  if (PyErr_CheckSignals() == -1) {
    throw py::error_already_set();
  }
}

}  // namespace delpi
