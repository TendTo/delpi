/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "Expression.h"

#include <sstream>
#include <utility>

namespace delpi {

Expression::Expression(Addends addends) : addends_(std::move(addends)) {}

std::vector<Variable> Expression::variables() const {
  std::vector<Variable> vars;
  vars.reserve(addends_.size());
  for (auto& [var, val] : addends_) vars.emplace_back(var);
  return vars;
}

bool Expression::equal_to(const Expression& e) const {
  return std::ranges::equal(
      addends_, e.addends_,
      [](const std::pair<const Variable, mpq_class>& p1, const std::pair<const Variable, mpq_class>& p2) {
        return p1.first.equal_to(p2.first) && p1.second == p2.second;
      });
}
bool Expression::less(const Expression& e) const {
  // Compare the two maps.
  return std::ranges::lexicographical_compare(
      addends_, e.addends_,
      [](const std::pair<const Variable, mpq_class>& p1, const std::pair<const Variable, mpq_class>& p2) {
        const auto& [var1, val1] = p1;
        const auto& [var2, val2] = p2;
        if (var1.less(var2)) return true;
        if (var2.less(var1)) return false;
        return val1 < val2;
      });
}
mpq_class Expression::Evaluate(const Environment& env) const {
  return std::accumulate(addends_.begin(), addends_.end(), 0,
                         [&env](const mpq_class& init, const std::pair<const Variable, mpq_class>& p) {
                           // Without the cast, it would return an expression template
                           return static_cast<mpq_class>(init + env.at(p.first) * p.second);
                         });
}
Expression Expression::Substitute(const std::unordered_map<Variable, Variable>& s) const {
  Expression ret{};
  for (const auto& [var, val] : addends_) {
    ret.Add(s.contains(var) ? s.at(var) : var, val);
  }
  return ret;
}
std::string Expression::ToString() const { return (std::stringstream{} << *this).str(); }

Expression& Expression::operator*=(const mpq_class& o) {
  for (auto& [var, val] : addends_) val *= o;
  return *this;
}
Expression& Expression::operator/=(const mpq_class& o) {
  for (auto& [var, val] : addends_) val /= o;
  return *this;
}
Expression Expression::operator*(const mpq_class& o) const {
  Expression temp{*this};
  return temp *= o;
}
Expression Expression::operator/(const mpq_class& o) const {
  Expression temp{*this};
  return temp /= o;
}

Expression& Expression::Add(const Variable& var, const mpq_class& val) {
  if (const auto it = addends_.find(var); addends_.end() == it) {
    addends_.emplace(var, val);
  } else {
    it->second = val + it->second;
  }
  return *this;
}
Expression& Expression::Subtract(const Variable& var, const mpq_class& val) { return Add(var, -val); }

Expression& Expression::operator+=(const Variable& o) { return Add(o, 1); }
Expression& Expression::operator-=(const Variable& o) { return Subtract(o, 1); }
Expression Expression::operator+(const Variable& o) const {
  Expression temp{*this};
  return temp += o;
}

Expression Expression::operator-(const Variable& o) const {
  Expression temp{*this};
  return temp -= o;
}

Expression& Expression::operator+=(const LinearMonomial& o) { return Add(o.var, o.val); }
Expression& Expression::operator-=(const LinearMonomial& o) { return Subtract(o.var, o.val); }
Expression Expression::operator+(const LinearMonomial& o) const {
  Expression temp{*this};
  return temp += o;
}

Expression Expression::operator-(const LinearMonomial& o) const {
  Expression temp{*this};
  return temp -= o;
}

Expression& Expression::operator+=(const Expression& o) {
  for (const auto& [var, val] : o.addends_) Add(var, val);
  return *this;
}
Expression& Expression::operator-=(const Expression& o) {
  for (const auto& [var, val] : o.addends_) Subtract(var, val);
  return *this;
}
Expression Expression::operator+(const Expression& o) const {
  Expression temp{*this};
  return temp += o;
}

Expression Expression::operator-(const Expression& o) const {
  Expression temp{*this};
  return temp -= o;
}

Expression operator*(const mpq_class& o, const Expression& e) {
  Expression temp{e};
  return temp *= o;
}
Expression operator+(const Variable& o, const Expression& e) {
  Expression temp(e);
  return temp += o;
}
Expression operator-(const Variable& o, const Expression& e) {
  Expression temp(e);
  return temp -= o;
}
Expression operator+(const LinearMonomial& o, const Expression& e) {
  Expression temp(e);
  return temp += o;
}
Expression operator-(const LinearMonomial& o, const Expression& e) {
  Expression temp(e);
  return temp -= o;
}

}  // namespace delpi