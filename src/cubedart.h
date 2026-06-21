#ifndef CUBEDART_H
#define CUBEDART_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the solver. Must be called once before solve or solveUpright.
FFI_PLUGIN_EXPORT void cubedart_init_solver();

// Checks if a cube facelet string represents a solved cube.
// Returns 1 if solved, 0 otherwise, -1 on error.
FFI_PLUGIN_EXPORT int32_t cubedart_is_solved(const char* facelets);

// Solves a cube from a facelet string, ignoring its orientation.
// Returns 1 on success, 0 if no solution found, -1 on error.
// The resulting algorithm string is copied into out_solution.
FFI_PLUGIN_EXPORT int32_t cubedart_solve(const char* facelets, int32_t max_depth, char* out_solution, int32_t out_max_len);

// Solves a cube assuming it is upright (U-top, F-front).
// Returns 1 on success, 0 if no solution found, -1 on error.
FFI_PLUGIN_EXPORT int32_t cubedart_solve_upright(const char* facelets, int32_t max_depth, char* out_solution, int32_t out_max_len);

// Obfuscates an algorithm.
// Returns 1 on success, -1 on error.
FFI_PLUGIN_EXPORT int32_t cubedart_obfuscate(const char* alg, int32_t num_premoves, int32_t min_length, char* out_obfuscated, int32_t out_max_len);

// Generates a random scramble algorithm.
// The result is copied to out_scramble.
FFI_PLUGIN_EXPORT void cubedart_scramble(char* out_scramble, int32_t out_max_len);

#ifdef __cplusplus
}
#endif

#endif // CUBEDART_H
