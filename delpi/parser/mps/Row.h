/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Row struct.
 */
#pragma once

#include <iosfwd>
#include <optional>
#include <vector>

#include "delpi/libs/gmp.h"
#include "delpi/parser/mps/SenseType.h"
#include "delpi/symbolic/Variable.h"

namespace delpi::mps {

/**
 * Structure representing a row in the LP solver in the form of a linear combination of variables.
 * @note Missing bounds are represented by `std::nullopt`, which means that the row is unbounded in that direction.
 * E.g. `lb` = `std::nullopt` and `ub` = `5` represents a row such that @f$ -\infty \leq \text{addends} \leq 5 @f$.
 */
struct Row {
  Row() = default;
  explicit Row(const SenseType _sense) : addends{}, lb{}, ub{}, sense{_sense} {}
  std::vector<std::pair<Variable, mpq_class>> addends;  ///< Linear combination of variables
  std::optional<mpq_class> lb;                          ///< Lower bound. If`std::nullopt`, indicated unboundness
  std::optional<mpq_class> ub;                          ///< Upper bound. If`std::nullopt`, indicated unboundness
  SenseType sense{SenseType::N};                        ///< SenseType of the row
};

std::ostream& operator<<(std::ostream& os, const Row& row);

}  // namespace delpi::mps

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::mps::Row)

#endif
