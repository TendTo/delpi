/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/mps/Scanner.h"

#include <algorithm>  // std::equal
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cctype>  // std::tolower
#include <utility>

#include "delpi/parser/mps/BoundType.h"
#include "delpi/parser/mps/SenseType.h"
#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi::mps {

inline bool ichar_equals(const char a, const char b) {
  return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

inline bool ieq(std::string_view a, std::string_view b) { return std::ranges::equal(a, b, ichar_equals); }

enum class Section {
  NONE,
  NAME,
  ROWS,
  COLUMNS,
  RHS,
  RANGES,
  BOUNDS,
  OBJSENSE,
  OBJNAME,
  ENDATA,
};

class BufferLineSource {
 public:
  explicit BufferLineSource(const std::string_view data, std::string name = "buffer")
      : cur_(data.data()), end_(data.data() + data.size()), name_{std::move(name)} {}
  BufferLineSource(const char *data, const std::size_t length, std::string name = "buffer")
      : cur_(data), end_(data + length), name_{std::move(name)} {}

  bool nextLine(std::string_view &out) {
    if (cur_ >= end_) return false;
    const char *start = cur_;
    while (cur_ < end_ && *cur_ != '\n') ++cur_;
    std::size_t len = static_cast<std::size_t>(cur_ - start);
    // Strip \r
    if (len > 0 && start[len - 1] == '\r') --len;
    if (cur_ < end_) ++cur_;  // skip '\n'
    out = {start, len};
    return true;
  }

  [[nodiscard]] const std::string &name() const { return name_; }

 protected:
  void initialize(const char *data, const std::size_t length) {
    cur_ = data;
    end_ = data + length;
  }

 private:
  const char *cur_;
  const char *end_;
  const std::string name_;
};

class MappedFileSource : public BufferLineSource {
 public:
  explicit MappedFileSource(const char *const filename)
      : BufferLineSource(nullptr, 0, filename),
        file_mapping_{filename, boost::interprocess::read_only},
        region_{file_mapping_, boost::interprocess::read_only} {
    region_.advise(boost::interprocess::mapped_region::advice_sequential);
    initialize(static_cast<const char *>(region_.get_address()), region_.get_size());
  }

 private:
  const boost::interprocess::file_mapping file_mapping_;
  boost::interprocess::mapped_region region_;
};

Section identifySection(const std::string_view word) noexcept {
  if (word == "NAME") return Section::NAME;
  if (word == "ROWS") return Section::ROWS;
  if (word == "COLUMNS") return Section::COLUMNS;
  if (word == "RHS") return Section::RHS;
  if (word == "RANGES") return Section::RANGES;
  if (word == "BOUNDS") return Section::BOUNDS;
  if (word == "OBJSENSE") return Section::OBJSENSE;
  if (word == "OBJNAME") return Section::OBJNAME;
  if (word == "ENDATA") return Section::ENDATA;
  return Section::NONE;
}

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

NewMpsScanner::NewMpsScanner(MpsDriver &driver) : driver_{driver}, line_no_{0} {}

bool NewMpsScanner::ParseFile(const std::string &filename) {
  try {
    MappedFileSource src(filename.c_str());
    ParseLines(src);
  } catch (const boost::interprocess::interprocess_exception &e) {
    DELPI_ERROR_FMT("Error reading file '{}': {}", filename, e.what());
    return false;
  }
  return true;
}

bool NewMpsScanner::ParseString(const std::string_view input) {
  BufferLineSource src{input};
  ParseLines(src);
  return true;
}

bool NewMpsScanner::ParseLines(BufferLineSource &src) {
  line_no_ = 0;
  Section current = Section::NONE;
  std::string_view line;

  while (src.nextLine(line)) {
    ++line_no_;

    // Skip empty lines
    if (line.empty()) continue;

    const std::size_t non_space_pos = line.find_first_not_of(' ');
    if (non_space_pos == std::string_view::npos) continue;  // Line is only spaces

    // Whole-line comments: '*'
    if (line[non_space_pos] == '*') {
      nextToken(line);  // Remove the first world (usually just the * char)
      HandleComment(line);
      continue;
    }

    // ── Section indicator: first character is NOT a space/tab ──
    // (Indicator records begin in column 1; data records begin in col 2+)
    if (line[0] != ' ' && line[0] != '\t') {
      std::string_view word = line;
      // Grab just the first word (section name may be followed by data, e.g.
      // "NAME  myProblem")
      std::string_view tok = nextToken(word);
      const Section s = identifySection(tok);
      switch (s) {
        case Section::NONE:
          DELPI_WARN_FMT("Ignoring unknowns section or record {}:{}, col 1: '{}'", src.name(), line_no_, tok);
          break;
        case Section::ENDATA:
          driver_.End();
          return true;
        case Section::NAME:
          HandleName(word);
          [[fallthrough]];
        default:
          current = s;
      }
      continue;
    }

    constexpr bool success = true;
    switch (current) {
      case Section::NONE:
        DELPI_WARN("Data record before any section indicator");
        break;
      case Section::NAME:
        // Extra NAME data records are unusual; ignore them.
        break;
      case Section::ROWS:
        HandleRows(line);
        break;
      case Section::COLUMNS:
        HandleColumns(line);
        break;
      case Section::RHS:
        HandleRhs(line);
        break;
      case Section::RANGES:
        HandleRanges(line);
        break;
      case Section::BOUNDS:
        HandleBounds(line);
        break;
      case Section::OBJSENSE:
        HandleObjSense(line);
        break;
      case Section::OBJNAME:
        HandleObjName(line);
        break;
      default:
        DELPI_UNREACHABLE();
    }
    if (!success) return false;
  }
  return false;
}

inline void NewMpsScanner::HandleComment(std::string_view line) {
  // Comments are ignored, but we could parse them for @set-option and @set-info directives in the future.
  const std::string_view word = nextToken(line);
  const std::string key = std::string{nextToken(line)};
  const std::string value = std::string{nextToken(line)};
  if (ieq(word, "@set-option")) {
    driver_.SetOption(key, value);
  } else if (ieq(word, "@set-info")) {
    driver_.SetInfo(key, value);
  }
}

inline void NewMpsScanner::HandleName(std::string_view line) {
  // rest is everything after "NAME" on the indicator line.
  const std::size_t start = line.find_first_not_of(" \t");
  if (start == std::string_view::npos) return;
  const std::size_t last_pos = line.find_last_not_of(" \t");
  driver_.m_problem_name() = line.substr(start, last_pos + 1);
}

inline void NewMpsScanner::HandleObjSense(std::string_view line) {
  const std::string_view tok = nextToken(line);
  if (tok.empty()) return;

  // Case-insensitive compare
  if (ieq(tok, "MIN") || ieq(tok, "MINIMIZE") || ieq(tok, "MINIMISE"))
    driver_.ObjectiveSense(true);
  else if (ieq(tok, "MAX") || ieq(tok, "MAXIMIZE") || ieq(tok, "MAXIMISE"))
    driver_.ObjectiveSense(false);
  else
    DELPI_WARN_FMT("Line {}: Unknown OBJSENSE value '{}'. Expected MIN or MAX. Ignoring", line_no_, tok);
}

inline void NewMpsScanner::HandleObjName(std::string_view line) {
  std::string_view tok = nextToken(line);
  if (!tok.empty()) driver_.ObjectiveName(tok);
}

inline void NewMpsScanner::HandleRows(std::string_view line) {
  // Format: <sense> <name>
  const std::string_view senseStr = nextToken(line);
  if (senseStr.empty()) return;
  std::string_view name = nextToken(line);
  if (name.empty()) DELPI_ERROR_FMT("Line {}: ROWS record missing row name: '{}'", line_no_, line);

  // Dollar-sign comment in field 3+ (not really applicable here, but be safe)
  const SenseType sense = ParseSense(senseStr.size() == 1 ? senseStr[0] : '\0');
  driver_.AddRow(sense, name);
}

inline void NewMpsScanner::HandleColumns(std::string_view line) {
  // Check for integer marker: MARKER  'INTORG' / 'INTEND'
  // Format used by many solvers:
  //   <colName>  'MARKER'  'INTORG'
  //   <colName>  'MARKER'  'INTEND'
  std::string_view f2 = nextToken(line);  // col name or bound type
  if (f2.empty()) return;
  std::string_view f3 = nextToken(line);
  if (f3.empty()) return;

  // Dollar-sign comment terminates the record
  if (f3[0] == '$') return;

  // Check for MARKER
  if (f3 == "'MARKER'") {
    std::string_view f4 = nextToken(line);
    driver_.SetMarker(f2, f4);
    return;
  }

  // Normal coefficient record
  // f2 = col name, f3 = row name, next = value, [optional: row2, value2]
  std::string_view rowName1 = f3;
  std::string_view valStr1 = nextToken(line);
  if (valStr1.empty()) DELPI_ERROR_FMT("Line {}: COLUMNS record missing coefficient value", line_no_);
  if (valStr1[0] == '$') return;  // comment
  driver_.AddColumn(f2, rowName1, gmp::StringToMpq(valStr1));

  // Optional second pair on the same line
  std::string_view rowName2 = nextToken(line);
  if (rowName2.empty() || rowName2[0] == '$') return;
  std::string_view valStr2 = nextToken(line);
  if (valStr2.empty() || valStr2[0] == '$') {
    DELPI_WARN_FMT("Line {}: COLUMNS record has field 5 (row name) but no field 6 (value)", line_no_);
    return;
  }
  driver_.AddColumn(f2, rowName2, gmp::StringToMpq(valStr2));
}

inline void NewMpsScanner::HandleRhs(std::string_view line) {
  // Format: [<rhsName>] <rowName> <value>  [<rowName> <value>]

  const std::string_view f1 = nextToken(line);
  const std::string_view f2 = nextToken(line);

  if (f1.empty() || f2.empty()) return;

  const std::string_view f3 = nextToken(line);
  if (f3.empty() || f3.starts_with('$')) {
    // No RHS name, f1 is row name, f2 is value
    driver_.AddRhs("", f1, gmp::StringToMpq(f2));
    return;
  }

  const std::string_view f4 = nextToken(line);
  if (f4.empty() || f4.starts_with('$')) {
    // No second pair, f1 is RHS name, f2 is row name, f3 is value
    driver_.AddRhs(f1, f2, gmp::StringToMpq(f3));
    return;
  }

  const std::string_view f5 = nextToken(line);
  if (f5.empty() || f5.starts_with('$')) {
    // No rhs, we have two pairs [name, value]
    driver_.AddRhs("", f1, gmp::StringToMpq(f2));
    driver_.AddRhs("", f3, gmp::StringToMpq(f4));
    return;
  }

  driver_.AddRhs(f1, f2, gmp::StringToMpq(f3));
  driver_.AddRhs(f1, f4, gmp::StringToMpq(f5));
}

inline void NewMpsScanner::HandleRanges(std::string_view line) {
  // Format: <rangeName> <rowName> <value>  [<rowName> <value>]
  const std::string_view rangeName = nextToken(line);
  if (rangeName.empty()) return;

  const std::string_view rowName1 = nextToken(line);
  if (rowName1.empty() || rowName1[0] == '$') return;
  const std::string_view valStr1 = nextToken(line);
  if (valStr1.empty() || valStr1[0] == '$') return;
  driver_.AddRange(rangeName, rowName1, gmp::StringToMpq(valStr1));

  const std::string_view rowName2 = nextToken(line);
  if (rowName2.empty() || rowName2[0] == '$') return;
  const std::string_view valStr2 = nextToken(line);
  if (valStr2.empty() || valStr2[0] == '$') return;
  driver_.AddRange(rangeName, rowName2, gmp::StringToMpq(valStr2));
}

inline void NewMpsScanner::HandleBounds(std::string_view line) {
  // Format: <type> [<bndName>] <colName> [<value>]
  std::string_view typeStr = nextToken(line);
  if (typeStr.empty()) return;

  std::string_view bndName = nextToken(line);
  if (bndName.empty()) {
    DELPI_ERROR_FMT("Line {}: BOUNDS record incomplete", line_no_);
    return;
  }

  std::string_view colName = nextToken(line);

  std::string_view valStr = nextToken(line);

  switch (const BoundType bt = ParseBoundType(typeStr)) {
    case BoundType::FR:
    case BoundType::MI:
    case BoundType::PL:
    case BoundType::BV:
      // Catch the (rare) case where colName is not present, shifting everything by one
      if (colName.empty() || colName[0] == '$') {
        colName = bndName;
        bndName = "";
      }
      driver_.AddBound(bt, bndName, colName);
      break;
    default:
      // Catch the (rare) case where bndName is not present, shifting everything by one
      if (valStr.empty() || valStr[0] == '$') {
        valStr = colName;
        colName = bndName;
        bndName = "";
      }
      driver_.AddBound(bt, bndName, colName, gmp::StringToMpq(valStr));
      break;
  }
}

}  // namespace delpi::mps
