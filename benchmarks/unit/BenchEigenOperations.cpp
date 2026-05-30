/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <benchmark/benchmark.h>

#include <execution>
#include <span>

#include "delpi/libs/eigen.h"

using delpi::Index;
using delpi::MatrixD;

constexpr unsigned int num_frequencies = 10;
constexpr double sigma_l = 1.4;

void UseComparisonAllTrue(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = (mat.array() > -2).all();
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseMinTrue(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = mat.minCoeff() > -2;
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseOMPNestedLoopTrue(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = true;
#pragma omp parallel for collapse(2) shared(res)
    for (Index i = 0; i < mat.rows(); i++) {
      for (Index j = 0; j < mat.cols(); j++) {
        if (mat(i, j) <= -2) res = false;
      }
    }
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseOMPSingleLoopTrue(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = true;
#pragma omp parallel for shared(res)
    for (Index i = 0; i < mat.size(); i++) {
      if (mat.data()[i] <= -2) res = false;
    }
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

template <class Policy>
  requires std::is_execution_policy<Policy>::value
void UseTransformReduceLoopTrue(benchmark::State& state) {
  constexpr Policy policy{};
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  std::span<const double> data_span{mat.data(), static_cast<std::size_t>(mat.size())};
  for (auto _ : state) {
    bool res = std::transform_reduce(policy, data_span.begin(), data_span.end(), true, std::logical_and<>{},
                                     [](const double val) { return val > -2; });
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseComparisonAllFalse(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = (mat.array() > 0).all();
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseMinFalse(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = mat.minCoeff() > 0;
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

template <class Policy>
  requires std::is_execution_policy<Policy>::value
void UseTransformReduceLoopFalse(benchmark::State& state) {
  constexpr Policy policy{};
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  std::span<const double> data_span{mat.data(), static_cast<std::size_t>(mat.size())};
  for (auto _ : state) {
    bool res = std::transform_reduce(policy, data_span.begin(), data_span.end(), true, std::logical_and<>{},
                                     [](const double val) { return val > 0; });
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseOMPNestedLoopFalse(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = true;
#pragma omp parallel for collapse(2) shared(res)
    for (Index i = 0; i < mat.rows(); i++) {
      for (Index j = 0; j < mat.cols(); j++) {
        if (mat(i, j) <= 0) res = false;
      }
    }
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

void UseOMPSingleLoopFalse(benchmark::State& state) {
  const MatrixD mat{MatrixD::Random(state.range(0), state.range(0))};
  for (auto _ : state) {
    bool res = true;
#pragma omp parallel for shared(res)
    for (Index i = 0; i < mat.size(); i++) {
      if (mat.data()[i] <= 0) res = false;
    }
    benchmark::DoNotOptimize(res);
  }
  state.SetComplexityN(state.range(0));
}

#define DELPI_RUNS Range(1 << 4, 1 << 10)->Complexity(benchmark::oNSquared)

BENCHMARK(UseComparisonAllTrue)->DELPI_RUNS;
BENCHMARK(UseMinTrue)->DELPI_RUNS;
BENCHMARK(UseOMPNestedLoopTrue)->DELPI_RUNS;
BENCHMARK(UseOMPSingleLoopTrue)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopTrue<std::execution::sequenced_policy>)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopTrue<std::execution::unsequenced_policy>)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopTrue<std::execution::parallel_policy>)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopTrue<std::execution::parallel_unsequenced_policy>)->DELPI_RUNS;
BENCHMARK(UseComparisonAllFalse)->DELPI_RUNS;
BENCHMARK(UseMinFalse)->DELPI_RUNS;
BENCHMARK(UseOMPNestedLoopFalse)->DELPI_RUNS;
BENCHMARK(UseOMPSingleLoopFalse)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopFalse<std::execution::sequenced_policy>)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopFalse<std::execution::unsequenced_policy>)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopFalse<std::execution::parallel_policy>)->DELPI_RUNS;
BENCHMARK(UseTransformReduceLoopFalse<std::execution::parallel_unsequenced_policy>)->DELPI_RUNS;