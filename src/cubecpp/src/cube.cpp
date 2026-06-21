// cube.cpp -- port of src/cube.coffee
#include "cubejs/cube.hpp"
#include "cubejs/solver.hpp"

#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace cubejs {

namespace {

// --- Facelet layout (matches the diagram in cube.hpp) --------------------

constexpr int centerFacelet[6] = {4, 13, 22, 31, 40, 49};

constexpr int cornerFacelet[8][3] = {
    {8, 9, 20},   {6, 18, 38},  {0, 36, 47},  {2, 45, 11},
    {29, 26, 15}, {27, 44, 24}, {33, 53, 42}, {35, 17, 51},
};

constexpr int edgeFacelet[12][2] = {
    {5, 10},  {7, 19},  {3, 37},  {1, 46},  {32, 16}, {28, 25},
    {30, 43}, {34, 52}, {23, 12}, {21, 41}, {50, 39}, {48, 14},
};

constexpr char centerColor[6] = {'U', 'R', 'F', 'D', 'L', 'B'};

constexpr char cornerColor[8][3] = {
    {'U', 'R', 'F'}, {'U', 'F', 'L'}, {'U', 'L', 'B'}, {'U', 'B', 'R'},
    {'D', 'F', 'R'}, {'D', 'L', 'F'}, {'D', 'B', 'L'}, {'D', 'R', 'B'},
};

constexpr char edgeColor[12][2] = {
    {'U', 'R'}, {'U', 'F'}, {'U', 'L'}, {'U', 'B'}, {'D', 'R'}, {'D', 'F'},
    {'D', 'L'}, {'D', 'B'}, {'F', 'R'}, {'F', 'L'}, {'B', 'L'}, {'B', 'R'},
};

// --- Move name <-> code ----------------------------------------------------

const std::unordered_map<char, int>& faceNumsMap() {
    static const std::unordered_map<char, int> m = {
        {'U', 0}, {'R', 1}, {'F', 2}, {'D', 3}, {'L', 4}, {'B', 5},
        {'E', 6}, {'M', 7}, {'S', 8},
        {'x', 9}, {'y', 10}, {'z', 11},
        {'u', 12}, {'r', 13}, {'f', 14}, {'d', 15}, {'l', 16}, {'b', 17},
    };
    return m;
}

constexpr const char* faceNames[18] = {
    "U", "R", "F", "D", "L", "B", "E", "M", "S",
    "x", "y", "z", "u", "r", "f", "d", "l", "b",
};

// --- Combinatorics helpers (used by the permutation coordinates) ---------

long long Cnk(int n, int k) {
    if (n < k) return 0;
    if (k > n / 2) k = n - k;
    long long s = 1;
    int i = n;
    int j = 1;
    while (i != n - k) {
        s *= i;
        s /= j;
        i--;
        j++;
    }
    return s;
}

long long factorial(int n) {
    long long f = 1;
    for (int i = 2; i <= n; i++) f *= i;
    return f;
}

void rotateLeft(std::vector<int>& a, int l, int r) {
    int tmp = a[l];
    for (int i = l; i <= r - 1; i++) a[i] = a[i + 1];
    a[r] = tmp;
}

void rotateRight(std::vector<int>& a, int l, int r) {
    int tmp = a[r];
    for (int i = r; i >= l + 1; i--) a[i] = a[i - 1];
    a[l] = tmp;
}

// --- Randomization ---------------------------------------------------------

std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

int randint(int mn, int mx) {
    std::uniform_int_distribution<int> dist(mn, mx);
    return dist(rng());
}

int getNumSwaps(const std::vector<int>& arr) {
    int n = static_cast<int>(arr.size());
    std::vector<bool> seen(n, false);
    int numSwaps = 0;
    while (true) {
        int cur = -1;
        for (int i = 0; i < n; i++) {
            if (!seen[i]) { cur = i; break; }
        }
        if (cur == -1) break;
        int cycleLength = 0;
        while (!seen[cur]) {
            seen[cur] = true;
            cycleLength++;
            cur = arr[cur];
        }
        numSwaps += cycleLength + 1;
    }
    return numSwaps;
}

bool arePermutationsValid(const std::vector<int>& cp, const std::vector<int>& ep) {
    return (getNumSwaps(ep) + getNumSwaps(cp)) % 2 == 0;
}

void generateValidRandomPermutation(std::vector<int>& cp, std::vector<int>& ep) {
    std::shuffle(ep.begin(), ep.end(), rng());
    std::shuffle(cp.begin(), cp.end(), rng());
    while (!arePermutationsValid(cp, ep)) {
        std::shuffle(ep.begin(), ep.end(), rng());
        std::shuffle(cp.begin(), cp.end(), rng());
    }
}

void randomizeOrientation(std::vector<int>& arr, int numOrientations) {
    for (auto& v : arr) v = randint(0, numOrientations - 1);
}

bool isOrientationValid(const std::vector<int>& arr, int numOrientations) {
    int sum = 0;
    for (int v : arr) sum += v;
    return sum % numOrientations == 0;
}

void generateValidRandomOrientation(std::vector<int>& co, std::vector<int>& eo) {
    randomizeOrientation(co, 3);
    while (!isOrientationValid(co, 3)) randomizeOrientation(co, 3);
    randomizeOrientation(eo, 2);
    while (!isOrientationValid(eo, 2)) randomizeOrientation(eo, 2);
}

// --- The 18 face transformations -------------------------------------------
//
// The first 9 (U, R, F, D, L, B, E, M, S) are the canonical moves. The
// remaining 9 (x, y, z, u, r, f, d, l, b) are composites, computed by
// applying an algorithm to a solved cube -- exactly like the original
// CoffeeScript does (`Cube.moves.push new Cube().move("R M' L'").toJSON()`).
//
// We can't go through Cube::move()/Cube::moves() here (that would be a
// chicken-and-egg problem, since moves() is what we're building), so we
// apply moves directly against the in-progress table `m`.
std::vector<MoveDef> buildMoves() {
    std::vector<MoveDef> m;
    m.reserve(18);

    m.push_back(MoveDef{
        {U, R, F, D, L, B},
        {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {UB, UR, UF, UL, DR, DF, DL, DB, FR, FL, BL, BR},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    });  // U

    m.push_back(MoveDef{
        {U, R, F, D, L, B},
        {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR},
        {2, 0, 0, 1, 1, 0, 0, 2},
        {FR, UF, UL, UB, BR, DF, DL, DB, DR, FL, BL, UR},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    });  // R

    m.push_back(MoveDef{
        {U, R, F, D, L, B},
        {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB},
        {1, 2, 0, 0, 2, 1, 0, 0},
        {UR, FL, UL, UB, DR, FR, DL, DB, UF, DF, BL, BR},
        {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0},
    });  // F

    m.push_back(MoveDef{
        {U, R, F, D, L, B},
        {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    });  // D

    m.push_back(MoveDef{
        {U, R, F, D, L, B},
        {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB},
        {0, 1, 2, 0, 0, 2, 1, 0},
        {UR, UF, BL, UB, DR, DF, FL, DB, FR, UL, DL, BR},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    });  // L

    m.push_back(MoveDef{
        {U, R, F, D, L, B},
        {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL},
        {0, 0, 1, 2, 0, 0, 2, 1},
        {UR, UF, UL, BR, DR, DF, DL, BL, FR, FL, UB, DB},
        {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1},
    });  // B

    m.push_back(MoveDef{
        {U, F, L, D, B, R},
        {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {UR, UF, UL, UB, DR, DF, DL, DB, FL, BL, BR, FR},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
    });  // E

    m.push_back(MoveDef{
        {B, R, U, F, L, D},
        {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {UR, UB, UL, DB, DR, UF, DL, DF, FR, FL, BL, BR},
        {0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
    });  // M

    m.push_back(MoveDef{
        {L, U, F, R, D, B},
        {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {UL, UF, DL, UB, UR, DF, DR, DB, FR, FL, BL, BR},
        {1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0},
    });  // S

    auto composite = [&](const std::string& alg) {
        Cube c;
        for (int code : Cube::parseAlg(alg)) {
            int face = code / 3;
            int power = code % 3;
            for (int x = 0; x <= power; x++) c.multiply(m[face]);
        }
        m.push_back(static_cast<CubeState>(c));
    };

    composite("R M' L'");  // x
    composite("U E' D'");  // y
    composite("F S B'");   // z
    composite("U E'");     // u
    composite("R M'");     // r
    composite("F S");      // f
    composite("D E");      // d
    composite("L M");      // l
    composite("B S'");     // b

    return m;
}

int inverseMoveCode(int move) {
    int face = move / 3;
    int power = move % 3;
    return face * 3 + (2 - power);
}

}  // namespace

// --- Construction / basic state --------------------------------------------

Cube::Cube() { identity(); }

void Cube::identity() {
    for (int i = 0; i <= 5; i++) center[i] = i;
    for (int i = 0; i <= 7; i++) { cp[i] = i; co[i] = 0; }
    for (int i = 0; i <= 11; i++) { ep[i] = i; eo[i] = 0; }
}

void Cube::init(const Cube& other) {
    center = other.center;
    cp = other.cp;
    co = other.co;
    ep = other.ep;
    eo = other.eo;
}

const std::vector<MoveDef>& Cube::moves() {
    static const std::vector<MoveDef> table = buildMoves();
    return table;
}

// --- Facelet conversion ------------------------------------------------

std::string Cube::asString() const {
    std::string result(54, ' ');

    for (int i = 0; i <= 5; i++) result[centerFacelet[i]] = centerColor[center[i]];

    for (int i = 0; i <= 7; i++) {
        int corner = cp[i];
        int ori = co[i];
        for (int n = 0; n <= 2; n++) {
            result[cornerFacelet[i][(n + ori) % 3]] = cornerColor[corner][n];
        }
    }

    for (int i = 0; i <= 11; i++) {
        int edge = ep[i];
        int ori = eo[i];
        for (int n = 0; n <= 1; n++) {
            result[edgeFacelet[i][(n + ori) % 2]] = edgeColor[edge][n];
        }
    }

    return result;
}

Cube Cube::fromString(const std::string& str) {
    Cube cube;

    if (str.size() != 54) {
        throw std::invalid_argument("Cube facelet string must be exactly 54 characters");
    }

    for (int i = 0; i <= 5; i++) {
        for (int j = 0; j <= 5; j++) {
            if (str[centerFacelet[i]] == centerColor[j]) cube.center[i] = j;
        }
    }

    for (int i = 0; i <= 7; i++) {
        int ori = 0;
        for (ori = 0; ori <= 2; ori++) {
            char c = str[cornerFacelet[i][ori]];
            if (c == 'U' || c == 'D') break;
        }
        char col1 = str[cornerFacelet[i][(ori + 1) % 3]];
        char col2 = str[cornerFacelet[i][(ori + 2) % 3]];

        for (int j = 0; j <= 7; j++) {
            if (col1 == cornerColor[j][1] && col2 == cornerColor[j][2]) {
                cube.cp[i] = j;
                cube.co[i] = ori % 3;
            }
        }
    }

    for (int i = 0; i <= 11; i++) {
        for (int j = 0; j <= 11; j++) {
            if (str[edgeFacelet[i][0]] == edgeColor[j][0] &&
                str[edgeFacelet[i][1]] == edgeColor[j][1]) {
                cube.ep[i] = j;
                cube.eo[i] = 0;
                break;
            }
            if (str[edgeFacelet[i][0]] == edgeColor[j][1] &&
                str[edgeFacelet[i][1]] == edgeColor[j][0]) {
                cube.ep[i] = j;
                cube.eo[i] = 1;
                break;
            }
        }
    }

    return cube;
}

// --- isSolved / randomize -----------------------------------------------

bool Cube::isSolved() const {
    Cube c = clone();
    c.move(c.upright());

    for (int i = 0; i <= 5; i++) if (c.center[i] != i) return false;
    for (int i = 0; i <= 7; i++) {
        if (c.cp[i] != i) return false;
        if (c.co[i] != 0) return false;
    }
    for (int i = 0; i <= 11; i++) {
        if (c.ep[i] != i) return false;
        if (c.eo[i] != 0) return false;
    }
    return true;
}

void Cube::randomize() {
    std::vector<int> cpv(cp.begin(), cp.end());
    std::vector<int> epv(ep.begin(), ep.end());
    std::vector<int> cov(co.begin(), co.end());
    std::vector<int> eov(eo.begin(), eo.end());

    generateValidRandomPermutation(cpv, epv);
    generateValidRandomOrientation(cov, eov);

    std::copy(cpv.begin(), cpv.end(), cp.begin());
    std::copy(epv.begin(), epv.end(), ep.begin());
    std::copy(cov.begin(), cov.end(), co.begin());
    std::copy(eov.begin(), eov.end(), eo.begin());
}

Cube Cube::random() {
    Cube c;
    c.randomize();
    return c;
}

// --- Multiplication / move application -------------------------------

Cube& Cube::centerMultiply(const CubeState& other) {
    std::array<int, 6> next{};
    for (int to = 0; to <= 5; to++) next[to] = center[other.center[to]];
    center = next;
    return *this;
}

Cube& Cube::cornerMultiply(const CubeState& other) {
    std::array<int, 8> nextCp{}, nextCo{};
    for (int to = 0; to <= 7; to++) {
        int from = other.cp[to];
        nextCp[to] = cp[from];
        nextCo[to] = (co[from] + other.co[to]) % 3;
    }
    cp = nextCp;
    co = nextCo;
    return *this;
}

Cube& Cube::edgeMultiply(const CubeState& other) {
    std::array<int, 12> nextEp{}, nextEo{};
    for (int to = 0; to <= 11; to++) {
        int from = other.ep[to];
        nextEp[to] = ep[from];
        nextEo[to] = (eo[from] + other.eo[to]) % 2;
    }
    ep = nextEp;
    eo = nextEo;
    return *this;
}

Cube& Cube::multiply(const CubeState& other) {
    centerMultiply(other);
    cornerMultiply(other);
    edgeMultiply(other);
    return *this;
}

std::vector<int> Cube::parseAlg(const std::string& alg) {
    const auto& faceNums = faceNumsMap();
    std::vector<int> result;
    std::istringstream iss(alg);
    std::string part;
    while (iss >> part) {
        if (part.size() > 2) {
            throw std::invalid_argument("Invalid move: " + part);
        }
        auto it = faceNums.find(part[0]);
        if (it == faceNums.end()) {
            throw std::invalid_argument("Invalid move: " + part);
        }
        int power;
        if (part.size() == 1) {
            power = 0;
        } else if (part[1] == '2') {
            power = 1;
        } else if (part[1] == '\'') {
            power = 2;
        } else {
            throw std::invalid_argument("Invalid move: " + part);
        }
        result.push_back(it->second * 3 + power);
    }
    return result;
}

std::string Cube::algToString(const std::vector<int>& moveCodes) {
    std::string out;
    for (int m : moveCodes) {
        int face = m / 3;
        int power = m % 3;
        out += faceNames[face];
        if (power == 1) out += '2';
        else if (power == 2) out += '\'';
        out += ' ';
    }
    if (!out.empty()) out.pop_back();
    return out;
}

Cube& Cube::move(int moveCode) {
    const auto& table = Cube::moves();
    int face = moveCode / 3;
    int power = moveCode % 3;
    for (int x = 0; x <= power; x++) multiply(table[face]);
    return *this;
}

Cube& Cube::move(const std::vector<int>& moveCodes) {
    for (int code : moveCodes) move(code);
    return *this;
}

Cube& Cube::move(const std::string& alg) { return move(parseAlg(alg)); }

std::string Cube::upright() const {
    Cube c = clone();
    std::vector<std::string> result;

    int i = -1;
    for (int k = 0; k <= 5; k++) {
        if (c.center[k] == F) { i = k; break; }
    }
    switch (i) {
        case D: result.push_back("x"); break;
        case U: result.push_back("x'"); break;
        case B: result.push_back("x2"); break;
        case R: result.push_back("y"); break;
        case L: result.push_back("y'"); break;
        default: break;
    }
    if (!result.empty()) c.move(result[0]);

    int j = -1;
    for (int k = 0; k <= 5; k++) {
        if (c.center[k] == U) { j = k; break; }
    }
    switch (j) {
        case L: result.push_back("z"); break;
        case R: result.push_back("z'"); break;
        case D: result.push_back("z2"); break;
        default: break;
    }

    std::string out;
    for (size_t k = 0; k < result.size(); k++) {
        if (k) out += ' ';
        out += result[k];
    }
    return out;
}

std::vector<int> Cube::simplify(const std::vector<int>& moveCodes) {
    // Adjacent same-face quarter turns are merged mod 4 (1=single,
    // 2=double, 3=inverse). The stack always maintains the invariant
    // "no two adjacent entries share a face", so a new token can only
    // ever interact with the current top -- no cascading re-checks needed.
    struct Item { int face; int count; };
    std::vector<Item> stack;

    for (int code : moveCodes) {
        int face = code / 3;
        int count = (code % 3) + 1;

        if (!stack.empty() && stack.back().face == face) {
            int merged = (stack.back().count + count) % 4;
            stack.pop_back();
            if (merged != 0) stack.push_back({face, merged});
        } else {
            stack.push_back({face, count});
        }
    }

    std::vector<int> result;
    result.reserve(stack.size());
    for (const auto& item : stack) result.push_back(item.face * 3 + (item.count - 1));
    return result;
}

std::string Cube::simplify(const std::string& alg) {
    return algToString(simplify(parseAlg(alg)));
}

int Cube::inverse(int moveCode) { return inverseMoveCode(moveCode); }

std::vector<int> Cube::inverse(const std::vector<int>& moveCodes) {
    std::vector<int> result;
    result.reserve(moveCodes.size());
    for (int m : moveCodes) result.push_back(inverseMoveCode(m));
    std::reverse(result.begin(), result.end());
    return result;
}

std::string Cube::inverse(const std::string& alg) {
    return algToString(inverse(parseAlg(alg)));
}

// --- Solver coordinates --------------------------------------------------

int Cube::twist() const {
    int v = 0;
    for (int i = 0; i <= 6; i++) v = 3 * v + co[i];
    return v;
}

void Cube::setTwist(int t) {
    int parity = 0;
    for (int i = 6; i >= 0; i--) {
        int ori = t % 3;
        t /= 3;
        co[i] = ori;
        parity += ori;
    }
    co[7] = (3 - parity % 3) % 3;
}

int Cube::flip() const {
    int v = 0;
    for (int i = 0; i <= 10; i++) v = 2 * v + eo[i];
    return v;
}

void Cube::setFlip(int flipVal) {
    int parity = 0;
    for (int i = 10; i >= 0; i--) {
        int ori = flipVal % 2;
        flipVal /= 2;
        eo[i] = ori;
        parity += ori;
    }
    eo[11] = (2 - parity % 2) % 2;
}

int Cube::cornerParity() const {
    int s = 0;
    for (int i = 1; i <= DRB; i++)
        for (int j = 0; j < i; j++)
            if (cp[j] > cp[i]) s++;
    return s % 2;
}

int Cube::edgeParity() const {
    int s = 0;
    for (int i = 1; i <= BR; i++)
        for (int j = 0; j < i; j++)
            if (ep[j] > ep[i]) s++;
    return s % 2;
}

int Cube::permIndex(bool corners, int start, int end, bool fromEnd) const {
    int maxOur = end - start;
    long long maxB = factorial(maxOur + 1);
    int maxAll = corners ? 7 : 11;
    const int* perm = corners ? cp.data() : ep.data();

    std::vector<int> our(maxOur + 1, -1);
    long long a = 0, b = 0;
    int x = 0;

    if (fromEnd) {
        for (int j = maxAll; j >= 0; j--) {
            if (perm[j] >= start && perm[j] <= end) {
                a += Cnk(maxAll - j, x + 1);
                our[maxOur - x] = perm[j];
                x++;
            }
        }
    } else {
        for (int j = 0; j <= maxAll; j++) {
            if (perm[j] >= start && perm[j] <= end) {
                a += Cnk(j, x + 1);
                our[x] = perm[j];
                x++;
            }
        }
    }

    for (int j = maxOur; j >= 0; j--) {
        int k = 0;
        while (our[j] != start + j) {
            rotateLeft(our, 0, j);
            k++;
        }
        b = static_cast<long long>(j + 1) * b + k;
    }

    return static_cast<int>(a * maxB + b);
}

void Cube::setPermIndex(bool corners, int start, int end, bool fromEnd, int index) {
    int maxOur = end - start;
    long long maxB = factorial(maxOur + 1);
    int maxAll = corners ? 7 : 11;
    int* perm = corners ? cp.data() : ep.data();

    std::vector<int> our(maxOur + 1);
    for (int i = 0; i <= maxOur; i++) our[i] = i + start;

    long long b = index % maxB;
    long long a = index / maxB;

    for (int i = 0; i <= maxAll; i++) perm[i] = -1;

    for (int j = 1; j <= maxOur; j++) {
        int k = static_cast<int>(b % (j + 1));
        b /= (j + 1);
        while (k > 0) {
            rotateRight(our, 0, j);
            k--;
        }
    }

    int x = maxOur;
    if (fromEnd) {
        for (int j = 0; j <= maxAll; j++) {
            long long c = Cnk(maxAll - j, x + 1);
            if (a - c >= 0) {
                perm[j] = our[maxOur - x];
                a -= c;
                x--;
            }
        }
    } else {
        for (int j = maxAll; j >= 0; j--) {
            long long c = Cnk(j, x + 1);
            if (a - c >= 0) {
                perm[j] = our[x];
                a -= c;
                x--;
            }
        }
    }
}

int Cube::URFtoDLF() const { return permIndex(true, URF, DLF, false); }
void Cube::setURFtoDLF(int idx) { setPermIndex(true, URF, DLF, false, idx); }

int Cube::URtoUL() const { return permIndex(false, UR, UL, false); }
void Cube::setURtoUL(int idx) { setPermIndex(false, UR, UL, false, idx); }

int Cube::UBtoDF() const { return permIndex(false, UB, DF, false); }
void Cube::setUBtoDF(int idx) { setPermIndex(false, UB, DF, false, idx); }

int Cube::URtoDF() const { return permIndex(false, UR, DF, false); }
void Cube::setURtoDF(int idx) { setPermIndex(false, UR, DF, false, idx); }

int Cube::FRtoBR() const { return permIndex(false, FR, BR, true); }
void Cube::setFRtoBR(int idx) { setPermIndex(false, FR, BR, true, idx); }

// --- Solving (delegates to Solver, see solver.hpp/.cpp) -----------------

void Cube::initSolver() { Solver::init(); }

std::optional<std::string> Cube::solveUpright(int maxDepth) const {
    return Solver::solveUpright(*this, maxDepth);
}

std::optional<std::string> Cube::solve(int maxDepth) const {
    return Solver::solve(*this, maxDepth);
}

std::string Cube::scramble() { return Solver::scramble(); }

}  // namespace cubejs
