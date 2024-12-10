/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#ifndef DELPI_PYTHON
#error DELPI_PYTHON is not defined. Ensure you are building with the option '--config=python'
#endif

#include "delpi/symbolic/symbolic.h"

#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "pydelpi.h"

namespace py = pybind11;
using namespace delpi;

namespace {

Expression::Addends to_mpq_map(const std::map<Variable, double> &addends) {
  Expression::Addends mpq_addends;
  for (const auto &[var, coeff] : addends) {
    mpq_addends.emplace(var, coeff);
  }
  return mpq_addends;
}

}  // namespace

void init_symbolic(py::module_ &m) {
  auto VariableClass = py::class_<Variable>(m, "Variable");
  auto ExpressionClass = py::class_<Expression>(m, "Expression");
  auto FormulaClass = py::class_<Formula>(m, "Formula");

  VariableClass.def(py::init<const std::string &>())
      .def(py::init<const std::string &>(), py::arg("name"))
      .def_property_readonly("id", &Variable::id)
      .def_property_readonly("name", &Variable::name)
      .def_property_readonly("is_dummy", &Variable::is_dummy)
      .def("equal_to", &Variable::equal_to, py::arg("o"))
      .def("less", &Variable::less, py::arg("o"))
      .def("__hash__", &Variable::hash)
      .def("__str__", STR_LAMBDA(Variable))
      .def("__repr__", REPR_LAMBDA(Variable))
      // Addition.
      .def(py::self + py::self)
      // Subtraction.
      .def(py::self - py::self)
      // Multiplication.
      .def(py::self * double())
      .def(double() * py::self)
      // Division.
      .def(py::self / double())
      // Unary Plus.
      .def(+py::self)
      // Unary Minus.
      .def(-py::self)
      // LT(<).
      .def(py::self < py::self)
      .def(py::self < double())
      // LE(<=).
      .def(py::self <= py::self)
      .def(py::self <= double())
      // GT(>).
      .def(py::self > py::self)
      .def(py::self > double())
      // GE(>=).
      .def(py::self >= py::self)
      .def(py::self >= double())
      // EQ(==).
      .def(py::self == py::self)
      .def(py::self == double())
      // NE(!=).
      .def(py::self != py::self)
      .def(py::self != double());

  ExpressionClass.def(py::init<>())
      .def(py::init<const Variable &>(), py::arg("var"))
      .def(py::init<>([](const std::map<Variable, double> &addends) {
             return std::make_unique<Expression>(to_mpq_map(addends));
           }),
           py::arg("addends"))
      .def_property_readonly("variables", &Expression::variables)
      .def("add", &Expression::Add, py::arg("var"), py::arg("coeff"))
      .def("subtract", &Expression::Subtract, py::arg("var"), py::arg("coeff"))
      .def(
          "evaluate",
          [](const Expression &self, const std::map<Variable, double> &env) {
            return self.Evaluate(to_mpq_map(env)).get_d();
          },
          py::arg("env"))
      .def("substitute", &Expression::Substitute, py::arg("expr_subst"))
      .def("equal_to", &Expression::equal_to, py::arg("o"))
      .def("less", &Expression::less, py::arg("o"))
      .def("__str__", STR_LAMBDA(Expression))
      .def("__repr__", REPR_LAMBDA(Expression))
      .def("__hash__", &Expression::hash)
      // Addition
      .def(py::self + py::self)
      .def(py::self + Variable())
      .def(Variable() + py::self)
      .def(py::self += py::self)
      .def(py::self += Variable())
      // Subtraction.
      .def(py::self - py::self)
      .def(py::self - Variable())
      .def(Variable() - py::self)
      .def(py::self -= py::self)
      .def(py::self -= Variable())
      // Multiplication.
      .def(py::self * double())
      .def(double() * py::self)
      .def(py::self *= double())
      // Division.
      .def(py::self / double())
      .def(py::self /= double())
      // Unary Plus.
      .def(+py::self)
      // Unary Minus.
      .def(-py::self)
      // LT(<).
      //
      // Note that for `double < Expression` case, the reflected op ('>' in this
      // case) is called. For example, `1 < x * y` will return `x * y > 1`.
      .def(py::self < py::self)
      .def(py::self < Variable())
      .def(py::self < double())
      // LE(<=).
      .def(py::self <= py::self)
      .def(py::self <= Variable())
      .def(py::self <= double())
      // GT(>).
      .def(py::self > py::self)
      .def(py::self > Variable())
      .def(py::self > double())
      // GE(>=).
      .def(py::self >= py::self)
      .def(py::self >= Variable())
      .def(py::self >= double())
      // EQ(==).
      .def(py::self == py::self)
      .def(py::self == Variable())
      .def(py::self == double())
      // NE(!=)
      .def(py::self != py::self)
      .def(py::self != Variable())
      .def(py::self != double());

  py::enum_<FormulaKind>(m, "FormulaKind")
      .value("EQ", FormulaKind::Eq)
      .value("NEQ", FormulaKind::Neq)
      .value("GT", FormulaKind::Gt)
      .value("GEQ", FormulaKind::Geq)
      .value("LT", FormulaKind::Lt)
      .value("LEQ", FormulaKind::Leq);

  FormulaClass.def(py::init<Expression, FormulaKind, double>(), py::arg("lhs"), py::arg("kind"), py::arg("rhs"))
      .def_property_readonly("variables", &Formula::variables)
      .def("equal_to", &Formula::equal_to, py::arg("o"))
      .def("less", &Formula::less, py::arg("o"))
      .def(
          "evaluate",
          [](const Formula &self, const std::map<Variable, double> &env) { return self.Evaluate(to_mpq_map(env)); },
          py::arg("env"))
      .def("substitute", &Formula::Substitute, py::arg("expr_subst"))
      .def("__str__", STR_LAMBDA(Formula))
      .def("__repr__", REPR_LAMBDA(Formula))
      .def("__hash__", &Formula::hash)
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self < py::self)
      .def(py::self <= py::self)
      .def(py::self > py::self)
      .def(py::self >= py::self);
}
