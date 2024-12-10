/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Variable class.
 */
#pragma once

#include <cstddef>
#include <iosfwd>
#include <limits>
#include <set>
#include <string>
#include <vector>

namespace delpi {

/**
 * Real symbolic variable
 */
class Variable {
 public:
  using Id = std::size_t;

  /**
   * Construct a new dummy variable object.
   *
   * The default constructor is needed to support some data structures.
   * The objects created by the default constructor share the same ID, `std::numeric_limits<Id>::max()`.
   * As a result, they all are identified as a single variable by equality operator (==) and have the same hash value as
   * well. It is allowed to construct a dummy variable, but it should not be used to construct a symbolic expression.
   */
  Variable() : id_{std::numeric_limits<Id>::max()} {}

  /**
   * Construct a new real variable object, assigning it a @p name.
   *
   * It will be given a unique incremental ID.
   * @param name name of the variable
   */
  explicit Variable(std::string name);

  /** @checker{a dummy\, i.e. has been created with the default constructor, variable} */
  [[nodiscard]] bool is_dummy() const { return id_ == std::numeric_limits<Id>::max(); }
  /** @getter{id, variable} */
  [[nodiscard]] Id id() const { return id_; }
  /** @getter{name, variable} */
  [[nodiscard]] const std::string &name() const { return names_[id_ + 1]; }

  /** @equal_to{variable, Two variables are the same if their @ref id_ is the same, regardless of their name.} */
  [[nodiscard]] bool equal_to(const Variable &o) const noexcept { return id_ == o.id_; }
  /** @less{variable, The ordering is based on the ID of the variable.} */
  [[nodiscard]] bool less(const Variable &o) const noexcept { return id_ < o.id_; }
  /** @hash{variable, The hash is based on the ID of the variable.} */
  [[nodiscard]] size_t hash() const noexcept { return std::hash<Id>{}(id_); }

 private:
  static std::vector<std::string> names_;  ///< Names of all existing variables.
  /**
   * Get the next unique identifier for a variable.
   * @return incremental unique identifier
   */
  static Id GetNextId();

  Id id_{};  ///< Unique identifier.
};

std::ostream &operator<<(std::ostream &os, const Variable &var);

using VariableSet = std::set<Variable>;

}  // namespace delpi

template <>
struct std::hash<delpi::Variable> {
  size_t operator()(const delpi::Variable &v) const noexcept { return v.hash(); }
};

template <>
struct std::less<delpi::Variable> {
  bool operator()(const delpi::Variable &lhs, const delpi::Variable &rhs) const noexcept { return lhs.less(rhs); }
};

template <>
struct std::equal_to<delpi::Variable> {
  bool operator()(const delpi::Variable &lhs, const delpi::Variable &rhs) const noexcept { return lhs.equal_to(rhs); }
};
