// solver.cpp -- port of src/solve.coffee (Kociemba's two-phase algorithm)
#include "cubejs/solver.hpp"

#include <algorithm>
#include <functional>
#include <sstream>

namespace cubejs {

// --- Static storage ----------------------------------------------------

bool Solver::initialized_ = false;
std::vector<std::vector<int>> Solver::twistTable_;
std::vector<std::vector<int>> Solver::flipTable_;
std::vector<std::vector<int>> Solver::FRtoBRTable_;
std::vector<std::vector<int>> Solver::URFtoDLFTable_;
std::vector<std::vector<int>> Solver::URtoDFTable_;
std::vector<std::vector<int>> Solver::URtoULTable_;
std::vector<std::vector<int>> Solver::UBtoDFTable_;
std::vector<std::vector<int>> Solver::mergeURtoDFTable_;
std::vector<uint32_t> Solver::sliceTwistPruning_;
std::vector<uint32_t> Solver::sliceFlipPruning_;
std::vector<uint32_t> Solver::sliceURFtoDLFParityPruning_;
std::vector<uint32_t> Solver::sliceURtoDFParityPruning_;

namespace {

// --- Coordinate space sizes (same constants as solve.coffee) -----------

constexpr int N_TWIST = 2187;     // 3^7 corner orientations
constexpr int N_FLIP = 2048;      // 2^11 possible edge flips
constexpr int N_PARITY = 2;       // 2 possible parities

constexpr int N_FRtoBR = 11880;   // 12!/(12-4)! permutations of FR..BR edges
constexpr int N_SLICE1 = 495;     // (12 choose 4) possible positions of FR..BR edges
constexpr int N_SLICE2 = 24;      // 4! permutations of FR..BR edges in phase 2

constexpr int N_URFtoDLF = 20160; // 8!/(8-6)! permutations of URF..DLF corners
constexpr int N_URtoDF = 20160;   // permutation of UR..DF edges in phase 2
constexpr int N_URtoUL = 1320;    // 12!/(12-3)! permutations of UR..UL edges
constexpr int N_UBtoDF = 1320;    // 12!/(12-3)! permutations of UB..DF edges

constexpr int parityTable[2][18] = {
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
};

// Phase 1: all 18 basic moves are valid.
const std::vector<int>& allMoves1() {
    static const std::vector<int> v = [] {
        std::vector<int> r;
        for (int i = 0; i <= 17; i++) r.push_back(i);
        return r;
    }();
    return v;
}

// Phase 2: double moves of all faces, plus quarter moves of U and D.
const std::vector<int>& allMoves2() {
    static const std::vector<int> v = {0, 1, 2, 4, 7, 9, 10, 11, 13, 16};
    return v;
}

// The list of next valid phase 1 moves when `lastFace` was turned last.
// Disallows commuting moves (e.g. U U') and enforces a canonical order
// for opposite faces (U before D, R before L, F before B) to avoid
// redundant move sequences like U D U'.
const std::vector<std::vector<int>>& nextMoves1() {
    static const std::vector<std::vector<int>> table = [] {
        std::vector<std::vector<int>> result(6);
        for (int lastFace = 0; lastFace <= 5; lastFace++) {
            for (int face = 0; face <= 5; face++) {
                if (face == lastFace || face == lastFace - 3) continue;
                for (int power = 0; power <= 2; power++) {
                    result[lastFace].push_back(face * 3 + power);
                }
            }
        }
        return result;
    }();
    return table;
}

const std::vector<std::vector<int>>& nextMoves2() {
    static const std::vector<std::vector<int>> table = [] {
        std::vector<std::vector<int>> result(6);
        for (int lastFace = 0; lastFace <= 5; lastFace++) {
            for (int face = 0; face <= 5; face++) {
                if (face == lastFace || face == lastFace - 3) continue;
                if (face == U || face == D) {
                    for (int power = 0; power <= 2; power++) result[lastFace].push_back(face * 3 + power);
                } else {
                    result[lastFace].push_back(face * 3 + 1);
                }
            }
        }
        return result;
    }();
    return table;
}

// --- 4-bit packed pruning table access ------------------------------------

inline int pruningGet(const std::vector<uint32_t>& table, long long index) {
    int pos = static_cast<int>(index % 8);
    long long slot = index >> 3;
    int shift = pos << 2;
    return static_cast<int>((table[slot] >> shift) & 0xFu);
}

inline void pruningSet(std::vector<uint32_t>& table, long long index, int value) {
    int pos = static_cast<int>(index % 8);
    long long slot = index >> 3;
    int shift = pos << 2;
    table[slot] &= ~(static_cast<uint32_t>(0xF) << shift);
    table[slot] |= (static_cast<uint32_t>(value) << shift);
}

// --- Move table generation -------------------------------------------------

using CoordGetter = int (Cube::*)() const;
using CoordSetter = void (Cube::*)(int);

// Loops through all valid values for the coordinate, setting the cube's
// state in each iteration, then applies each of the 18 basic moves to
// the cube and records the resulting coordinate.
std::vector<std::vector<int>> computeMoveTable(bool corners, CoordGetter getter,
                                                CoordSetter setter, int size) {
    Cube cube;
    std::vector<std::vector<int>> table(size, std::vector<int>(18));
    const auto& moves = Cube::moves();

    for (int i = 0; i < size; i++) {
        (cube.*setter)(i);
        int idx = 0;
        for (int j = 0; j <= 5; j++) {
            const MoveDef& mv = moves[j];
            for (int k = 0; k <= 2; k++) {
                if (corners) cube.cornerMultiply(mv); else cube.edgeMultiply(mv);
                table[i][idx++] = (cube.*getter)();
            }
            // 4th application restores the cube before moving to the next face.
            if (corners) cube.cornerMultiply(mv); else cube.edgeMultiply(mv);
        }
    }
    return table;
}

int mergeURtoDFCompute(Cube& a, Cube& b, int URtoUL, int UBtoDF) {
    a.setURtoUL(URtoUL);
    b.setUBtoDF(UBtoDF);
    for (int i = 0; i <= 7; i++) {
        if (a.ep[i] != -1) {
            if (b.ep[i] != -1) return -1;  // collision
            b.ep[i] = a.ep[i];
        }
    }
    return b.URtoDF();
}

std::vector<uint32_t> computePruningTable(int size, const std::vector<int>& moves,
                                           const std::function<int(int, int)>& nextIndex) {
    std::vector<uint32_t> table((size + 7) / 8, 0xFFFFFFFFu);

    int depth = 0;
    pruningSet(table, 0, depth);
    int done = 1;

    while (done != size) {
        for (int index = 0; index < size; index++) {
            if (pruningGet(table, index) == depth) {
                for (int move : moves) {
                    int next = nextIndex(index, move);
                    if (pruningGet(table, next) == 0xF) {
                        pruningSet(table, next, depth + 1);
                        done++;
                    }
                }
            }
        }
        depth++;
    }

    return table;
}

std::string moveNameStr(int code) {
    static const char* names[18] = {
        "U", "U2", "U'", "R", "R2", "R'", "F", "F2", "F'",
        "D", "D2", "D'", "L", "L2", "L'", "B", "B2", "B'",
    };
    return names[code];
}

}  // namespace

// --- Search state ------------------------------------------------------

struct Solver::SearchState {
    SearchState* parent = nullptr;
    int lastMove = -1;
    int depth = 0;

