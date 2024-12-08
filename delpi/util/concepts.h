/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Collection of concepts used in the delpi library.
 *
 * Concepts have been introduced in the c++20 standard and are used in the delpi library
 * in order to make the code more readable and to provide better error messages in templated code.
 */
#pragma once

#include <concepts>

namespace delpi {

/**
 * Check if the type `T` is a self-reference counter type, i.e. it has both `AddRef` and `Release` methods.
 *
 * The `AddRef` method should increment the internal reference counter.
 * The `Release` method should decrement it, and if it reaches zero, the object should be deleted using its destructor.
 * @code
 * template <Iterable T>
 * void foo(T t); // T can be any iterable type
 * @endcode
 * @tparam T type to check
 */
template <class T>
concept SelfReferenceCounter = requires(T t) {
  { t.AddRef() };
  { t.Release() };
};  // NOLINT(readability/braces) per C++ standard concept definition

/**
 * Check if the type `T` is an iterable type.
 * @code
 * template <Iterable T>
 * void foo(T t); // T can be any iterable type
 * @endcode
 * @tparam T type to check
 */
template <class T>
concept Iterable = requires(T t) {
  { t.begin() } -> std::convertible_to<typename T::iterator>;
  { t.end() } -> std::convertible_to<typename T::iterator>;
};  // NOLINT(readability/braces) per C++ standard concept definition

/**
 * Check if the type `T` is an iterable type with a `size()` method.
 * @code
 * template <SizedIterable T>
 * void foo(T t); // T can be any iterable type with a size() method
 * @endcode
 * @tparam T type to check
 */
template <class T>
concept SizedIterable = requires(T t) {
  { t.begin() } -> std::convertible_to<typename T::iterator>;
  { t.end() } -> std::convertible_to<typename T::iterator>;
  { t.size() } -> std::convertible_to<std::size_t>;
};  // NOLINT(readability/braces) per C++ standard concept definition

/**
 * Check if the type `T` is an iterable type with elements of type `U`.
 * @code
 * template <TypedIterable<int> T>
 * void foo(T t); // T can be any iterable type with elements of type int
 * @endcode
 * @tparam T type to check
 * @tparam U type of the elements
 */
template <typename T, typename U>
concept TypedIterable = requires(T t, U u) {
  { t.begin() } -> std::convertible_to<typename T::iterator>;
  { t.end() } -> std::convertible_to<typename T::iterator>;
} && std::same_as<typename T::value_type, U>;

/**
 * Check if the type `T` is an iterable type with elements of type `U` and a `size()` method.
 * @code
 * template <TypedSizedIterable<int> T>
 * void foo(T t); // T can be any iterable type with elements of type int and a size() method
 * @endcode
 * @tparam T type to check
 * @tparam U type of the elements
 */
template <typename T, typename U>
concept SizedTypedIterable = requires(T t, U u) {
  { t.begin() } -> std::convertible_to<typename T::iterator>;
  { t.end() } -> std::convertible_to<typename T::iterator>;
  { t.size() } -> std::convertible_to<std::size_t>;
} && std::same_as<typename T::value_type, U>;

/**
 * Check if the type `T` is any of the types `U`.
 * @code
 * template <IsAnyOf<int, float, bool> T>
 * void foo(T t); // T can be either int, float or bool
 * @endcode
 * @tparam T type to check
 * @tparam U any number of types to check against
 */
template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

/**
 * Check if the type `T` is not any of the types `U`.
 * @code
 * template <IsNotAnyOf<int, float, bool> T>
 * void foo(T t); // T can be any type except int, float or bool
 * @endcode
 * @tparam T type to check
 * @tparam U any number of types to check against
 */
template <typename T, typename... U>
concept IsNotAnyOf = !IsAnyOf<T, U...>;

/**
 * Check if the type `T` supports the arithmetic operations `+`, `-`, `*`, `/`.
 * @code
 * template <Arithmetic T>
 * void foo(T a, T b); // a and b can be added, subtracted, multiplied and divided with the corresponding operator
 * @endcode
 * @tparam T type to check
 */
template <class T>
concept Arithmetic = requires(T a, T b) {
  { a + b } -> std::convertible_to<T>;
  { a - b } -> std::convertible_to<T>;
  { a* b } -> std::convertible_to<T>;
  { a / b } -> std::convertible_to<T>;
};  // NOLINT(readability/braces) per C++ standard concept definition

/**
 * Check if the type `T` supports the arithmetic operations `+`, `-`, `*`, `/`
 * and the comparison operators `<, `>`, `<=`, `>=`.
 * @code
 * template <Numeric T>
 * void foo(T a); // a can be added, subtracted, multiplied, divided and ordered with the corresponding operator
 * @endcode
 * @tparam T type to check
 */
template <class T>
concept Numeric = std::totally_ordered<T> && Arithmetic<T>;

}  // namespace delpi
