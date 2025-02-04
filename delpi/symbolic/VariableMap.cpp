/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/symbolic/VariableMap.h"

#include <ostream>

namespace delpi {

template class VariableMap<int>;

template <class T>
std::ostream& operator<<(std::ostream& os, const VariableMap<T>& var_map) {
  os << "{";
  bool is_first = true;
  for (std::size_t i = 0; i < var_map.capacity(); ++i) {
    const Variable var{i};
    if (!var_map.Contains(var)) continue;
    os << (is_first ? "" : ", ") << var << " -> " << var_map.At(var);
    is_first = false;
  }
  return os << "}";
}

template std::ostream& operator<<(std::ostream& os, const VariableMap<int>& var_map);

}  // namespace delpi
