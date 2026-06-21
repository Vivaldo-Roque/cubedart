#include "cubedart.h"
#include "cubecpp/include/cubejs/cube.hpp"
#include "cubecpp/include/cubejs/solver.hpp"
#include "cubecpp/include/cubejs/obfuscate.hpp"

#include <cstring>
#include <exception>

using namespace cubejs;

extern "C" {

FFI_PLUGIN_EXPORT void cubedart_init_solver() {
    try {
        Cube::initSolver();
    } catch (...) {
        // Ignore initialization errors
    }
}

FFI_PLUGIN_EXPORT int32_t cubedart_is_solved(const char* facelets) {
    if (!facelets) return -1;
    try {
        Cube cube = Cube::fromString(facelets);
        return cube.isSolved() ? 1 : 0;
    } catch (...) {
        return -1;
    }
}

FFI_PLUGIN_EXPORT int32_t cubedart_solve(const char* facelets, int32_t max_depth, char* out_solution, int32_t out_max_len) {
    if (!facelets || !out_solution || out_max_len <= 0) return -1;
    try {
        Cube cube = Cube::fromString(facelets);
        auto sol = cube.solve(max_depth);
        if (sol) {
            strncpy(out_solution, sol->c_str(), out_max_len - 1);
            out_solution[out_max_len - 1] = '\0';
            return 1;
        }
        return 0; // No solution
    } catch (...) {
        return -1;
    }
}

FFI_PLUGIN_EXPORT int32_t cubedart_solve_upright(const char* facelets, int32_t max_depth, char* out_solution, int32_t out_max_len) {
    if (!facelets || !out_solution || out_max_len <= 0) return -1;
    try {
        Cube cube = Cube::fromString(facelets);
        auto sol = cube.solveUpright(max_depth);
        if (sol) {
            strncpy(out_solution, sol->c_str(), out_max_len - 1);
            out_solution[out_max_len - 1] = '\0';
            return 1;
        }
        return 0;
    } catch (...) {
        return -1;
    }
}

FFI_PLUGIN_EXPORT int32_t cubedart_obfuscate(const char* alg, int32_t num_premoves, int32_t min_length, char* out_obfuscated, int32_t out_max_len) {
    if (!alg || !out_obfuscated || out_max_len <= 0) return -1;
    try {
        std::string result = obfuscate(alg, num_premoves, min_length);
        strncpy(out_obfuscated, result.c_str(), out_max_len - 1);
        out_obfuscated[out_max_len - 1] = '\0';
        return 1;
    } catch (...) {
        return -1;
    }
}

FFI_PLUGIN_EXPORT void cubedart_scramble(char* out_scramble, int32_t out_max_len) {
    if (!out_scramble || out_max_len <= 0) return;
    try {
        std::string scr = Cube::scramble();
        strncpy(out_scramble, scr.c_str(), out_max_len - 1);
        out_scramble[out_max_len - 1] = '\0';
    } catch (...) {
        out_scramble[0] = '\0';
    }
}

} // extern "C"
