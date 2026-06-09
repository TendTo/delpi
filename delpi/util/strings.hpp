/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @copyright 2017 Toyota Research Institute (dreal4)
 * @licence BSD 3-Clause License
 * String utilities.
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>

namespace delpi {

/**
 * Check whether two characters are equal, ignoring case.
 * @param a first character
 * @param b second character
 * @return true if the two characters are the same
 * @return false if the two characters are different
 */
inline bool ichar_equals(const char a, const char b) noexcept {
  return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

/**
 * Check whether two strings are the same, ignoring the case of the characters.
 * @param a first string
 * @param b second string
 * @return true if the two strings are the same
 * @return false if the two strings are different
 */
inline bool ieq(std::string_view a, std::string_view b) { return std::ranges::equal(a, b, ichar_equals); }

/**
 * Given a string, find the next word token contained within ignoring all whitespaces.
 * Whitespace characters are defined according to the `std::isspace` standard function.
 * @warning This function WILL modify the input string view by removing the token and all leading whitespace from it.
 * @param line string to check
 * @return view of a word in the string without any whitespaces. Can be emtpy if such a word does not exist.
 */
inline std::string_view nextToken(std::string_view &line) noexcept {
  // Skip leading whitespace
  std::size_t i = 0;
  while (i < line.size() && std::isspace(line[i])) ++i;
  if (i == line.size()) {
    line = {};
    return {};
  }
  line.remove_prefix(i);

  // Find end of token
  std::size_t j = 0;
  while (j < line.size() && !std::isspace(line[j])) ++j;

  const std::string_view tok = line.substr(0, j);
  line.remove_prefix(j);
  return tok;
}

/**
 * Given a string, find the next word token contained within ignoring all whitespaces.
 * Whitespace characters are defined according to the `std::isspace` standard function.
 * @param line string to check
 * @return view of a word in the string without any whitespaces. Can be emtpy if such a word does not exist.
 */
inline std::string_view peakNextToken(const std::string_view line) noexcept {
  // Skip leading whitespace
  std::size_t i = 0;
  while (i < line.size() && std::isspace(line[i])) ++i;
  if (i >= line.size()) return {};

  // Find end of token
  std::size_t j = i + 1;
  while (j < line.size() && !std::isspace(line[j])) ++j;
  return line.substr(i, j - i);
}

/**
 * Remove leading and trailing whitespace from a string.
 * Whitespace characters are defined according to the `std::isspace` standard function.
 * @param str string to trim
 * @return view of the input string without leading or trailing whitespace.
 * Can be empty if the input string is only whitespace.
 */
inline std::string_view trim(const std::string_view str) noexcept {
  std::size_t start = 0, end = str.size() - 1;
  while (start < str.size() && std::isspace(str[start])) ++start;
  while (end > start && std::isspace(str[end])) --end;
  return str.substr(start, end - start + 1);
}

}  // namespace delpi
