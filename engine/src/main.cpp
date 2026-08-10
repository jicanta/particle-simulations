#include <exception>
#include <iostream>

#include "commands.hpp"

namespace {

void printUsage() {
    std::cerr << "uso: cim <comando> [opciones]\n\n"
              << "  generate  --n <N> [--l 20] [--seed 1] [--periodic]\n"
              << "            [--static ../data/static.txt] [--dynamic ../data/dynamic.txt]\n\n"
              << "  neighbors [--method cim|brute] [--m <M>] [--rc 1] [--periodic] [--verify]\n"
              << "            [--static ../data/static.txt] [--dynamic ../data/dynamic.txt]\n"
              << "            [--out ../data/neighbors.txt]\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Arguments arguments(argc, argv);
        if (arguments.command() == "generate") {
            return runGenerate(arguments);
        }
        if (arguments.command() == "neighbors") {
            return runNeighbors(arguments);
        }
        printUsage();
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
