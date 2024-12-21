/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */

#include "delpi/symbolic/VariableSet.h"

#include <ostream>

#include "delpi/util/error.h"

#define VarSetIndex(var) (var.id() - min_id_)
#define IdSetIndex(id) (id - min_id_)

namespace delpi {

bool VariableSet::Insert(const Id id) {
  if (min_id_ == std::numeric_limits<Id>::max()) {
    DELPI_ASSERT(vars_.empty(), "VariableSet is not empty but min_id is not set");
    min_id_ = id;
    vars_.emplace_back(true);
    return true;
  }
  if (id < min_id_) {
    vars_.resize(vars_.size() + min_id_ - id, false);
    std::rotate(vars_.begin(), vars_.begin() + static_cast<long>(vars_.size() - (min_id_ - id)), vars_.end());
    min_id_ = id;
    vars_.front() = true;
    return true;
  }
  if (id >= min_id_ + vars_.size()) {
    vars_.resize(id - min_id_ + 1);
    vars_.back() = true;
    return true;
  }
  const bool was_present = vars_.at(IdSetIndex(id));
  vars_.at(IdSetIndex(id)) = true;
  return !was_present;
}
std::size_t VariableSet::size() const { return std::ranges::count(vars_, true); }
bool VariableSet::empty() const {
  return std::ranges::all_of(vars_, [](const bool var) { return !var; });
}
std::vector<Variable> VariableSet::variables() const {
  std::vector<Variable> vars;
  vars.reserve(vars_.size());
  for (std::size_t i = 0; i < vars_.size(); ++i) {
    if (vars_.at(i)) vars.emplace_back(min_id_ + i);
  }
  return vars;
}
Variable VariableSet::Get(Id id) const {
  if (Contains(id)) return Variable{id};
  DELPI_OUT_OF_RANGE_FMT("Variable with id {} not found in the set", id);
}
bool VariableSet::Contains(const Id id) const {
  return id >= min_id_ && IdSetIndex(id) < vars_.size() && vars_.at(IdSetIndex(id));
}
bool VariableSet::Remove(const Id id) {
  if (!Contains(id)) return false;
  vars_.at(IdSetIndex(id)) = false;
  return true;
}
void VariableSet::Clear() {
  vars_.clear();
  min_id_ = std::numeric_limits<Id>::max();
}

std::ostream& operator<<(std::ostream& os, const VariableSet& var_set) {
  os << "{";
  bool is_first = true;
  for (const Variable var : var_set.variables()) {
    os << (is_first ? "" : ", ") << var;
    is_first = false;
  }
  return os << "}";
}

}  // namespace delpi
