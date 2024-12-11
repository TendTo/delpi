/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include <fstream>

#include "delpi/util/ArgParser.h"
#include "delpi/util/Config.h"

using delpi::ArgParser;
using delpi::Config;

class TestArgParser : public ::testing::Test {
 protected:
  ArgParser parser_{};
  const std::string filename_mps_{"TempFile.mps"};
  const std::string bad_filename_{"TempFile.err"};
  const std::string non_existing_filename_{"NotExistingTempFile.mps"};
  void SetUp() override {
    std::ofstream m{filename_mps_};
    std::ofstream bf{bad_filename_};
  }
  void TearDown() override {
    std::remove(filename_mps_.c_str());
    std::remove(bad_filename_.c_str());
  }
};

TEST_F(TestArgParser, Contructor) { EXPECT_NO_THROW(ArgParser parser{}); }

TEST_F(TestArgParser, DefaultValues) {
  const char *argv[] = {"delpi", filename_mps_.c_str()};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_DOUBLE_EQ(parser_.get<double>("precision"), 9.999999999999996e-4);
  EXPECT_FALSE(parser_.get<bool>("produce-models"));
  EXPECT_EQ(parser_.get<uint>("random-seed"), 0u);
  //  EXPECT_EQ(parser_.get<uint>("jobs"), 1u);
  EXPECT_FALSE(parser_.get<bool>("continuous-output"));
  EXPECT_FALSE(parser_.get<bool>("debug-parsing"));
  EXPECT_FALSE(parser_.get<bool>("debug-scanning"));
  EXPECT_EQ(parser_.get<Config::Format>("format"), Config::Format::AUTO);
  EXPECT_FALSE(parser_.get<bool>("in"));
  EXPECT_EQ(parser_.get<Config::LpSolver>("lp-solver"), Config::LpSolver::SOPLEX);
  EXPECT_FALSE(parser_.get<bool>("timings"));
  EXPECT_EQ(parser_.get<int>("verbose-simplex"), 0);
  EXPECT_FALSE(parser_.get<bool>("silent"));
}

TEST_F(TestArgParser, ParseVerbosityIncreaseOne) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--verbose"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.ToConfig().verbose_delpi(), Config::default_verbose_delpi + 1);
}

TEST_F(TestArgParser, ParseVerbosityIncreaseMultiple) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "-VV"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.ToConfig().verbose_delpi(), Config::default_verbose_delpi + 2);
}

TEST_F(TestArgParser, ParseVerbosityIncreaseMax) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "-VVVVVVVVV"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.ToConfig().verbose_delpi(), 5);
}

TEST_F(TestArgParser, ParseVerbosityDecreaseOne) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--quiet"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.ToConfig().verbose_delpi(), Config::default_verbose_delpi - 1);
}

TEST_F(TestArgParser, ParseVerbosityDecreaseMultiple) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "-qq"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.ToConfig().verbose_delpi(), Config::default_verbose_delpi - 2);
}

TEST_F(TestArgParser, ParseVerbosityDecreaseMin) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "-qqqqqqqqqqqqqq"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.ToConfig().verbose_delpi(), 0);
}

TEST_F(TestArgParser, ParsePrecision) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--precision", "2.1"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_DOUBLE_EQ(parser_.get<double>("precision"), 2.1);
}

TEST_F(TestArgParser, ParseInvalidPrecision) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--precision", "-1"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --precision");
}

// TEST_F(TestArgParser, ParseJobs) {
//   const char *argv[] = {"delpi", filename_mps_.c_str(), "--jobs", "2"};
//   parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
//   EXPECT_EQ(parser_.get<uint>("jobs"), 2u);
// }
//
// TEST_F(TestArgParser, ParseInvalidJobs) {
//   const char *argv[] = {"delpi", filename_mps_.c_str(), "--jobs", "-1"};
//   EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Failed to parse '-1' as decimal integer");
// }

TEST_F(TestArgParser, ParseContinuousOutput) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--continuous-output"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_TRUE(parser_.get<bool>("continuous-output"));
}

TEST_F(TestArgParser, ParseProduceModels) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--produce-models"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_TRUE(parser_.get<bool>("produce-models"));
}

TEST_F(TestArgParser, ParseRandomSeed) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--random-seed", "10"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.get<uint>("random-seed"), 10ul);
}

TEST_F(TestArgParser, InvalidInAndFile) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--in"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --in");
}

TEST_F(TestArgParser, FileNotFound) {
  const char *argv[] = {"delpi", non_existing_filename_.c_str()};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "cannot find file");
}

TEST_F(TestArgParser, FileNotProvided) {
  const char *argv[] = {"delpi"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for file");
}

TEST_F(TestArgParser, AutoFormatSmt2) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--format", "auto"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.get<std::string>("file"), filename_mps_);
  EXPECT_EQ(parser_.get<Config::Format>("format"), Config::Format::AUTO);
}

TEST_F(TestArgParser, AutoFormatMps) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--format", "auto"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.get<std::string>("file"), filename_mps_);
  EXPECT_EQ(parser_.get<Config::Format>("format"), Config::Format::AUTO);
}

TEST_F(TestArgParser, WrongAutoFormat) {
  const char *argv[] = {"delpi", bad_filename_.c_str(), "--format", "auto"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for file");
}

TEST_F(TestArgParser, MpsFormat) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--format", "mps"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(parser_.get<std::string>("file"), filename_mps_);
  EXPECT_EQ(parser_.get<Config::Format>("format"), Config::Format::MPS);
}

TEST_F(TestArgParser, WrongFormat) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--format", "invalid"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --format");
}

TEST_F(TestArgParser, Exhaustive) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--precision", "0"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  auto config = parser_.ToConfig();
  EXPECT_DOUBLE_EQ(config.precision(), 0.0);
}

TEST_F(TestArgParser, Silent) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--silent"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_TRUE(parser_.get<bool>("silent"));
}

TEST_F(TestArgParser, WrongSilentWithVerbose) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--silent", "--verbose"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --verbose");
}

TEST_F(TestArgParser, WrongSilentWithQuiet) {
  const char *argv[] = {"delpi", filename_mps_.c_str(), "--silent", "--quiet"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --quiet");
}

TEST_F(TestArgParser, In) {
  const char *argv[] = {"delpi", "--in", "--format", "mps"};
  parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv);
  auto config = parser_.ToConfig();
  EXPECT_TRUE(config.read_from_stdin());
}

TEST_F(TestArgParser, WrongInMissingFormat) {
  const char *argv[] = {"delpi", "--in"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --in");
}

TEST_F(TestArgParser, WrongInAutoFormat) {
  const char *argv[] = {"delpi", "--in", "--format", "auto"};
  EXPECT_DEATH(parser_.Parse(sizeof(argv) / sizeof(argv[0]), argv), "Invalid argument for --in");
}