    // Phase 1 coordinates
    int flip = 0, twist = 0, slice = 0;

    // Phase 2 coordinates
    int parity = 0, URFtoDLF = 0, FRtoBR = 0;

    // Merged into URtoDF when phase 2 begins
    int URtoUL = 0, UBtoDF = 0, URtoDF = 0;
};

std::string Solver::buildSolution(const SearchState& s) {
    if (s.parent) return buildSolution(*s.parent) + moveNameStr(s.lastMove) + " ";
    return "";
}

// --- Table construction --------------------------------------------------

void Solver::init() {
    if (initialized_) return;

    twistTable_ = computeMoveTable(true, &Cube::twist, &Cube::setTwist, N_TWIST);
    flipTable_ = computeMoveTable(false, &Cube::flip, &Cube::setFlip, N_FLIP);
    FRtoBRTable_ = computeMoveTable(false, &Cube::FRtoBR, &Cube::setFRtoBR, N_FRtoBR);
    URFtoDLFTable_ = computeMoveTable(true, &Cube::URFtoDLF, &Cube::setURFtoDLF, N_URFtoDLF);
    URtoDFTable_ = computeMoveTable(false, &Cube::URtoDF, &Cube::setURtoDF, N_URtoDF);
    URtoULTable_ = computeMoveTable(false, &Cube::URtoUL, &Cube::setURtoUL, N_URtoUL);
    UBtoDFTable_ = computeMoveTable(false, &Cube::UBtoDF, &Cube::setUBtoDF, N_UBtoDF);

    mergeURtoDFTable_.assign(336, std::vector<int>(336));
    {
        Cube a, b;
        for (int u = 0; u < 336; u++) {
            for (int v = 0; v < 336; v++) {
                mergeURtoDFTable_[u][v] = mergeURtoDFCompute(a, b, u, v);
            }
        }
    }

    sliceTwistPruning_ = computePruningTable(
        N_SLICE1 * N_TWIST, allMoves1(),
        [](int index, int move) -> int {
            int slice = index % N_SLICE1;
            int twist = index / N_SLICE1;
            int newSlice = FRtoBRTable_[slice * 24][move] / 24;
            int newTwist = twistTable_[twist][move];
            return newTwist * N_SLICE1 + newSlice;
        });

    sliceFlipPruning_ = computePruningTable(
        N_SLICE1 * N_FLIP, allMoves1(),
        [](int index, int move) -> int {
            int slice = index % N_SLICE1;
            int flip = index / N_SLICE1;
            int newSlice = FRtoBRTable_[slice * 24][move] / 24;
            int newFlip = flipTable_[flip][move];
            return newFlip * N_SLICE1 + newSlice;
        });

    sliceURFtoDLFParityPruning_ = computePruningTable(
        N_SLICE2 * N_URFtoDLF * N_PARITY, allMoves2(),
        [](int index, int move) -> int {
            int parity = index % 2;
            int slice = (index / 2) % N_SLICE2;
            int URFtoDLF = (index / 2) / N_SLICE2;
            int newParity = parityTable[parity][move];
            int newSlice = FRtoBRTable_[slice][move];
            int newURFtoDLF = URFtoDLFTable_[URFtoDLF][move];
            return (newURFtoDLF * N_SLICE2 + newSlice) * 2 + newParity;
        });

    sliceURtoDFParityPruning_ = computePruningTable(
        N_SLICE2 * N_URtoDF * N_PARITY, allMoves2(),
        [](int index, int move) -> int {
            int parity = index % 2;
            int slice = (index / 2) % N_SLICE2;
            int URtoDF = (index / 2) / N_SLICE2;
            int newParity = parityTable[parity][move];
            int newSlice = FRtoBRTable_[slice][move];
            int newURtoDF = URtoDFTable_[URtoDF][move];
            return (newURtoDF * N_SLICE2 + newSlice) * 2 + newParity;
        });

    initialized_ = true;
}

// --- Search ----------------------------------------------------------------

void Solver::initState(SearchState& s, const Cube& cube) {
    s.parent = nullptr;
    s.lastMove = -1;
    s.depth = 0;

    s.flip = cube.flip();
    s.twist = cube.twist();
    s.slice = cube.FRtoBR() / N_SLICE2;

    s.parity = cube.cornerParity();
    s.URFtoDLF = cube.URFtoDLF();
    s.FRtoBR = cube.FRtoBR();

    s.URtoUL = cube.URtoUL();
    s.UBtoDF = cube.UBtoDF();
}

int Solver::minDist1(const SearchState& s) {
    int d1 = pruningGet(sliceFlipPruning_, static_cast<long long>(N_SLICE1) * s.flip + s.slice);
    int d2 = pruningGet(sliceTwistPruning_, static_cast<long long>(N_SLICE1) * s.twist + s.slice);
    return std::max(d1, d2);
}

Solver::SearchState Solver::next1(SearchState& s, int move) {
    SearchState n;
    n.parent = &s;
    n.lastMove = move;
    n.depth = s.depth + 1;

    n.flip = flipTable_[s.flip][move];
    n.twist = twistTable_[s.twist][move];
    n.slice = FRtoBRTable_[s.slice * 24][move] / 24;

    return n;
}

int Solver::minDist2(const SearchState& s) {
    int index1 = (N_SLICE2 * s.URtoDF + s.FRtoBR) * N_PARITY + s.parity;
    int d1 = pruningGet(sliceURtoDFParityPruning_, index1);

    int index2 = (N_SLICE2 * s.URFtoDLF + s.FRtoBR) * N_PARITY + s.parity;
    int d2 = pruningGet(sliceURFtoDLFParityPruning_, index2);

    return std::max(d1, d2);
}

void Solver::initPhase2(SearchState& s, bool top) {
    if (s.parent == nullptr) return;  // already assigned for the initial state

    initPhase2(*s.parent, false);

    s.URFtoDLF = URFtoDLFTable_[s.parent->URFtoDLF][s.lastMove];
    s.FRtoBR = FRtoBRTable_[s.parent->FRtoBR][s.lastMove];
    s.parity = parityTable[s.parent->parity][s.lastMove];
    s.URtoUL = URtoULTable_[s.parent->URtoUL][s.lastMove];
    s.UBtoDF = UBtoDFTable_[s.parent->UBtoDF][s.lastMove];

    if (top) {
        s.URtoDF = mergeURtoDFTable_[s.URtoUL][s.UBtoDF];
    }
}

Solver::SearchState Solver::next2(SearchState& s, int move) {
    SearchState n;
    n.parent = &s;
    n.lastMove = move;
    n.depth = s.depth + 1;

    n.URFtoDLF = URFtoDLFTable_[s.URFtoDLF][move];
    n.FRtoBR = FRtoBRTable_[s.FRtoBR][move];
    n.parity = parityTable[s.parity][move];
    n.URtoDF = URtoDFTable_[s.URtoDF][move];

    return n;
}

void Solver::phase2(SearchState& state, int depth, std::optional<std::string>& solution) {
    if (depth == 0) {
        if (minDist2(state) == 0) {
            solution = buildSolution(state);
        }
    } else {
        if (minDist2(state) <= depth) {
            const auto& candidates =
                (state.lastMove == -1) ? allMoves2() : nextMoves2()[state.lastMove / 3];
            for (int move : candidates) {
                SearchState next = next2(state, move);
                phase2(next, depth - 1, solution);
                if (solution) return;
            }
        }
    }
}

void Solver::phase2search(SearchState& state, int maxDepthTotal,
                           std::optional<std::string>& solution) {
    initPhase2(state, true);

    int limit = maxDepthTotal - state.depth;
    for (int depth = 1; depth <= limit && !solution; depth++) {
        phase2(state, depth, solution);
    }
}

void Solver::phase1(SearchState& state, int depth, int maxDepthTotal,
                     std::optional<std::string>& solution) {
    if (depth == 0) {
        if (minDist1(state) == 0) {
            bool lastIsPhase2Move = false;
            if (state.lastMove != -1) {
                const auto& m2 = allMoves2();
                lastIsPhase2Move = std::find(m2.begin(), m2.end(), state.lastMove) != m2.end();
            }
            if (state.lastMove == -1 || !lastIsPhase2Move) {
                phase2search(state, maxDepthTotal, solution);
            }
        }
    } else {
        if (minDist1(state) <= depth) {
            const auto& candidates =
                (state.lastMove == -1) ? allMoves1() : nextMoves1()[state.lastMove / 3];
            for (int move : candidates) {
                SearchState next = next1(state, move);
                phase1(next, depth - 1, maxDepthTotal, solution);
                if (solution) return;
            }
        }
    }
}

std::optional<std::string> Solver::solveUpright(const Cube& cube, int maxDepth) {
    if (!initialized_) init();

    SearchState root;
    initState(root, cube);

    std::optional<std::string> solution;
    for (int depth = 1; depth <= maxDepth && !solution; depth++) {
        phase1(root, depth, maxDepth, solution);
    }

    if (!solution) return std::nullopt;

    std::string s = *solution;
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

std::optional<std::string> Solver::solve(const Cube& cube, int maxDepth) {
    Cube c = cube.clone();
    std::string uprightAlg = c.upright();
    c.move(uprightAlg);

    Cube rotationCube;
    rotationCube.move(uprightAlg);
    std::array<int, 6> rotation = rotationCube.center;

    auto uprightSolution = solveUpright(c, maxDepth);
    if (!uprightSolution) return std::nullopt;

    static const char faceNames[6] = {'U', 'R', 'F', 'D', 'L', 'B'};
    auto faceNum = [](char ch) -> int {
        switch (ch) {
            case 'U': return 0;
            case 'R': return 1;
            case 'F': return 2;
            case 'D': return 3;
            case 'L': return 4;
            case 'B': return 5;
            default: return -1;
        }
    };

    std::string result;
    std::istringstream iss(*uprightSolution);
    std::string token;
    bool first = true;
    while (iss >> token) {
        int origFace = faceNum(token[0]);
        int newFace = rotation[origFace];
        if (!first) result += ' ';
        first = false;
        result += faceNames[newFace];
        if (token.size() > 1) result += token[1];
    }

    return result;
}

std::string Solver::scramble() {
    Cube c = Cube::random();
    auto sol = solve(c, 22);
    if (!sol) return std::string();
    return Cube::inverse(*sol);
}

}  // namespace cubejs
