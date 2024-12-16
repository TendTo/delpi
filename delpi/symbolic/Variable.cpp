/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/symbolic/Variable.h"

#include <atomic>
#include <ostream>

#include "delpi/util/error.h"

using std::atomic;
using std::make_shared;
using std::ostream;
using std::ostringstream;
using std::string;

namespace delpi {

std::vector<std::string> Variable::names_{{"dummy"}};

Variable::Id Variable::GetNextId() {
  static std::atomic<Id> next_id{0};
  const std::size_t counter = next_id.fetch_add(1);
  return counter;
}

Variable::Variable(std::string name) : id_{GetNextId()} {
  DELPI_ASSERT(id_ < std::numeric_limits<Id>::max(), "The ID of the variable has reached the maximum value.");
  names_.push_back(std::move(name));
  DELPI_ASSERT(names_.size() == id_ + 2u, "The size of the names vector is not equal to the ID + dummy string.");
}

ostream &operator<<(ostream &os, const Variable &var) { return os << var.name(); }

}  // namespace delpi
