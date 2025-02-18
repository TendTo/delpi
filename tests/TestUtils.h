/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 dlinear
 * @licence BSD 3-Clause License
 * Solvers utilities for testing.
 */
#pragma once

#include <gtest/gtest.h>

#include "delpi/util/Config.h"

const auto enabled_test_solvers = ::testing::Values(
#ifdef DELPI_ENABLED_QSOPTEX
    delpi::Config::LpSolver::QSOPTEX
#endif
#ifdef DELPI_ENABLED_SOPLEX
#ifdef DELPI_ENABLED_QSOPTEX
    ,
#endif
#endif
#ifdef DELPI_ENABLED_SOPLEX
    delpi::Config::LpSolver::SOPLEX
#endif
);
