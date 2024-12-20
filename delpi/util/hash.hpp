/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Hashing functions.
 */
#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <utility>
#include <vector>

/** @namespace delpi::hash Namespace containing all the hash functions and utilities */
namespace delpi::hash {

/** Combines a given hash value `seed` and a hash of parameter `v`. */
template <class T>
size_t hash_combine(size_t seed, const T &v);

template <class T, class... Rest>
size_t hash_combine(size_t seed, const T &v, Rest... rest) {
  return hash_combine(hash_combine(seed, v), rest...);
}

/** Computes the combined hash value of the elements of an iterator range. */
template <typename It>
size_t hash_range(It first, It last) {
  size_t seed{};
  for (; first != last; ++first) {
    seed = hash_combine(seed, *first);
  }
  return seed;
}

/** Computes the hash value of `v` using std::hash. */
template <class T>
struct hash_value {
  size_t operator()(const T &v) const { return std::hash<T>{}(v); }
};

/** Computes the hash value of a pair `p`. */
template <class T1, class T2>
struct hash_value<std::pair<T1, T2>> {
  size_t operator()(const std::pair<T1, T2> &p) const { return hash_combine(0, p.first, p.second); }
};

/** Computes the hash value of a vector `vec`. */
template <class T>
struct hash_value<std::vector<T>> {
  size_t operator()(const std::vector<T> &vec) const { return hash_range(vec.begin(), vec.end()); }
};

/** Computes the hash value of a set `s`. */
template <class T>
struct hash_value<std::set<T>> {
  size_t operator()(const std::set<T> &s) const { return hash_range(s.begin(), s.end()); }
};

/** Computes the hash value of a `map`. */
template <class T1, class T2>
struct hash_value<std::map<T1, T2>> {
  size_t operator()(const std::map<T1, T2> &map) const { return hash_range(map.begin(), map.end()); }
};

/** Combines two hash values into one. The following code is public domain
 *  according to http://www.burtleburtle.net/bob/hash/doobs.html. */
template <class T>
inline size_t hash_combine(size_t seed, const T &v) {
  seed ^= hash_value<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  return seed;
}

}  // namespace delpi::hash
