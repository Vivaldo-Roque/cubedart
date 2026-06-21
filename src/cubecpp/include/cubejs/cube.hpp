// cubejs/cube.hpp
//
// C++ port of cube.js (https://github.com/ldez/cubejs) by ldez / akheron,
// originally written in CoffeeScript. This file ports `src/cube.coffee`:
// the cube model itself (state, moves, facelet conversion, randomization).
//
// The solving algorithm (Kociemba's two-phase algorithm, ported from
// `src/solve.coffee`) lives in solver.hpp/.cpp; Cube::solve() etc. are
// thin wrappers declared here and implemented in cube.cpp.

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace cubejs {

// --- Cubie indices -------------------------------------------------------

// Centers
enum CenterIndex { U = 0, R = 1, F = 2, D = 3, L = 4, B = 5 };

// Corners
enum CornerIndex {
    URF = 0, UFL = 1, ULB = 2, UBR = 3,
    DFR = 4, DLF = 5, DBL = 6, DRB = 7
};

// Edges
enum EdgeIndex {
    UR = 0, UF = 1, UL = 2, UB = 3, DR = 4, DF = 5,
    DL = 6, DB = 7, FR = 8, FL = 9, BL = 10, BR = 11
};

// A face transformation: how a single quarter turn (or composite, for
// E/M/S/x/y/z/u/r/f/d/l/b) permutes/reorients centers, corners and edges.
// Cube derives from this, since cornerMultiply/edgeMultiply/centerMultiply
// only need this shape, mirroring the duck-typing used in the original
// JS/CoffeeScript (Cube.moves entries are plain objects, not Cube instances).
struct CubeState {
    std::array<int, 6> center{};
    std::array<int, 8> cp{};
    std::array<int, 8> co{};
    std::array<int, 12> ep{};
    std::array<int, 12> eo{};
};

using MoveDef = CubeState;

class Cube : public CubeState {
public:
    // Constructs an identity (solved) cube.
    Cube();

    void identity();
    void init(const Cube& other);

    // Facelet string <-> cube state. The string has 54 characters, 9 per
    // face, in the order U R F D L B:
    //
    //              +------------+
    //              | U1  U2  U3 |
    //              | U4  U5  U6 |
    //              | U7  U8  U9 |
    // +------------+------------+------------+------------+
    // | L1  L2  L3 | F1  F2  F3 | R1  R2  R3 | B1  B2  B3 |
    // | L4  L5  L6 | F4  F5  F6 | R4  R5  R6 | B4  B5  B6 |
    // | L7  L8  L9 | F7  F8  F9 | R7  R8  R9 | B7  B8  B9 |
    // +------------+------------+------------+------------+
    //              | D1  D2  D3 |
    //              | D4  D5  D6 |
    //              | D7  D8  D9 |
    //              +------------+
    static Cube fromString(const std::string& str);
    std::string asString() const;

    Cube clone() const { return *this; }

    bool isSolved() const;

    // Randomizes the cube in place to a uniformly random *solvable* state.
    void randomize();
    static Cube random();

    Cube& centerMultiply(const CubeState& other);
    Cube& cornerMultiply(const CubeState& other);
    Cube& edgeMultiply(const CubeState& other);
    Cube& multiply(const CubeState& other);

    // Parses a string like "U F R2 B' D2 L'" into a list of move codes
    // (face * 3 + power, power: 0 = single, 1 = double, 2 = inverse).
    // Throws std::invalid_argument on a malformed move.
    static std::vector<int> parseAlg(const std::string& alg);
    static std::string algToString(const std::vector<int>& moves);

    // Cancels adjacent same-face redundancy (e.g. "R R'" -> "", "R R" ->
    // "R2", "R2 R'" -> "R"). Does not reorder moves on different faces,
    // so it only catches cancellations that are already adjacent in the
    // sequence (which is exactly what's needed after concatenating
    // algorithm fragments, e.g. in obfuscate()).
    static std::vector<int> simplify(const std::vector<int>& moveCodes);
    static std::string simplify(const std::string& alg);

    Cube& move(const std::string& alg);
    Cube& move(int moveCode);
    Cube& move(const std::vector<int>& moveCodes);

    // Returns a rotation algorithm (x/y/z/x'/y'/z'/x2/y2/z2/"") that
    // brings this cube's centers into the standard U-top F-front
    // orientation.
    std::string upright() const;

    static int inverse(int moveCode);
    static std::vector<int> inverse(const std::vector<int>& moves);
    static std::string inverse(const std::string& alg);

    // --- Coordinates used by the solver -------------------------------
    int twist() const;
    void setTwist(int t);
    int flip() const;
    void setFlip(int f);
    int cornerParity() const;
    int edgeParity() const;

    int URFtoDLF() const;
    void setURFtoDLF(int idx);
    int URtoUL() const;
    void setURtoUL(int idx);
    int UBtoDF() const;
    void setUBtoDF(int idx);
    int URtoDF() const;
    void setURtoDF(int idx);
    int FRtoBR() const;
    void setFRtoBR(int idx);

    // The 18 face transformations: U, R, F, D, L, B, E, M, S, x, y, z, u,
    // r, f, d, l, b (in that order). Used internally by move()/multiply().
    static const std::vector<MoveDef>& moves();

    // --- High level solving (see solver.hpp/.cpp) ----------------------

    // Precomputes the move and pruning tables needed by solve() /
    // solveUpright(). Takes well under a second on a modern machine, but
    // must be called once before solving (idempotent).
    static void initSolver();

    // Solves the cube assuming it's already in the upright (U-top,
    // F-front) orientation. Returns std::nullopt if no solution exists
    // within maxDepth moves (shouldn't happen for maxDepth >= 20, since
    // every solvable cube can be solved in at most 20 moves).
    std::optional<std::string> solveUpright(int maxDepth = 22) const;

    // Solves the cube regardless of orientation.
    std::optional<std::string> solve(int maxDepth = 22) const;

    // Generates a random scramble algorithm (the inverse of solving a
    // random cube), suitable for handing to a human to scramble a
    // physical cube.
    static std::string scramble();

private:
    int permIndex(bool corners, int start, int end, bool fromEnd) const;
    void setPermIndex(bool corners, int start, int end, bool fromEnd, int index);
};

} // namespace cubejs
