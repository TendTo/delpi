/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Soplex wrapper.
 *
 * This header includes the Soplex library and provides a various helpers.
 * Other files in the library should depend on this header instead of the Soplex library directly.
 * Instead of including <soplex.h>, include "delpi/libs/soplex.h".
 * In the build files, instead of depending on "@soplex", depend on "//delpi/libs:soplex".
 */
#pragma once

#ifndef DELPI_ENABLED_SOPLEX
#error SoPlex is not enabled. Please enable it by adding "--//tools:enable_soplex" to the bazel command.
#endif

#pragma GCC system_header

#include <soplex/soplex.h>  // IWYU pragma: export

extern template class soplex::SoPlexBase<soplex::Real>;

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(soplex::VectorRational)
OSTREAM_FORMATTER(soplex::Rational)
OSTREAM_FORMATTER(soplex::SPxSolver::Status)
OSTREAM_FORMATTER(soplex::DSVectorRational)
OSTREAM_FORMATTER(soplex::SVectorRational)

#endif
