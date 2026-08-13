#include <exception>
#include <iostream>

#include "commands.hpp"

namespace {

void printUsage() {
  std::cerr
      << "uso: cim <comando> [opciones]\n\n"
      << "  generate  --n <N> [--l 20] [--seed 1] [--periodic]\n"
      << "            [--static ../data/static.txt] [--dynamic "
         "../data/dynamic.txt]\n\n"
      << "  neighbors [--method cim|brute] [--m <M>] [--rc 1] [--periodic]\n"
      << "            [--particle <id>] [--verify]\n"
      << "            [--static ../data/static.txt] [--dynamic "
         "../data/dynamic.txt]\n"
      << "            [--out ../data/neighbors.txt]\n\n"
      << "  benchmark --sweep m --n <N> [--l 20] [--rc 1] [--repeats 100]\n"
      << "            [--out ../data/benchmark_m.csv]\n"
      << "  benchmark --sweep n --n-max <N> [--n-min 10] [--steps 10] "
         "[--density <d>]\n"
      << "            [--m <M>] [--l 20] [--rc 1] [--repeats 100]\n"
      << "            [--out ../data/benchmark_n.csv]\n\n"
      << "  comunes a benchmark: [--method cim|brute] [--periodic] [--seed 1]\n"
      << "                       [--min-radius 0.23] [--max-radius 0.26]\n";
}

}

int main(int argc, char** argv) {
  try {
    const Arguments arguments(argc, argv);
    if (arguments.command() == "generate") {
      return runGenerate(arguments);
    }
    if (arguments.command() == "neighbors") {
      return runNeighbors(arguments);
    }
    if (arguments.command() == "benchmark") {
      return runBenchmark(arguments);
    }
    printUsage();
    return 1;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << "\n";
    return 1;
  }
}
