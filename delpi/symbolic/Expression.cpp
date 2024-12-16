/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/symbolic/Expression.h"

#include <sstream>
#include <utility>
#include <vector>

#include "delpi/symbolic/ExpressionCell.h"
#include "delpi/util/error.h"

namespace delpi {

Expression::Expression() : ptr_(ExpressionCell::New()) {}
Expression::Expression(Variable var) : ptr_(ExpressionCell::New(std::move(var))) {}
Expression::Expression(Addend addend) : ptr_(ExpressionCell::New(std::move(addend))) {}
Expression::Expression(Addends addends) : ptr_{ExpressionCell::New(std::move(addends))} {}
Expression::Expression(std::vector<Addend> addends)
    : ptr_{ExpressionCell::New(Addends{addends.begin(), addends.end()})} {}
Expression::Expression(const Expression& e) : ptr_{e.ptr_} {}
Expression::Expression(Expression&& e) noexcept : ptr_{std::move(e.ptr_)} {}
Expression& Expression::operator=(const Expression& e) {
  ptr_ = e.ptr_;
  return *this;
}
Expression& Expression::operator=(Expression&& e) noexcept {
  ptr_ = std::move(e.ptr_);
  return *this;
}
Expression::~Expression() {}

std::vector<Variable> Expression::variables() const { return ptr_->variables(); }
const Expression::Addends& Expression::addends() const { return ptr_->addends(); }

bool Expression::equal_to(const Expression& o) const noexcept { return ptr_->equal_to(*o.ptr_); }
bool Expression::less(const Expression& o) const noexcept { return ptr_->less(*o.ptr_); }
std::size_t Expression::hash() const noexcept { return ptr_->hash(); }
template <MapFromTo<Variable, mpq_class> T>
mpq_class Expression::Evaluate(const T& env) const {
  return ptr_->Evaluate(env);
}
Expression Expression::Substitute(const SubstitutionMap& s) const { return ptr_->Substitute(s); }
std::string Expression::ToString() const { return (std::stringstream{} << *this).str(); }
std::ostream& Expression::Print(std::ostream& os) const { return ptr_->Print(os); }
std::size_t Expression::use_count() const { return ptr_->use_count(); }

Expression& Expression::operator*=(const mpq_class& o) {
  if (o == 1) return *this;
  if (ptr_->use_count() != 1) ptr_ = ExpressionCell::Copy(*ptr_);
  DELPI_ASSERT(ptr_->use_count() == 1, "The expression must be the only owner to modify its expression cell");
  ptr_->Multiply(o);
  return *this;
}
Expression& Expression::operator/=(const mpq_class& o) {
  if (o == 1) return *this;
  if (ptr_->use_count() != 1) ptr_ = ExpressionCell::Copy(*ptr_);
  DELPI_ASSERT(ptr_->use_count() == 1, "The expression must be the only owner to modify its expression cell");
  ptr_->Divide(o);
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

Expression& Expression::Add(const Variable& var, const mpq_class& coeff) {
  if (coeff == 0) return *this;
  if (ptr_->use_count() != 1) ptr_ = ExpressionCell::Copy(*ptr_);
  DELPI_ASSERT(ptr_->use_count() == 1, "The expression must be the only owner to modify its expression cell");
  ptr_->Add(var, coeff);
  return *this;
}

Expression& Expression::Subtract(const Variable& var, const mpq_class& coeff) { return Add(var, -coeff); }

Expression Expression::operator-() const { return *this * -1; }
Expression Expression::operator+() const { return *this; }

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

Expression& Expression::operator+=(const Addend& o) { return Add(o.first, o.second); }
Expression& Expression::operator-=(const Addend& o) { return Subtract(o.first, o.second); }
Expression Expression::operator+(const Addend& o) const {
  Expression temp{*this};
  return temp += o;
}

Expression Expression::operator-(const Addend& o) const {
  Expression temp{*this};
  return temp -= o;
}

Expression& Expression::operator+=(const Expression& o) {
  for (const auto& [var, coeff] : o.addends()) Add(var, coeff);
  return *this;
}
Expression& Expression::operator-=(const Expression& o) {
  for (const auto& [var, coeff] : o.addends()) Subtract(var, coeff);
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

Expression operator-(const Variable& var) { return Expression{Expression::Addend{var, -1}}; }

Expression operator*(const mpq_class& lhs, const Expression& rhs) {
  Expression temp{rhs};
  return temp *= lhs;
}
Expression operator*(const mpq_class& lhs, const Variable& rhs) {
  Expression temp{rhs};
  return temp *= lhs;
}
Expression operator*(const Variable& lhs, const mpq_class& rhs) {
  Expression temp{lhs};
  return temp *= rhs;
}
Expression operator/(const Variable& lhs, const mpq_class& rhs) {
  Expression temp{lhs};
  return temp /= rhs;
}
Expression operator+(const Variable& lhs, const Expression& rhs) {
  Expression temp(rhs);
  return temp += lhs;
}
Expression operator-(const Variable& lhs, const Expression& rhs) {
  Expression temp(rhs);
  return temp -= lhs;
}
Expression operator+(const Expression::Addend& lhs, const Expression& rhs) {
  Expression temp(rhs);
  return temp += lhs;
}
Expression operator-(const Expression::Addend& lhs, const Expression& rhs) {
  Expression temp(rhs);
  return temp -= lhs;
}

std::ostream& operator<<(std::ostream& os, const Expression& e) { return e.Print(os); }

template mpq_class Expression::Evaluate(const std::map<Variable, mpq_class>& env) const;
template mpq_class Expression::Evaluate(const std::unordered_map<Variable, mpq_class>& env) const;

}  // namespace delpi
