/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#ifndef DELPI_PYTHON
#error DELPI_PYTHON is not defined. Ensure you are building with the option '--config=python'
#endif

#include "delpi/solver/solver.h"

#include <pybind11/stl.h>

#include "pydelpi.h"

namespace py = pybind11;
using namespace delpi;

void init_solver(py::module_ &m) {
  py::class_<Column>(m, "Column")  //
      .def(py::init<>())
      .def_readwrite("var", &Column::var)
      .def_readwrite("lb", &Column::lb)
      .def_readwrite("ub", &Column::ub)
      .def_readwrite("obj", &Column::obj)
      .def("__str__", STR_LAMBDA(Column))
      .def("__repr__", REPR_LAMBDA(Column));

  py::class_<Row>(m, "Row")  //
      .def(py::init<>())
      .def_readwrite("addends", &Row::addends)
      .def_readwrite("lb", &Row::lb)
      .def_readwrite("ub", &Row::ub)
      .def("__str__", STR_LAMBDA(Row))
      .def("__repr__", REPR_LAMBDA(Row));

  py::enum_<LpResult>(m, "LpResult")
      .value("OPTIMAL", LpResult::OPTIMAL)
      .value("DELTA_OPTIMAL", LpResult::DELTA_OPTIMAL)
      .value("UNBOUNDED", LpResult::UNBOUNDED)
      .value("INFEASIBLE", LpResult::INFEASIBLE)
      .value("ERROR", LpResult::ERROR)
      .value("UNSOLVED", LpResult::UNSOLVED);

  py::class_<LpSolver>(m, "LpSolver")
      .def_static("get_instance", &LpSolver::GetInstance, py::arg("config"))
      .def_property_readonly("variables", &LpSolver::variables)
      .def_property_readonly("constraints", &LpSolver::constraints)
      .def("var", &LpSolver::var, py::arg("column_idx"))
      .def("parse", &LpSolver::Parse)
      .def("parse_file", &LpSolver::ParseFile, py::arg("filename"))
      .def("parse_string", &LpSolver::ParseString, py::arg("input"))
      .def("add_column", py::overload_cast<const Variable &>(&LpSolver::AddColumn), py::arg("column"))
      .def("add_column", py::overload_cast<const Variable &, const mpq_class &>(&LpSolver::AddColumn),
           py::arg("column"), py::arg("obj"))
      .def("add_column",
           py::overload_cast<const Variable &, const mpq_class &, const mpq_class &>(&LpSolver::AddColumn),
           py::arg("column"), py::arg("lb"), py::arg("ub"))
      .def("add_column",
           py::overload_cast<const Variable &, const mpq_class &, const mpq_class &, const mpq_class &>(
               &LpSolver::AddColumn),
           py::arg("column"), py::arg("obj"), py::arg("lb"), py::arg("ub"))
      .def("add_row", py::overload_cast<const Formula &>(&LpSolver::AddRow), py::arg("formula"))
      .def("add_row", py::overload_cast<const Expression &, FormulaKind, const mpq_class &>(&LpSolver::AddRow),
           py::arg("formula"), py::arg("kind"), py::arg("rhs"))
      .def("solve", &LpSolver::Solve, py::arg("precision"), py::arg("store_solution") = true)
      .def("solution", [](const LpSolver &self) { return self.solution(); })
      .def("solution", [](const LpSolver &self, const Variable &var) { return self.solution(var); })
      .def("row", &LpSolver::row, py::arg("row_idx"))
      .def("column", &LpSolver::column, py::arg("column_idx"));
}