/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * OptType of the LP problem.
 */
#pragma once

#include <iosfwd>

namespace delpi::lp {

/**
 * OptType of the LP problem,
 */
enum class OptType {
  MIN,  ///< Minimize
  MAX,  ///< Maximize
};

std::ostream &operator<<(std::ostream &os, const OptType &sense);

}  // namespace delpi::lp

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::lp::OptType)

#endif
