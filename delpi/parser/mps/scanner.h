/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Scanner utility class.
 * This class extends from the MpsFlexLexer class generated by Flex. It
 * provides a class named MpsScanner used to parse the file using the Flex
 * API.
 */
#pragma once

#include <iosfwd>

#ifndef __DELPI_MPS_SCANNER_H__
#define yyFlexLexer MpsFlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#ifdef DELPI_PYDELPI
#include "pydelpi/interrupt.h"
#endif

#include "delpi/parser/mps/BoundType.h"
#include "delpi/parser/mps/SenseType.h"
// All the types needed for the scanner to work must be imported before parser.yy.hh.
// Needs to be included after the above headers.
#include "delpi/parser/mps/parser.yy.hpp"

namespace delpi::mps {

/**
 * MpsScanner is a derived class to add some extra function to the scanner
 * class. Flex itself creates a class named yyFlexLexer, which is renamed using
 * macros to ExampleFlexLexer. However we change the context of the generated
 * yylex() function to be contained within the MpsScanner class. This is
 * required because the yylex() defined in ExampleFlexLexer has no parameters.
 */
class MpsScanner : public MpsFlexLexer {
 public:
  /**
   * Create a new scanner object. The streams arg_yyin and arg_yyout default
   * to cin and cout, but that assignment is only made when initializing in
   * yylex().
   */
  explicit MpsScanner(std::istream *arg_yyin = nullptr, std::ostream *arg_yyout = nullptr);

  MpsScanner(const MpsScanner &) = delete;
  MpsScanner(MpsScanner &&) = delete;
  MpsScanner &operator=(const MpsScanner &) = delete;
  MpsScanner &operator=(MpsScanner &&) = delete;

  /** Required for virtual functions */
  ~MpsScanner() override;

  /**
   * This is the main lexing function. It is generated by flex according to
   * the macro declaration YY_DECL above. The generated bison parser then
   * calls this virtual function to fetch new tokens.
   */
  virtual MpsParser::token_type lex(MpsParser::semantic_type *yylval, MpsParser::location_type *yylloc);

  /** Enable debug output (via arg_yyout) if compiled into the scanner. */
  void set_debug(bool b);
};

}  // namespace delpi::mps
