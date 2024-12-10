/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#ifndef DELPI_PYTHON
#error DELPI_PYTHON is not defined. Ensure you are building with the option '--config=python'
#endif

#include "delpi/solver/solver.h"

#include <pybind11/operators.h>

#include "pydelpi.h"

namespace py = pybind11;
using namespace delpi;

void init_solver(py::module_ &m) {
  py::enum_<LpResult>(m, "LpResult")
      .value("OPTIMAL", LpResult::OPTIMAL)
      .value("DELTA_OPTIMAL", LpResult::DELTA_OPTIMAL)
      .value("UNBOUNDED", LpResult::UNBOUNDED)
      .value("INFEASIBLE", LpResult::INFEASIBLE)
      .value("ERROR", LpResult::ERROR)
      .value("UNSOLVED", LpResult::UNSOLVED);
}