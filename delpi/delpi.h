/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * delpi library.
 * delpi is an exact LP solver with support for delta relaxation.
 * Under the hood, it uses a modified version of either [QSopt_ex](https://www.math.uwaterloo.ca/~bico/qsopt/ex/)
 * or [SoPlex](https://soplex.zib.de/).
 */
#pragma once

#include "delpi/parser/parser.h"
#include "delpi/solver/solver.h"
#include "delpi/symbolic/symbolic.h"
#include "delpi/util/ArgParser.h"
#include "delpi/util/Config.h"

/**
 * @namespace delpi
 * Global namespace for the delpi library.
 */
namespace delpi {}  // namespace delpi
