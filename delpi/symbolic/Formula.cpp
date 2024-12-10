#include "Formula.h"

#include <ostream>
#include <utility>

#include "delpi/util/error.h"
#include "delpi/util/hash.hpp"

namespace delpi {

Formula::Formula(Expression expression, const FormulaKind kind, mpq_class rhs)
    : expression_{std::move(expression)}, kind_{kind}, rhs_{std::move(rhs)} {}

Formula Formula::Substitute(const Expression::SubstitutionMap& s) const {
  return Formula{expression_.Substitute(s), kind_, rhs_};
}
bool Formula::Evaluate(const Expression::Environment& env) const {
  const mpq_class value{expression_.Evaluate(env)};
  switch (kind_) {
    case FormulaKind::Eq:
      return value == rhs_;
    case FormulaKind::Neq:
      return value != rhs_;
    case FormulaKind::Lt:
      return value < rhs_;
    case FormulaKind::Leq:
      return value <= rhs_;
    case FormulaKind::Gt:
      return value > rhs_;
    case FormulaKind::Geq:
      return value >= rhs_;
    default:
      DELPI_UNREACHABLE();
  }
}

bool Formula::equal_to(const Formula& o) const noexcept {
  if (this == &o) return true;
  if (kind_ != o.kind_) return false;
  if (rhs_ != o.rhs_) return false;
  return expression_.equal_to(o.expression_);
}
bool Formula::less(const Formula& o) const noexcept {
  if (this == &o) return false;
  if (kind_ != o.kind_) return kind_ < o.kind_;
  if (rhs_ != o.rhs_) return rhs_ < o.rhs_;
  return expression_.less(o.expression_);
}
std::size_t Formula::hash() const noexcept { return hash::hash_combine(expression_.hash(), kind_, rhs_); }

Formula Formula::operator-() const { return Formula{-expression_, -kind_, -rhs_}; }
Formula Formula::operator!() const { return Formula{expression_, !kind_, rhs_}; }
std::strong_ordering Formula::operator<=>(const Formula& o) const {
  if (less(o)) return std::strong_ordering::less;
  if (o.less(*this)) return std::strong_ordering::greater;
  return std::strong_ordering::equal;
}

Formula operator==(const Variable& lhs, const Variable& rhs) { return Formula{lhs - rhs, FormulaKind::Eq, 0}; }
Formula operator!=(const Variable& lhs, const Variable& rhs) { return Formula{lhs - rhs, FormulaKind::Neq, 0}; }
Formula operator<(const Variable& lhs, const Variable& rhs) { return Formula{lhs - rhs, FormulaKind::Lt, 0}; }
Formula operator<=(const Variable& lhs, const Variable& rhs) { return Formula{lhs - rhs, FormulaKind::Leq, 0}; }
Formula operator>(const Variable& lhs, const Variable& rhs) { return Formula{lhs - rhs, FormulaKind::Gt, 0}; }
Formula operator>=(const Variable& lhs, const Variable& rhs) { return Formula{lhs - rhs, FormulaKind::Geq, 0}; }

Formula operator==(const Expression& lhs, const Expression& rhs) { return Formula{lhs - rhs, FormulaKind::Eq, 0}; }
Formula operator!=(const Expression& lhs, const Expression& rhs) { return Formula{lhs - rhs, FormulaKind::Neq, 0}; }
Formula operator<(const Expression& lhs, const Expression& rhs) { return Formula{lhs - rhs, FormulaKind::Lt, 0}; }
Formula operator<=(const Expression& lhs, const Expression& rhs) { return Formula{lhs - rhs, FormulaKind::Leq, 0}; }
Formula operator>(const Expression& lhs, const Expression& rhs) { return Formula{lhs - rhs, FormulaKind::Gt, 0}; }
Formula operator>=(const Expression& lhs, const Expression& rhs) { return Formula{lhs - rhs, FormulaKind::Geq, 0}; }

Formula operator==(mpq_class lhs, const Expression& rhs) { return Formula{rhs, FormulaKind::Eq, std::move(lhs)}; }
Formula operator!=(mpq_class lhs, const Expression& rhs) { return Formula{rhs, FormulaKind::Neq, std::move(lhs)}; }
Formula operator<(mpq_class lhs, const Expression& rhs) { return Formula{rhs, FormulaKind::Gt, std::move(lhs)}; }
Formula operator<=(mpq_class lhs, const Expression& rhs) { return Formula{rhs, FormulaKind::Geq, std::move(lhs)}; }
Formula operator>(mpq_class lhs, const Expression& rhs) { return Formula{rhs, FormulaKind::Lt, std::move(lhs)}; }
Formula operator>=(mpq_class lhs, const Expression& rhs) { return Formula{rhs, FormulaKind::Leq, std::move(lhs)}; }

Formula operator==(const Expression& lhs, mpq_class rhs) { return Formula{lhs, FormulaKind::Eq, std::move(rhs)}; }
Formula operator!=(const Expression& lhs, mpq_class rhs) { return Formula{lhs, FormulaKind::Neq, std::move(rhs)}; }
Formula operator<(const Expression& lhs, mpq_class rhs) { return Formula{lhs, FormulaKind::Lt, std::move(rhs)}; }
Formula operator<=(const Expression& lhs, mpq_class rhs) { return Formula{lhs, FormulaKind::Leq, std::move(rhs)}; }
Formula operator>(const Expression& lhs, mpq_class rhs) { return Formula{lhs, FormulaKind::Gt, std::move(rhs)}; }
Formula operator>=(const Expression& lhs, mpq_class rhs) { return Formula{lhs, FormulaKind::Geq, std::move(rhs)}; }

std::ostream& operator<<(std::ostream& os, const Formula& formula) {
  return os << "(" << formula.expression() << " " << formula.kind() << " " << formula.rhs() << ")";
}

}  // namespace delpi