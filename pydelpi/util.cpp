/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#ifndef DELPI_PYTHON
#error DELPI_PYTHON is not defined. Ensure you are building with the option '--config=python'
#endif

#include <pybind11/stl.h>

#include "delpi/util/ArgParser.h"
#include "delpi/util/Config.h"
#include "pydelpi.h"

namespace py = pybind11;
using namespace delpi;

void init_util(py::module_ &m) {
  m.attr("LOG_NONE") = -1;
  m.attr("LOG_CRITICAL") = 0;
  m.attr("LOG_ERROR") = 1;
  m.attr("LOG_WARN") = 2;
  m.attr("LOG_INFO") = 3;
  m.attr("LOG_DEBUG") = 4;
  m.attr("LOG_TRACE") = 5;
  m.def("set_verbosity", [](const int value) { DELPI_LOG_INIT_VERBOSITY(value); });

  py::enum_<Config::LPSolver>(m, "LPSolver")
      .value("QSOPTEX", Config::LPSolver::QSOPTEX)
      .value("SOPLEX", Config::LPSolver::SOPLEX);

  py::enum_<Config::Format>(m, "Format").value("AUTO", Config::Format::AUTO).value("MPS", Config::Format::MPS);

  py::enum_<Config::LPMode>(m, "LPMode")
      .value("AUTO", Config::LPMode::AUTO)
      .value("PURE_PRECISION_BOOSTING", Config::LPMode::PURE_PRECISION_BOOSTING)
      .value("PURE_ITERATIVE_REFINEMENT", Config::LPMode::PURE_ITERATIVE_REFINEMENT)
      .value("HYBRID", Config::LPMode::HYBRID);

  py::class_<Config>(m, "Config")
      .def(py::init<>())
      .def_static(
          "from_args",
          [](const std::vector<std::string> &args) {
            const char **argv = new const char *[args.size()];
            for (size_t i = 0; i < args.size(); ++i) {
              argv[i] = const_cast<char *>(args[i].c_str());
            }
            ArgParser argparser{};
            argparser.Parse(static_cast<int>(args.size()), argv);
            delete[] argv;
            return argparser.ToConfig();
          },
          py::arg("args"))
      .def_property("csv", &Config::csv, [](Config &self, bool value) { self.m_csv() = value; })
      .def_property("continuous_output", &Config::continuous_output,
                    [](Config &self, const bool value) { self.m_continuous_output() = value; })
      .def_property("debug_parsing", &Config::debug_parsing,
                    [](Config &self, const bool value) { self.m_debug_parsing() = value; })
      .def_property("debug_scanning", &Config::debug_scanning,
                    [](Config &self, const bool value) { self.m_debug_scanning() = value; })
      .def_property("filename", &Config::filename,
                    [](Config &self, const std::string &value) { self.m_filename() = value; })
      .def_property("format", &Config::format,
                    [](Config &self, const Config::Format &value) { self.m_format() = value; })
      .def_property("lp_mode", &Config::lp_mode,
                    [](Config &self, const Config::LPMode value) { self.m_lp_mode() = value; })
      .def_property("lp_solver", &Config::lp_solver,
                    [](Config &self, const Config::LPSolver &value) { self.m_lp_solver() = value; })
      .def_property("number_of_jobs", &Config::number_of_jobs,
                    [](Config &self, const int value) { self.m_number_of_jobs() = value; })
      .def_property("optimize", &Config::optimize, [](Config &self, const bool value) { self.m_optimize() = value; })
      .def_property("precision", &Config::precision, [](Config &self, double value) { self.m_precision() = value; })
      .def_property("produce_model", &Config::produce_models,
                    [](Config &self, const bool value) { self.m_produce_models() = value; })
      .def_property("random_seed", &Config::random_seed, [](Config &self, int value) { self.m_random_seed() = value; })
      .def_property("read_from_stdin", &Config::read_from_stdin,
                    [](Config &self, const bool value) { self.m_read_from_stdin() = value; })
      .def_property("silent", &Config::silent, [](Config &self, bool value) { self.m_silent() = value; })
      .def_property("verbose_delpi", &Config::verbose_delpi,
                    [](Config &self, const int value) { self.m_verbose_delpi() = value; })
      .def_property("verbose_simplex", &Config::verbose_simplex,
                    [](Config &self, const int value) { self.m_verbose_simplex() = value; })
      .def_property("verify", &Config::verify, [](Config &self, const bool value) { self.m_verify() = value; })
      .def_property("with_timings", &Config::with_timings,
                    [](Config &self, const bool value) { self.m_with_timings() = value; })
      .def("__str__", [](const Config &self) { return (std::stringstream() << self).str(); });
}