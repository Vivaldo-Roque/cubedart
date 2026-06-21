// cubejs/solver.hpp
//
// C++ port of src/solve.coffee: Herbert Kociemba's two-phase algorithm
// for solving the 3x3x3 Rubik's Cube, as ported to JS/CoffeeScript by
// ldez/akheron in cube.js (https://github.com/ldez/cubejs).
//
// Usage:
//   cubejs::Cube::initSolver();          // once, a few hundred ms
//   auto solution = cube.solve();        // std::optional<std::string>

#pragma once

#include "cubejs/cube.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cubejs {

class Solver {
public:
    static void init();
    static bool isInitialized() { return initialized_; }

    static std::optional<std::string> solveUpright(const Cube& cube, int maxDepth = 22);
    static std::optional<std::string> solve(const Cube& cube, int maxDepth = 22);
    static std::string scramble();

private:
    static bool initialized_;

    // Move tables: table[coordValue][moveCode (0..17)] -> newCoordValue
    static std::vector<std::vector<int>> twistTable_;
    static std::vector<std::vector<int>> flipTable_;
    static std::vector<std::vector<int>> FRtoBRTable_;
    static std::vector<std::vector<int>> URFtoDLFTable_;
    static std::vector<std::vector<int>> URtoDFTable_;
    static std::vector<std::vector<int>> URtoULTable_;
    static std::vector<std::vector<int>> UBtoDFTable_;
    static std::vector<std::vector<int>> mergeURtoDFTable_;  // 336 x 336

    // Pruning tables: 4 bits per entry, packed 8-to-a-uint32_t.
    static std::vector<uint32_t> sliceTwistPruning_;
    static std::vector<uint32_t> sliceFlipPruning_;
    static std::vector<uint32_t> sliceURFtoDLFParityPruning_;
    static std::vector<uint32_t> sliceURtoDFParityPruning_;

    struct SearchState;

    static void initState(SearchState& s, const Cube& cube);
    static int minDist1(const SearchState& s);
    static int minDist2(const SearchState& s);
    static SearchState next1(SearchState& s, int move);
    static SearchState next2(SearchState& s, int move);
    static void initPhase2(SearchState& s, bool top);
    static std::string buildSolution(const SearchState& s);

    static void phase1(SearchState& state, int depth, int maxDepthTotal,
                        std::optional<std::string>& solution);
    static void phase2search(SearchState& state, int maxDepthTotal,
                              std::optional<std::string>& solution);
    static void phase2(SearchState& state, int depth, std::optional<std::string>& solution);
};

}  // namespace cubejs
