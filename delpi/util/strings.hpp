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
 * Whitespace includes both ' ' and '\\t' characters.
 * @warning This function WILL modify the input string view by removing the token and all leading whitespace from it.
 * @param line string to check
 * @return view of a word in the string without any whitespaces. Can be emtpy if such a word does not exist.
 */
inline std::string_view nextToken(std::string_view &line) noexcept {
  // Skip leading whitespace
  std::size_t i = 0;
  while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
  if (i == line.size()) {
    line = {};
    return {};
  }
  line.remove_prefix(i);

  // Find end of token
  std::size_t j = 0;
  while (j < line.size() && line[j] != ' ' && line[j] != '\t') ++j;

  const std::string_view tok = line.substr(0, j);
  line.remove_prefix(j);
  return tok;
}

}  // namespace delpi
