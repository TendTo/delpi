/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * VariableMap class.
 */
#pragma once

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "delpi/symbolic/Variable.h"
#include "delpi/util/concepts.h"
#include "delpi/util/exception.h"

namespace delpi {

/**
 * Map from variables to an arbitrary type T optimised for fast access at the cost of memory.
 * Instead of using a hash map, it uses a vector of std::optional values to track the value each variable maps to.
 * The vector contains @f$ m @f$ elements, where @f$ m @f$ is the number obtained by subtracting the minimum id
 * of the variables in the map from the maximum id plus 1, i.e. @f$ m = \text{max_id} - \text{min_id} + 1 @f$.
 * Hence, the structure will be most efficient, both in terms of memory and speed,
 * when the variables are contiguous and have a small range of ids,
 * but it will be extremely inefficient in terms of space when the variables are sparse with respect to their ids.
 * This also effects computing properties over the whole map of variables.
 * For instance, checking if the map is empty or iterating over all the entries is linear in @f$ m @f$,
 * not in the number of entries.
 * @tip If you know the variable indexes are sparse, use a standard map instead.
 */
template <class T>
class VariableMap {
  static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");

 public:
  using Id = Variable::Id;
  using key_type = Variable;
  using mapped_type = T;
  using value_type = std::pair<const Variable, T>;

  /** @constructor{variable map} */
  VariableMap() : min_id_{std::numeric_limits<Id>::max()} {}
  /**
   * Construct a new variable map from a range of `items`.
   * @tparam Items key-value pairs to add to the map
   * @param items range of key-value pairs to add to the map
   */
  template <TypedIterable<value_type> Items>
  explicit VariableMap(const Items& items) : VariableMap{} {
    Insert(items);
  }

  /** @getter{min_id, variable map} */
  [[nodiscard]] Id min_id() const { return min_id_; }
  /** @getter{max_id, variable map} */
  [[nodiscard]] Id max_id() const { return vars_.size() + min_id_; }
  /** @getter{number of variables, variable map, The computation to obtain this value is linear in @f$ m @f$.} */
  [[nodiscard]] std::size_t size() const {
    return std::ranges::count_if(vars_, [](const std::optional<mapped_type>& item) { return item.has_value(); });
  }
  /** @getter{size of @ref vars_, variable map} */
  [[nodiscard]] std::size_t capacity() const { return vars_.size(); }
  /** @checked{empty, variable map} */
  [[nodiscard]] bool empty() const { return vars_.empty(); }
  /** @getter{all key-value pairs, variable map} */
  [[nodiscard]] std::vector<mapped_type> items() const {
    std::vector<mapped_type> items;
    items.reserve(vars_.size());
    for (const std::optional<mapped_type>& item : vars_) {
      if (item.has_value()) items.emplace_back(item.value());
    }
    return items;
  }

