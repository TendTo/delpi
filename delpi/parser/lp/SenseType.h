/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * SenseType of a constraint row.
 * The sense indicates the type or relation a contraint row has with
 * respect to its right-hand side.
 * The supported values are 'L', 'E', 'G', or 'N'.
 * They represent, respectively, less than, equal to, greater than, or
 * no constraint, usually applied to the objective function.
 */
#pragma once

#include <iosfwd>
#include <string>

namespace delpi::lp {

/**
 * SenseType of a constraint row.
 */
enum class SenseType {
  L,  ///< Less or equal to
  E,  ///< Equal to
  G,  ///< Greater or equal to
  N   ///< No sense, used for the objective function
};

std::ostream &operator<<(std::ostream &os, const SenseType &sense);

}  // namespace delpi::lp

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::lp::SenseType)

#endif
