/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence Apache-2.0 license
 */
#include "ExpressionCell.h"

#include <utility>

#include "delpi/util/error.h"
#include "delpi/util/hash.hpp"

namespace delpi {
intrusive_ptr<ExpressionCell> ExpressionCell::New() { return intrusive_ptr(new ExpressionCell()); }
intrusive_ptr<ExpressionCell> ExpressionCell::New(Variable var) {
  return intrusive_ptr(new ExpressionCell{std::move(var)});
}
intrusive_ptr<ExpressionCell> ExpressionCell::New(LinearMonomial linear_monomial) {
  return intrusive_ptr(new ExpressionCell{std::move(linear_monomial)});
}
intrusive_ptr<ExpressionCell> ExpressionCell::New(Addends addends) {
  return intrusive_ptr(new ExpressionCell{std::move(addends)});
}
intrusive_ptr<ExpressionCell> ExpressionCell::Copy(const ExpressionCell& o) {
  return {intrusive_ptr(new ExpressionCell{o.addends_})};
}

ExpressionCell::ExpressionCell(Variable var) : hash_{0} { addends_.emplace(std::move(var), 1); }
ExpressionCell::ExpressionCell(LinearMonomial linear_monomial) : hash_{0} {
  addends_.emplace(std::move(linear_monomial.var), std::move(linear_monomial.coeff));
}
ExpressionCell::ExpressionCell(Addends addends) : hash_{0}, addends_{std::move(addends)} {}

std::vector<Variable> ExpressionCell::variables() const {
  std::vector<Variable> vars;
  vars.reserve(addends_.size());
  for (auto& [var, coeff] : addends_) vars.emplace_back(var);
  return vars;
}

bool ExpressionCell::equal_to(const ExpressionCell& o) const noexcept {
  if (this == &o) return true;
  return std::ranges::equal(
      addends_, o.addends_,
      [](const std::pair<const Variable, mpq_class>& p1, const std::pair<const Variable, mpq_class>& p2) {
        return p1.first.equal_to(p2.first) && p1.second == p2.second;
      });
}
bool ExpressionCell::less(const ExpressionCell& o) const noexcept {
  // Compare the two maps.
  if (this == &o) return false;
  return std::ranges::lexicographical_compare(
      addends_, o.addends_,
      [](const std::pair<const Variable, mpq_class>& p1, const std::pair<const Variable, mpq_class>& p2) {
        const auto& [var1, val1] = p1;
        const auto& [var2, val2] = p2;
        if (var1.less(var2)) return true;
        if (var2.less(var1)) return false;
        return val1 < val2;
      });
}
std::size_t ExpressionCell::hash() const noexcept {
  if (hash_ == 0) hash_ = hash::hash_value<Addends>{}(addends_);
  return hash_;
}

ExpressionCell& ExpressionCell::Add(const Variable& var, const mpq_class& coeff) {
  if (coeff == 0) return *this;
  hash_ = 0;
  if (const auto it = addends_.find(var); addends_.end() == it) {
    addends_.emplace(var, coeff);
  } else {
    mpq_class new_coeff{coeff + it->second};
    if (0 == new_coeff) {
      addends_.erase(it);
    } else {
      it->second = std::move(new_coeff);
    }
  }
  return *this;
}
ExpressionCell& ExpressionCell::Multiply(const mpq_class& coeff) {
  if (coeff == 1) return *this;
  hash_ = 0;
  if (coeff == 0) addends_.clear();
  for (auto& it : addends_) it.second *= coeff;
  return *this;
}
ExpressionCell& ExpressionCell::Divide(const mpq_class& coeff) {
  if (coeff == 1) return *this;
  if (coeff == 0) DELPI_RUNTIME_ERROR("Division by 0");
  hash_ = 0;
  for (auto& it : addends_) it.second /= coeff;
  return *this;
}

mpq_class ExpressionCell::Evaluate(const Environment& env) const {
  return std::accumulate(addends_.begin(), addends_.end(), mpq_class{0},
                         [&env](const mpq_class& init, const std::pair<const Variable, mpq_class>& p) {
                           // Without the cast, it would return an expression template
                           return static_cast<mpq_class>(init + env.at(p.first) * p.second);
                         });
}
Expression ExpressionCell::Substitute(const SubstitutionMap& s) const {
  Expression ret{};
  for (const auto& [var, coeff] : addends_) {
    ret.Add(s.contains(var) ? s.at(var) : var, coeff);
  }
  return ret;
}

std::ostream& ExpressionCell::Print(std::ostream& os) const {
  bool print_plus{false};
  os << "(";
  for (auto& [var, coeff] : addends_) {
    PrintAddend(os, print_plus, var, coeff);
    print_plus = true;
  }
  os << ")";
  return os;
}
std::ostream& ExpressionCell::PrintAddend(std::ostream& os, const bool print_plus, const Variable& var,
                                          const mpq_class& coeff) {
  if (coeff > 0.0) {
    if (print_plus) {
      os << " + ";
    }
    // Do not print "1 * t"
    if (coeff != 1.0) {
      os << coeff << " * ";
    }
  } else {
    // Instead of printing "+ (- E)", just print "- E".
    os << " - ";
    if (coeff != -1.0) {
      os << (-coeff) << " * ";
    }
  }
  return os << var;
}

}  // namespace delpi