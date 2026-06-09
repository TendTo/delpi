/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * Scanner class.
 */
#include "delpi/parser/lp/Scanner.h"

#include "delpi/util/error.h"
#include "delpi/util/logging.h"
#include "delpi/util/strings.hpp"

namespace delpi::lp {

enum class Section {
  NONE,
  PROBLEM,
  OBJECTIVE_MAX,
  OBJECTIVE_MIN,
  CONSTRAINTS,
  BOUNDS,
  BINARY,
  GENERAL,
  END,
};

enum class Infinity {
  NONE,
  POSITIVE,
  NEGATIVE,
};

inline Section identifySection(const std::string_view word) noexcept {
  if (ieq(word, "problem")) return Section::PROBLEM;
  if (ieq(word, "min") || ieq(word, "minimum") || ieq(word, "minimize")) return Section::OBJECTIVE_MIN;
  if (ieq(word, "max") || ieq(word, "maximum") || ieq(word, "maximize")) return Section::OBJECTIVE_MAX;
  if (ieq(word, "subject to") || ieq(word, "such that") || ieq(word, "subj to") || ieq(word, "st") || ieq(word, "s.t."))
    return Section::CONSTRAINTS;
  if (ieq(word, "bounds") || ieq(word, "bound")) return Section::BOUNDS;
  if (ieq(word, "binary") || ieq(word, "bin") || ieq(word, "binaries")) return Section::BINARY;
  if (ieq(word, "general") || ieq(word, "gen")) return Section::GENERAL;
  if (ieq(word, "end")) return Section::END;
  return Section::NONE;
}

inline std::optional<SenseType> TryParseSenseType(const std::string_view token) {
  if (token == "<=" || token == "<") return SenseType::L;
  if (token == ">=" || token == ">") return SenseType::G;
  if (token == "=" || token == "==") return SenseType::E;
  return std::nullopt;
}

inline Infinity ParseInfinity(std::string_view token) {
  if (token.empty()) return Infinity::NONE;
  bool is_negative = token[0] == '-';
  if (token[0] == '-' || token[0] == '+') token.remove_prefix(1);
  if (ieq(token, "inf") || ieq(token, "infinity") || ieq(token, "infinit")) {
    return is_negative ? Infinity::NEGATIVE : Infinity::POSITIVE;
  }
  return Infinity::NONE;
}

bool LpScanner::ParseLines(BufferLineSource& src) {
  Section current = Section::NONE;
  std::string_view line;

  while (src.nextLine(line)) {
    ++line_no_;

    // Remove any comments
    if (const std::size_t pos = line.find('\\'); pos != std::string_view::npos) {
      HandleComment(line.substr(pos + 1));
      line = line.substr(0, pos);
    }

    // Skip empty lines
    if (line.empty()) continue;

    // Section indicator: check if the first token is a section keyword
    if (const Section new_section = identifySection(trim(line)); new_section != Section::NONE) {
      // Finalize the objective section
      if (current == Section::OBJECTIVE_MAX || current == Section::OBJECTIVE_MIN) {
        driver_.SetObjectiveType(current == Section::OBJECTIVE_MAX ? OptType::MAX : OptType::MIN);
        driver_.AddObjective(expression_, identifier_);
        ResetExpression();
      }

      // Go to the next section
      ResetExpression();
      current = new_section;
      continue;
    }

    line = trim(line);

    bool res = true;
    switch (current) {
      case Section::NONE:
        DELPI_WARN("Data record before any section indicator");
        break;
      case Section::PROBLEM:
        driver_.SetProblemName(line);
        break;
      case Section::OBJECTIVE_MAX:
      case Section::OBJECTIVE_MIN:
        res = HandleObjective(line);
        break;
      case Section::CONSTRAINTS:
        res = HandleConstrains(line);
        break;
      case Section::BOUNDS:
        res = HandleBounds(line);
        break;
      case Section::BINARY:
        HandleBinary(line);
        break;
      case Section::GENERAL:
        HandleGeneral(line);
        break;
      case Section::END:
        driver_.End();
        return true;
      default:
        DELPI_UNREACHABLE();
    }

    if (!res) return false;
  }

  if (current == Section::OBJECTIVE_MAX || current == Section::OBJECTIVE_MIN) {
    driver_.SetObjectiveType(current == Section::OBJECTIVE_MAX ? OptType::MAX : OptType::MIN);
    driver_.AddObjective(expression_, identifier_);
    ResetExpression();
  }
  driver_.End();
  return true;
}

bool LpScanner::HandleComment(std::string_view line) {
  // Comments are ignored, but we could parse them for @set-option and @set-info directives in the future.
  const std::string_view word = nextToken(line);
  const std::string key = std::string{nextToken(line)};
  const std::string value = std::string{nextToken(line)};
  if (ieq(word, "@set-option")) {
    driver_.SetOption(key, value);
  } else if (ieq(word, "@set-info")) {
    driver_.SetInfo(key, value);
  }
  return true;
}

bool LpScanner::HandleObjective(std::string_view line) {
  if (const std::size_t colon_pos = line.find_first_of(':'); colon_pos != std::string_view::npos) {
    identifier_ = trim(line.substr(0, colon_pos));
    line = trim(line.substr(colon_pos + 1));
  }

  while (true) {
    const std::string_view token = nextToken(line);

    if (token.empty()) break;

    if (!ParseTerm(token, term_)) {
      DELPI_ERROR_FMT("{}:{}, Invalid term '{}'", filename_, line_no_, token);
      return false;
    }

    if (term_.var.has_value()) {
      mpq_class coeff = term_.coeff.value_or(mpq_class{1});
      if (term_.sign.has_value()) coeff *= term_.sign.value();
      expression_[term_.var.value()] += coeff;

      term_ = Term{};
    }
  }

  return true;
}
bool LpScanner::HandleConstrains(std::string_view line) {
  // New row identifier
  if (const std::size_t colon_pos = line.find_first_of(':'); colon_pos != std::string_view::npos) {
    identifier_ = trim(line.substr(0, colon_pos));
    line = trim(line.substr(colon_pos + 1));
  }

  while (true) {
    const std::string_view token = nextToken(line);

    if (token.empty()) return true;

    if (std::optional<SenseType> sense = TryParseSenseType(token); sense.has_value()) {
      const std::string_view next_token = nextToken(line);
      if (next_token.empty()) {
        DELPI_ERROR_FMT("{}:{}, Missing RHS for constraint '{}'", filename_, line_no_, identifier_);
        return false;
      }
      const mpq_class rhs = gmp::StringToMpq(next_token);
      driver_.AddConstraint(expression_, sense.value(), rhs, identifier_);
      ResetExpression();
      return true;
    }

    if (!ParseTerm(token, term_)) {
      DELPI_ERROR_FMT("{}:{}, Invalid term '{}'", filename_, line_no_, token);
      return false;
    }

    if (term_.var.has_value()) {
      mpq_class coeff = term_.coeff.value_or(mpq_class{1});
      if (term_.sign.has_value()) coeff *= term_.sign.value();
      expression_[term_.var.value()] += coeff;

      term_ = Term{};
    }
  }
}
bool LpScanner::HandleBounds(std::string_view line) {
  const std::string_view term_1 = nextToken(line);
  if (term_1.empty()) return true;

  const std::string_view op_1 = nextToken(line);

  if (op_1.empty()) {
    DELPI_ERROR_FMT("{}:{} Invalid bound '{}'", filename_, line_no_, line);
    ;
    return false;
  }

  if (ieq(op_1, "free")) {
    driver_.AddFreeBound(term_1);
    return true;
  }

  if (op_1 == "=") {
    mpq_class value{gmp::StringToMpq(nextToken(line))};
    driver_.AddLowerBound(term_1, value);
    driver_.AddUpperBound(term_1, value);
    return true;
  }

  const std::string_view term_2 = nextToken(line);

  if (term_2.empty()) {
    DELPI_ERROR_FMT("{}:{}, Invalid bound '{}'", filename_, line_no_, line);
    return false;
  }

  const std::string_view op_2 = nextToken(line);
  const std::string_view term_3 = nextToken(line);

  if (!(op_1 == "<" || op_1 == "<=")) {
    DELPI_ERROR_FMT("{}:{}, Invalid bound operator '{}', expected '<=", filename_, line_no_, op_1);
    return false;
  }

  if (!op_2.empty() && !(op_2 == "<" || op_2 == "<=")) {
    DELPI_ERROR_FMT("{}:{}, Invalid bound operator '{}', expected '<=", filename_, line_no_, op_2);
    return false;
  }

  if (!term_1.empty() && !term_3.empty()) {
    driver_.AddLowerBound(
        term_2, ParseInfinity(term_1) == Infinity::NEGATIVE ? std::nullopt : std::optional{gmp::StringToMpq(term_1)});
    driver_.AddUpperBound(
        term_2, ParseInfinity(term_3) == Infinity::POSITIVE ? std::nullopt : std::optional{gmp::StringToMpq(term_3)});
  } else if (term_1.find_first_of("01234567890.eE-+") == 0 || ParseInfinity(term_1) != Infinity::NONE) {
    // Variables cannot start with [0-9], '.', '/', 'e/E', or must not match infinity keywords
    driver_.AddLowerBound(
        term_2, ParseInfinity(term_1) == Infinity::NEGATIVE ? std::nullopt : std::optional{gmp::StringToMpq(term_1)});
  } else {
    driver_.AddUpperBound(
        term_1, ParseInfinity(term_2) == Infinity::POSITIVE ? std::nullopt : std::optional{gmp::StringToMpq(term_2)});
  }

  return true;
}

void LpScanner::HandleBinary(std::string_view line) {
  for (std::string_view var = nextToken(line); !var.empty(); var = nextToken(line)) {
    driver_.MarkBinary(var);
  }
}

void LpScanner::HandleGeneral(std::string_view line) {
  for (std::string_view var = nextToken(line); !var.empty(); var = nextToken(line)) {
    driver_.MarkGeneral(var);
  }
}

bool LpScanner::ParseTerm(std::string_view token, Term& prev) {
  // Ignore emtpy strings and multiplication operators
  if (token.empty() || token[0] == '*') return true;

  // Handle sign
  if (token[0] == '+' || token[0] == '-') {
    if (prev.sign.has_value()) return false;
    prev.sign = (token[0] == '+') ? 1.0 : -1.0;
    token.remove_prefix(1);
  }

  if (token.empty()) return true;

  // Handle coefficient
  if (std::isdigit(token[0]) || token[0] == '.') {
    if (prev.coeff.has_value()) return false;
    // We are assuming a variable attached to the coefficient cannot start with a number [0-9], a '.', '/', 'e' or 'E'
    const std::size_t end_pos = token.find_first_not_of("0123456789.eE/+-");
    std::string_view coeff_str = token.substr(0, end_pos);
    prev.coeff = gmp::StringToMpq(coeff_str);
    if (end_pos == std::string_view::npos) return true;
    token.remove_prefix(end_pos);
  }

  if (token.empty()) return true;

  if (prev.var.has_value()) return false;
  prev.var = token;
  return true;
}

void LpScanner::ResetExpression() {
  identifier_ = {};
  expression_.clear();
  term_ = Term{};
}

}  // namespace delpi::lp