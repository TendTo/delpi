/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Utilities for filesystem operations.
 *
 * Simple utilities that make operations on the filesystem easier.
 */
#pragma once

#include <string>
#include <vector>

namespace delpi {

/**
 * Get the extension of the file.
 *
 * Extracts the extension from `name`, meaning the part of the file name
 * after the last dot.
 * @note It returns an empty string if there is no extension in `name`.
 * @param name name of the file
 * @return extension of the file
 */
std::string GetExtension(const std::string &name);

/**
 * Split a C-string by whitespace.
 *
 * Each word is returned as a separate string in a vector.
 * @note This function is not Unicode-aware.
 * @note The words are trimmed.
 * @param in input string to split
 * @return vector os strings
 */
std::vector<std::string> SplitStringByWhitespace(const char *in);

/**
 * Get the files in a directory.
 *
 * @param path path to the directory
 * @param extension filter the selection to files that have a matching extension
 * @return vector of strings, each string being the path to each file in the directory.
 */
std::vector<std::string> GetFiles(const std::string &path, const std::string& extension = "");

}  // namespace delpi
