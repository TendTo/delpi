/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * VariableSet class.
 */
#pragma once

#include <iosfwd>
#include <limits>
#include <vector>

#include "delpi/symbolic/Variable.h"
#include "delpi/util/concepts.h"

namespace delpi {

/**
 * Set of variables optimised for fast access at the cost of memory.
 * Instead of using a hash set, it uses a vector of booleans to track the presence of each variable.
 * The vector contains @f$ m @f$ elements, where @f$ m @f$ is the number obtained by subtracting the minimum id
 * of the variables in the set from the maximum id plus 1, i.e. @f$ m = \text{max_id} - \text{min_id} + 1 @f$.
 * Hence, the structure will be most efficient, both in terms of memory and speed,
 * when the variables are contiguous and have a small range of ids,
 * but it will be extremely inefficient in terms of space when the variables are sparse with respect to their ids.
 * This also effects computing properties over the whole set of variables.
 * For instance, checking if the set is empty or iterating over all the variables is linear in @f$ m @f$,
 * not in the number of variables.
 * @tip If you know the variable indexes are sparse, use a standard set instead.
 */
class VariableSet {
 public:
  using Id = Variable::Id;

  /** @constructor{variable set} */
  VariableSet() : min_id_{std::numeric_limits<Id>::max()} {}
  /**
   * Construct a new variable set from a range of `vars`.
   * @tparam T generic iterable containing variables
   * @param vars range of variables to add to the set
   */
  template <TypedIterable<Variable> T>
  explicit VariableSet(const T& vars) : VariableSet{} {
    Insert(vars);
  }
  /**
   * Construct a new variable set from a sequence of variables.
   * @tip This method will add the variables in the set one by one.
   * Using @ref VariableSet(T) can be more efficient for large sets.
   * @tparam Vars sequence of variables to add to the set
   * @param var first variable to add to the set
   * @param vars remaining variables to add to the set
   */
  template <std::same_as<Variable>... Vars>
  explicit VariableSet(const Variable& var, const Vars&... vars) : VariableSet{} {
    Insert(var, vars...);
  }

  /** @getter{min_id, variable set} */
  [[nodiscard]] Id min_id() const { return min_id_; }
  /** @getter{max_id, variable set} */
  [[nodiscard]] Id max_id() const { return vars_.size() + min_id_; }
  /** @getter{number of variables, variable set, The computation to obtain this value is linear in @f$ m @f$.} */
  [[nodiscard]] std::size_t size() const;
  /** @getter{size of @ref vars_, variable set} */
  [[nodiscard]] std::size_t capacity() const { return vars_.size(); }
  /** @checked{empty, variable set} */
  [[nodiscard]] bool empty() const;
  /** @getter{all added variables, variable set} */
  [[nodiscard]] std::vector<Variable> variables() const;

