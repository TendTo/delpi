/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Qsopt_ex wrapper.
 * This header includes the Qsopt_ex library and provides a various helpers.
 * Other files in the library should depend on this header instead of the Qsopt_ex library directly.
 * Instead of including <qsopt_ex/Qsopt_ex.h>, include "delpi/libs/libqsopt_ex.h".
 * In the build files, instead of depending on "@qsopt_ex", depend on "//delpi/libs:qsopt_ex".
 */
#pragma once

#ifndef DELPI_ENABLED_QSOPTEX
#error QSopt_ex is not enabled. Please enable it by adding "--//tools:enable_qsoptex" to the bazel command.
#endif

#include <gmpxx.h>

extern "C" {
#include <qsopt_ex/QSopt_ex.h>  // IWYU pragma: export
}

// These #defines from <qsopt_ex/QSopt_ex.h> cause problems for us
// because they mess with SoPlex's enums.
#undef OPTIMAL
#undef DUAL_INFEASIBLE

#include <iosfwd>
#include <string>

/** @namespace delpi::qsopt_ex Namespace containing all the utility functions to interact with the QSopt_ex solver */
namespace delpi::qsopt_ex {

/**
 * Convert a string to a mpq_class.
 * @param str string representation of a rational number
 * @return pointer to a dynamically allocated mpq_class. Must be freed with delete.
 * @warning The caller is responsible for freeing the returned pointer.
 */
mpq_class *StringToMpqPtr(const std::string &str);
/**
 * Convert a string to a mpq_class.
 * @param str string representation of a rational number
 * @return mpq_class object
 */
mpq_class StringToMpq(const std::string &str);
/**
 * Convert a C-string to a mpq_class.
 * @param str C-string representation of a rational number
 * @return pointer to a dynamically allocated mpq_class. Must be freed with delete.
 * @warning The caller is responsible for freeing the returned pointer.
 */
mpq_class *CStringToMpqPtr(const char str[]);
/**
 * Convert a string to a mpq_class.
 * @param str C-string representation of a rational number
 * @return mpq_class object
 */
mpq_class CStringToMpq(const char str[]);

/**
 * A wrapper around an array of mpq_t elements.
 * It is used to pass around arrays of mpq_t, ensuring they are cleaned up after use.
 * The array is allocated by AllocateMpqArray() and freed by FreeMpqArray().
 */
class MpqArray {
 public:
  /**
   * Construct a new MpqArray object, allocating the array with `n_elements` elements.
   * @param n_elements The number of elements in the array.
   */
  explicit MpqArray(size_t n_elements = 0);
  MpqArray(const MpqArray &) = delete;
  MpqArray(MpqArray &&) = delete;
  MpqArray &operator=(const MpqArray &) = delete;
  MpqArray &operator=(MpqArray &&) = delete;
  /** Destroy the MpqArray object, freeing the array */
  ~MpqArray();

  /**
   * Obtain a constant pointer to the internal @ref array_.
   * @return internal mpq_t array as a constant pointer
   */
  operator const mpq_t * const *() const { return &array_; }
  /**
   * Obtain a constant pointer to the internal @ref array_.
   * @return internal mpq_t array as a constant pointer
   */
  operator mpq_t **() { return &(array_); }

  /**
   * Obtain a constant pointer to the internal @ref array_.
   * @return internal mpq_t array as a constant pointer
   */
  operator const mpq_t *() const { return array_; }

  /**
   * Obtain a pointer to the internal array.
   * @return internal mpq_t array
   */
  operator mpq_t *() { return array_; }

  mpq_t &operator[](const int idx) { return array_[idx]; }

  const mpq_t &operator[](const int idx) const { return array_[idx]; }

  /** @getter{size, array} */
  [[nodiscard]] size_t size() const { return array_ ? reinterpret_cast<size_t *>(array_)[-1] : 0; }

  /**
   * Resize the array to have `nElements` elements.
   *
   * All the previous elements are lost.
   * @param nElements new  number of elements in the array
   */
  void Resize(size_t nElements);

 private:
  mpq_t *array_;  ///< array of mpq_t. It is allocated by AllocateMpqArray() and freed by FreeMpqArray().

  /**
   * Allocate the array with `n_elements` elements.
   *
   * The array has a peculiar structure, where the element at index -1 is the size of the array.
   * All the other `n_elements` elements are mpq_t.
   * @param n_elements The number of elements in the array.
   */
  void AllocateMpqArray(size_t n_elements);

  /** Free the array of mpq_t */
  void FreeMpqArray();
};

std::ostream &operator<<(std::ostream &os, const MpqArray &array);

void QSXStart();
void QSXFinish();

}  // namespace delpi::qsopt_ex

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::qsopt_ex::MpqArray);

#endif
