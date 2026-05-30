/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <benchmark/benchmark.h>

#include <utility>

#include "delpi/libs/gmp.h"

using delpi::gmp::StringToMpq;
using std::make_pair;

namespace {

inline mpq_class OldStringToMpq(std::string_view str) {
  // Remove leading + and - sign
  if (str.empty()) return {0};
  const bool is_negative = str[0] == '-';
  if (is_negative || str[0] == '+') str.remove_prefix(1);
  if (str == "inf") return {1e100};
  if (str == "-inf") return {-1e100};

  // case 1: string is given in integer format
  const size_t symbol_pos = str.find_first_of("/.Ee");
  if (symbol_pos == std::string::npos) {
    const size_t start_pos = str.find_first_not_of('0', str[0] == '+' ? 1 : 0);
    if (start_pos == std::string_view::npos) return {0};
    return is_negative ? -mpq_class{str.data() + start_pos} : mpq_class{str.data() + start_pos};
  }

  // case 2: string is given in nom/denom format
  if (str[symbol_pos] == '/') {
    mpq_class res{str.data()};
    res.canonicalize();
    return is_negative ? -res : res;
  }

  const size_t e_pos = str[symbol_pos] == 'e' || str[symbol_pos] == 'E' ? symbol_pos : str.find_first_of("Ee");
  mpz_class mult{is_negative ? -1 : 1};
  bool is_exp_positive = true;

  // case 3a: string is given as base-10 decimal number (e)
  if (e_pos != std::string::npos) {
    const long exponent = std::stol(str.data() + e_pos + 1);  // NOLINT(runtime/int)
    is_exp_positive = exponent >= 0;
    mult = 10;
    mpz_pow_ui(mult.get_mpz_t(), mult.get_mpz_t(), std::abs(exponent));
    if (is_negative) mult = -mult;
    // Remove the exponent
    str = str.substr(0, e_pos);

    if (str.empty()) return is_exp_positive ? mpq_class{mult} : is_negative ? mpq_class{-1, -mult} : mpq_class{1, mult};
  }

  const size_t len = str.length();

  // case 3b: string does not contain a . , only an exponent E
  if (str[symbol_pos] == 'e' || str[symbol_pos] == 'E') {
    int plus_pos = str[0] == '+' ? 1 : 0;
    char *const str_number = new char[len - plus_pos + 1];
    memcpy(str_number, str.data() + plus_pos, len - plus_pos);
    str_number[len - plus_pos] = '\0';
    const mpq_class res{str_number, 10};
    delete[] str_number;
    return is_exp_positive ? mpq_class{res * mult} : mpq_class{res / mult};
  }

  const size_t &dot_pos = symbol_pos;

  // case 3c: string contains a .
  size_t start_pos = str.find_first_not_of('0');
  size_t digits;

  // case 4a: string starts with a . or the numbers before the . are all 0
  if (start_pos == dot_pos) {
    start_pos = str.find_first_not_of('0', dot_pos + 1);
    // case 5: string contains only a .
    if (start_pos == std::string_view::npos) {
      return {0};
    } else {
      digits = len - start_pos;
    }
  } else {  // case 4b: string does not start with a . and the numbers before the . are not all 0
    digits = len - start_pos - 1;
  }

  const size_t n_decimals = len - dot_pos - 1;
  char *const str_number = new char[digits + n_decimals + 3];

  if (digits > n_decimals) {
    memcpy(str_number, str.data() + start_pos, digits - n_decimals);
    memcpy(str_number + dot_pos, str.data() + dot_pos + 1, n_decimals);
  } else {
    memcpy(str_number, str.data() + start_pos, n_decimals);
  }

  str_number[digits] = '/';
  str_number[digits + 1] = '1';
  memset(str_number + digits + 2, '0', n_decimals);
  str_number[digits + 2 + n_decimals] = '\0';

  mpq_class res{str_number, 10};
  delete[] str_number;
  res.canonicalize();
  return is_exp_positive ? mpq_class{res * mult} : res / mult;
}

}  // namespace

constexpr int array_size = 33;
const std::array<std::string, array_size> test_cases{
    "0",
    ".",
    "0.",
    "0.0",
    ".0",
    "15",
    "1.5",
    "0000015.",
    ".15",
    ".15",
    ".0015",
    "15.0",
    "15.00",
    "0150",
    "1.5E2",
    "1.5E-2",
    ".e+2",
    ".5E+2",
    "000000.5E+2",
    "000000.005E-2",
    "E+2",
    "7E+2",
    "E-2",
    "15/6",
    "0/1010",
    "",
    "inf",
    "27478957737906802747895773790680/3332829324078086700849327918825751640085460886132816108357146041",
    "-27478957737906802747895773790680/3332829324078086700849327918825751640085460886132816108357146041",
    "-27478957737906802747895773790680.3332829324078086700849327918825751640085460886132816108357146041",
    "27478957737906802747895773790680.3332829324078086700849327918825751640085460886132816108357146041",
    "-27478957737906802747895773790680.3332829324078086700849327918825751640085460886132816108357146041e10",
    "27478957737906802747895773790680.3332829324078086700849327918825751640085460886132816108357146041e-10"};

void OldStringToMpqMethod(benchmark::State &state) {
  const std::string &s = test_cases.at(state.range(0));
  for (auto _ : state) {
    mpq_class res = OldStringToMpq(s);
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void ReducedAllocStringToMpq(benchmark::State &state) {
  const std::string &s = test_cases.at(state.range(0));
  for (auto _ : state) {
    mpq_class res = StringToMpq(s);
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

#define DELPI_RUNS DenseRange(0, array_size - 1, 1)->Complexity(benchmark::oN)

BENCHMARK(OldStringToMpqMethod)->DELPI_RUNS;
BENCHMARK(ReducedAllocStringToMpq)->DELPI_RUNS;
