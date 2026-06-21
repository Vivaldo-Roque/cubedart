// obfuscate.cpp -- see obfuscate.hpp for the rationale behind dropping
// the original's explicit wcaOrient() step.
#include "cubejs/obfuscate.hpp"
#include "cubejs/solver.hpp"

#include <cmath>
#include <random>
#include <stdexcept>

namespace cubejs {

namespace {

std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

int randint(int mn, int mx) {
    std::uniform_int_distribution<int> dist(mn, mx);
    return dist(rng());
}

}  // namespace

std::vector<int> getPremoves(int length) {
    std::vector<int> result;
    result.reserve(length);

    int previous = U;  // prevents the first move from being U or D
    for (int i = 0; i < length; i++) {
        int face;
        do {
            face = randint(0, 5);
        } while (face == previous || std::abs(face - previous) == 3);
        previous = face;
        int power = (randint(0, 1) == 0) ? 0 : 2;  // single or inverse, never double
        result.push_back(face * 3 + power);
    }
    return result;
}

std::string obfuscate(const std::string& algorithm, int numPremoves, int minLength,
                       int maxLength, int maxDepth) {
    if (maxLength < minLength) {
        throw std::invalid_argument(
            "maxLength (" + std::to_string(maxLength) +
            ") deve ser >= minLength (" + std::to_string(minLength) + ")");
    }

    if (numPremoves > 60) {
        throw std::runtime_error(
            "obfuscate: nao foi possivel atingir minLength mesmo com muitos premoves "
            "(verifique se minLength e' razoavel para o algoritmo dado)");
    }

    if (numPremoves < 0) {
        throw std::runtime_error(
            "obfuscate: nao foi possivel manter abaixo de maxLength "
            "(tente aumentar maxLength ou reduzir minLength)");
    }

    if (!Solver::isInitialized()) Cube::initSolver();

    std::vector<int> premoves = getPremoves(numPremoves);

    // Start with a solved cube and do (inverse of premoves + algorithm).
    Cube c;
    c.move(Cube::inverse(premoves));
    c.move(algorithm);  // throws std::invalid_argument if malformed

    // Bring centers back to standard orientation.
    std::string uprightAlg = c.upright();
    c.move(uprightAlg);
    std::string orient = Cube::inverse(uprightAlg);

    // Find the solution to the (now upright) cube state.
    auto solutionOpt = c.solveUpright(maxDepth);
    if (!solutionOpt) {
        return obfuscate(algorithm, numPremoves + 1, minLength, maxLength, maxDepth);
    }

    // premoves + inverse(solution) + orient, simplified.
    std::vector<int> combined = premoves;
    std::vector<int> invSolution = Cube::inverse(Cube::parseAlg(*solutionOpt));
    combined.insert(combined.end(), invSolution.begin(), invSolution.end());
    std::vector<int> orientMoves = Cube::parseAlg(orient);
    combined.insert(combined.end(), orientMoves.begin(), orientMoves.end());

    std::vector<int> simplified = Cube::simplify(combined);
    int resultLength = static_cast<int>(simplified.size());

    // Check constraints
    if (resultLength < minLength) {
        // Too short: try with one more premove
        return obfuscate(algorithm, numPremoves + 1, minLength, maxLength, maxDepth);
    }

    if (resultLength > maxLength) {
        // Too long: try with one fewer premove
        return obfuscate(algorithm, numPremoves - 1, minLength, maxLength, maxDepth);
    }

    // Just right!
    return Cube::algToString(simplified);
}

}  // namespace cubejs