  /**
   * Use the `id` to get the variable in the set, if it exists.
   * @param id id of the variable to get
   * @return variable with the given id, if it exists
   * @throws DelpiOutOfRangeException if the id is not in the set
   */
  [[nodiscard]] Variable Get(Id id) const;
  /**
   * Check if the `var` is in the set.
   * @param var variable to check
   * @return true if the variable is in the set
   * @return false if the variable is not in the set
   */
  [[nodiscard]] bool Contains(const Variable& var) const { return Contains(var.id()); }
  /**
   * Insert a `var` to the set.
   * If the variable is already in the set, nothing happens.
   * @param var variable to add
   * @return true if the variable was added
   * @return false if the variable was already in the set
   */
  bool Insert(const Variable& var) { return Insert(var.id()); }
  /**
   * Insert a range of `vars` to the set.
   * Only variables not already in the set will be added.
   * @tparam T generic iterable containing variables
   * @param vars range of variables to add
   */
  template <TypedIterable<Variable> T>
  void Insert(const T& vars) {
    using Comparator = std::less<Variable>;
    // If the set is empty, initialise it with min_id_ = min(vars.id) and size m = max(vars.id) - min_id_ + 1
    if (min_id_ == std::numeric_limits<Id>::max()) {
      min_id_ = std::ranges::min_element(vars, Comparator{})->id();
      vars_.resize(std::ranges::max_element(vars, Comparator{})->id() - min_id_ + 1, false);
      for (const Variable var : vars) vars_.at(var.id() - min_id_) = true;
      return;
    }
    // If the set is not empty, resize it to the new range and insert the variables
    // First compute the new min_id and max_id comparing with the current min_id and max_id
    // Then resize the vector to the new range m = new_max - new_min + 1
    const Id new_min = std::min(std::ranges::min_element(vars, Comparator{})->id(), min_id_);
    const Id new_max = std::max(std::ranges::max_element(vars, Comparator{})->id(), vars_.size() + min_id_ - 1);
    const std::size_t max_diff = new_max - (vars_.size() + min_id_ - 1);
    const std::size_t old_size = vars_.size();
    vars_.resize(new_max - new_min + 1);
    // If the lower bound of the new range is less than the current min_id, rotate the vector to the left by max_diff
    if (new_min < min_id_) {
      std::rotate(vars_.begin(), vars_.begin() + static_cast<long>(old_size),
                  vars_.end() - static_cast<long>(max_diff));
      min_id_ = new_min;
    }
    for (const Variable var : vars) vars_.at(var.id() - min_id_) = true;
  }
  /**
   * Insert a sequence of variables to the set.
   * Only variables not already in the set will be added.
   * @tip This method will add the variables in the set one by one.
   * Using @ref Insert(T) can be more efficient for large sets.
   * @tparam Vars sequence of variables to add
   * @param var first variable to add
   * @param vars remaining variables to add
   */
  template <std::same_as<Variable>... Vars>
  void Insert(const Variable& var, const Vars&... vars) {
    Insert(var);
    Insert(vars...);
  }
  /**
   * Remove a `var` from the set.
   * If the variable is not in the set, nothing happens.
   * @param var variable to remove
   * @return true if the variable was removed
   * @return false if the variable was not in the set
   */
  bool Remove(const Variable& var) { return Remove(var.id()); }
  /**
   * Remove a range of `vars` from the set.
   * Only variables in the set will be removed.
   * @tparam T generic iterable containing variables
   * @param vars range of variables to remove
   */
  template <TypedIterable<Variable> T>
  void Remove(const T& vars) {
    for (const Variable& var : vars) Remove(var);
  }
  /**
   * Remove a sequence of variables from the set.
   * Only variables in the set will be removed.
   * @tparam Vars sequence of variables to remove
   * @param var first variable to remove
   * @param vars remaining variables to remove
   */
  template <std::same_as<Variable>... Vars>
  void Remove(const Variable& var, const Vars&... vars) {
    Remove(var);
    Remove(vars...);
  }
  /** Clear the set of all variables. */
  void Clear();

 private:
  /**
   * Check if the variable with the given `id` is in the set.
   * @param id id of the variable to check
   * @return true if the variable is in the set
   * @return false if the variable is not in the set
   */
  [[nodiscard]] bool Contains(Id id) const;
  /**
   * Insert a variable with the given `id` to the set.
   * @param id id of the variable to add
   * @return true if the variable was added
   * @return false if the variable was already in the set
   */
  bool Insert(Id id);
  /**
   * Remove a variable with the given `id` from the set.
   * @param id id of the variable to remove
   * @return true if the variable was removed
   * @return false if the variable was not in the set
   */
  bool Remove(Id id);

  Id min_id_;               ///< Minimum id of the variables in the set.
  std::vector<bool> vars_;  ///< Vector tracking the presence of each variable in the set.
};

std::ostream& operator<<(std::ostream& os, const VariableSet& var_set);

}  // namespace delpi