  /**
   * Use the `var` to get the value it is mapped to, if it exists.
   * @param var variable whose value to get
   * @return value mapped to the variable, if it exists
   * @throws DelpiOutOfRangeException if the `var` is not in the map
   */
  [[nodiscard]] mapped_type At(const Variable var) const {
    if (!Contains(var)) throw DelpiOutOfRangeException("Variable not in the map");
    return vars_.at(var.id() - min_id_).value();
  }
  /**
   * Check if the `var` is in the map.
   * @param var variable to check
   * @return true if the variable is in the map
   * @return false if the variable is not in the map
   */
  [[nodiscard]] bool Contains(const Variable& var) const { return Contains(var.id()); }
  /**
   * Insert a `var` to the map and set it to the `value`.
   * Existing variables will be updated with the new `value`.
   * @param var variable to add
   * @param value value to set the variable to
   * @return true if the variable was added
   * @return false if the variable was already in the map
   */
  bool Insert(const Variable& var, const mapped_type& value) { return Insert(var.id(), value); }
  /**
   * Insert a range of key-value pairs to the map.
   * Existing variables will be updated with the new values.
   * @tparam I generic iterable containing variables
   * @param items range of key-value pairs to add
   */
  template <TypedIterable<value_type> I>
  void Insert(const I& items) {
    const auto Comparator = [](const std::pair<Variable, T>& a, const std::pair<Variable, T>& b) {
      return a.first.less(b.first);
    };
    // If the map is empty, initialise it with min_id_ = min(vars.id) and size m = max(vars.id) - min_id_ + 1
    if (min_id_ == std::numeric_limits<Id>::max()) {
      min_id_ = std::ranges::min_element(items, Comparator)->first.id();
      vars_.resize(std::ranges::max_element(items, Comparator)->first.id() - min_id_ + 1, false);
      for (const value_type& item : items) vars_.at(item.first.id() - min_id_) = item.second;
      return;
    }
    // If the map is not empty, resize it to the new range and insert the variables
    // First compute the new min_id and max_id comparing with the current min_id and max_id
    // Then resize the vector to the new range m = new_max - new_min + 1
    const Id new_min = std::min(std::ranges::min_element(items, Comparator)->first.id(), min_id_);
    const Id new_max = std::max(std::ranges::max_element(items, Comparator)->first.id(), vars_.size() + min_id_ - 1);
    const std::size_t max_diff = new_max - (vars_.size() + min_id_ - 1);
    const std::size_t old_size = vars_.size();
    vars_.resize(new_max - new_min + 1);
    // If the lower bound of the new range is less than the current min_id, rotate the vector to the left by max_diff
    if (new_min < min_id_) {
      std::rotate(vars_.begin(), vars_.begin() + static_cast<std::int64_t>(old_size),
                  vars_.end() - static_cast<std::int64_t>(max_diff));
      min_id_ = new_min;
    }
    for (const value_type& item : items) vars_.at(item.first.id() - min_id_) = item.second;
  }
  /**
   * Remove a `var` from the map.
   * If the variable is not in the map, nothing happens.
   * @param var variable to remove
   * @return true if the variable was removed
   * @return false if the variable was not in the map
   */
  bool Remove(const Variable& var) { return Remove(var.id()); }
  /**
   * Remove a range of `vars` from the map.
   * Only variables in the map will be removed.
   * @param vars range of variables to remove
   */
  template <TypedIterable<Variable> V>
  void Remove(const V& vars) {
    for (const Variable& var : vars) Remove(var);
  }
  /** Clear the map of all variables. */
  void Clear() {
    vars_.clear();
    min_id_ = std::numeric_limits<Id>::max();
  }

 private:
  /**
   * Check if the variable with the given `id` is in the map.
   * @param id id of the variable to check
   * @return true if the variable is in the map
   * @return false if the variable is not in the map
   */
  [[nodiscard]] bool Contains(const Id id) const {
    return id >= min_id_ && id - min_id_ < vars_.size() && vars_.at(id - min_id_).has_value();
  }
  /**
   * Insert a variable with the given `id` to the map and set it to the `value`.
   * @param id id of the variable to add
   * @param value value to set the variable to
   * @return true if the variable was added
   * @return false if the variable was already in the map
   */
  bool Insert(const Id id, const T& value) {
    if (min_id_ == std::numeric_limits<Id>::max()) {
      min_id_ = id;
      vars_.emplace_back(value);
      return true;
    }
    if (id < min_id_) {
      vars_.resize(vars_.size() + min_id_ - id, std::nullopt);
      std::ranges::rotate(vars_, vars_.begin() + static_cast<std::int64_t>(vars_.size() - (min_id_ - id)));
      min_id_ = id;
      vars_.front() = value;
      return true;
    }
    if (id >= min_id_ + vars_.size()) {
      vars_.resize(id - min_id_ + 1, std::nullopt);
      vars_.back() = value;
      return true;
    }
    vars_.at(id - min_id_) = value;
    return false;
  }
  /**
   * Remove a variable with the given `id` from the map.
   * @param id id of the variable to remove
   * @return true if the variable was removed
   * @return false if the variable was not in the map
   */
  bool Remove(const Id id) {
    if (!Contains(id)) return false;
    const bool was_present = vars_.at(id - min_id_).has_value();
    vars_.at(id - min_id_) = std::nullopt;
    return was_present;
  }

  Id min_id_;                           ///< Minimum id of the variables in the map.
  std::vector<std::optional<T>> vars_;  ///< Vector tracking the value each variable is mapped to.
};

template <class T>
std::ostream& operator<<(std::ostream& os, const VariableMap<T>& var_map);

extern template class VariableMap<int>;

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::VariableMap<int>)

#endif
