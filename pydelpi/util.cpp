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

  py::enum_<Config::LpSolver>(m, "LpSolverName")
      .value("QSOPTEX", Config::LpSolver::QSOPTEX)
      .value("SOPLEX", Config::LpSolver::SOPLEX);

  py::enum_<Config::Format>(m, "Format").value("AUTO", Config::Format::AUTO).value("MPS", Config::Format::MPS);

  py::enum_<Config::LpMode>(m, "LpMode")
      .value("AUTO", Config::LpMode::AUTO)
      .value("PURE_PRECISION_BOOSTING", Config::LpMode::PURE_PRECISION_BOOSTING)
      .value("PURE_ITERATIVE_REFINEMENT", Config::LpMode::PURE_ITERATIVE_REFINEMENT)
      .value("HYBRID", Config::LpMode::HYBRID);

  py::class_<Config>(m, "Config")
      .def(py::init<>())
      .def(py::init<>([](const std::string &filename, const Config::LpSolver &lp_solver, const double precision,
                         const bool csv, const bool continuous_output, const bool debug_parsing,
                         const bool debug_scanning, const Config::Format &format, const Config::LpMode &lp_mode,
                         const int number_of_jobs, const bool optimize, const bool produce_models,
                         const int random_seed, const bool read_from_stdin, const bool silent, const int verbose_delpi,
                         const int verbose_simplex, const bool verify, const bool with_timings) {
             std::unique_ptr<Config> config{std::make_unique<Config>()};
             config->m_csv() = csv;
             config->m_continuous_output() = continuous_output;
             config->m_debug_parsing() = debug_parsing;
             config->m_debug_scanning() = debug_scanning;
             config->m_filename() = filename;
             config->m_format() = format;
             config->m_lp_mode() = lp_mode;
             config->m_lp_solver() = lp_solver;
             config->m_number_of_jobs() = number_of_jobs;
             config->m_optimize() = optimize;
             config->m_precision() = precision;
             config->m_produce_models() = produce_models;
             config->m_random_seed() = random_seed;
             config->m_read_from_stdin() = read_from_stdin;
             config->m_silent() = silent;
             config->m_verbose_delpi() = verbose_delpi;
             config->m_verbose_simplex() = verbose_simplex;
             config->m_verify() = verify;
             config->m_with_timings() = with_timings;
             return config;
           }),
           py::arg("filename") = "", py::arg_v("lp_solver", Config::default_lp_solver, "LpSolverName.SOPLEX"),
           py::arg("precision") = Config::default_precision, py::arg("csv") = Config::default_csv,
           py::arg("continuous_output") = Config::default_continuous_output,
           py::arg("debug_parsing") = Config::default_debug_parsing,
           py::arg("debug_scanning") = Config::default_debug_scanning,
           py::arg_v("format", Config::default_format, "Format.AUTO"),
           py::arg_v("lp_mode", Config::default_lp_mode, "LpMode.AUTO"),
           py::arg("number_of_jobs") = Config::default_number_of_jobs, py::arg("optimize") = Config::default_optimize,
           py::arg("produce_models") = Config::default_produce_models,
           py::arg("random_seed") = Config::default_random_seed,
           py::arg("read_from_stdin") = Config::default_read_from_stdin, py::arg("silent") = Config::default_silent,
           py::arg("verbose_delpi") = Config::default_verbose_delpi,
           py::arg("verbose_simplex") = Config::default_verbose_simplex, py::arg("verify") = Config::default_verify,
           py::arg("with_timings") = Config::default_with_timings)
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
                    [](Config &self, const Config::LpMode value) { self.m_lp_mode() = value; })
      .def_property("lp_solver", &Config::lp_solver,
                    [](Config &self, const Config::LpSolver &value) { self.m_lp_solver() = value; })
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
      .def("__str__", STR_LAMBDA(Config))
      .def("__repr__", REPR_LAMBDA(Config));
}