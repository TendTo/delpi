#pragma once

#include <algorithm>

namespace delpi {

/**
 * Return the minimum of the given values.
 * @tparam T type of the values
 * @param a first value
 * @param args remaining values
 * @return minimum of the values
 */
template <class T, class... Args>
T Min(const T& a, const Args&... args) {
  if constexpr (sizeof...(args) == 0) {
    return a;
  } else {
    return std::min(a, Min(args...));
  }
}

/**
 * Return the maximum of the given values.
 * @tparam T type of the values
 * @param a first value
 * @param args remaining values
 * @return maximum of the values
 */
template <class T, class... Args>
T Max(const T& a, const Args&... args) {
  if constexpr (sizeof...(args) == 0) {
    return a;
  } else {
    return std::max(a, Max(args...));
  }
}

}  // namespace delpi
