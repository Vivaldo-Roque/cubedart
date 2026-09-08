#include "../cubedart.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[FAIL] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            g_tests_failed++; \
        } else { \
            std::cout << "[PASS] " << msg << std::endl; \
            g_tests_passed++; \
        } \
    } while (0)

const char* SOLVED_FACELETS = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";
// Scramble: F R U R' U' F'
const char* SCRAMBLED_FACELETS = "UUFURUUUURRRRRRRRRFUFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

void test_init_solver() {
    std::cout << "\n--- Running test_init_solver ---" << std::endl;
    // Should be callable multiple times without crashing
    cubedart_init_solver();
    cubedart_init_solver();
    TEST_ASSERT(true, "cubedart_init_solver executed successfully");
}

void test_is_solved() {
    std::cout << "\n--- Running test_is_solved ---" << std::endl;
    // Solved state
    int res = cubedart_is_solved(SOLVED_FACELETS);
    TEST_ASSERT(res == 1, "is_solved returns 1 for solved state");

    // Scrambled state
    res = cubedart_is_solved(SCRAMBLED_FACELETS);
    TEST_ASSERT(res == 0, "is_solved returns 0 for scrambled state");

    // Null pointer
    res = cubedart_is_solved(nullptr);
    TEST_ASSERT(res == -1, "is_solved returns -1 for nullptr");

    // Invalid length
    res = cubedart_is_solved("UUUU");
    TEST_ASSERT(res == -1, "is_solved returns -1 for too short facelets");

    // Invalid characters (returns -1)
    res = cubedart_is_solved("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    TEST_ASSERT(res == -1, "is_solved returns -1 for invalid character string");
}

void test_solve() {
    std::cout << "\n--- Running test_solve ---" << std::endl;
    char solution[1024] = {0};

    // Scrambled cube
    int res = cubedart_solve(SCRAMBLED_FACELETS, 22, solution, sizeof(solution));
    TEST_ASSERT(res == 1, "solve returns 1 for scrambled cube");
    TEST_ASSERT(strlen(solution) > 0, "solve produces non-empty solution");
    std::cout << "  Solution: " << solution << std::endl;

    // Upright solve
    memset(solution, 0, sizeof(solution));
    res = cubedart_solve_upright(SCRAMBLED_FACELETS, 22, solution, sizeof(solution));
    TEST_ASSERT(res == 1, "solve_upright returns 1 for scrambled cube");
    TEST_ASSERT(strlen(solution) > 0, "solve_upright produces non-empty solution");

    // Null and edge cases
    res = cubedart_solve(nullptr, 22, solution, sizeof(solution));
    TEST_ASSERT(res == -1, "solve returns -1 for null facelets");

    res = cubedart_solve(SCRAMBLED_FACELETS, 22, nullptr, sizeof(solution));
    TEST_ASSERT(res == -1, "solve returns -1 for null output buffer");

    res = cubedart_solve(SCRAMBLED_FACELETS, 22, solution, 0);
    TEST_ASSERT(res == -1, "solve returns -1 for 0 max length");

    res = cubedart_solve("INVALID_FACELETS", 22, solution, sizeof(solution));
    TEST_ASSERT(res == -1, "solve returns -1 for invalid facelets");
}

void test_obfuscate() {
    std::cout << "\n--- Running test_obfuscate ---" << std::endl;
    char obf[1024] = {0};
    const char* alg = "F R U R' U' F'";

    int res = cubedart_obfuscate(alg, 3, 10, 20, 25, 22, obf, sizeof(obf));
    TEST_ASSERT(res == 1, "obfuscate returns 1 for valid algorithm");
    TEST_ASSERT(strlen(obf) > 0, "obfuscate produces non-empty output");
    TEST_ASSERT(strcmp(obf, alg) != 0, "obfuscate produces a different algorithm");
    std::cout << "  Original: " << alg << "\n  Obfuscated: " << obf << std::endl;

    // Null checks
    res = cubedart_obfuscate(nullptr, 3, 10, 20, 25, 22, obf, sizeof(obf));
    TEST_ASSERT(res == -1, "obfuscate returns -1 for null algorithm");

    res = cubedart_obfuscate(alg, 3, 10, 20, 25, 22, nullptr, sizeof(obf));
    TEST_ASSERT(res == -1, "obfuscate returns -1 for null output buffer");
}

void test_scramble() {
    std::cout << "\n--- Running test_scramble ---" << std::endl;
    char scramble_buf1[1024] = {0};
    char scramble_buf2[1024] = {0};

    cubedart_scramble(scramble_buf1, sizeof(scramble_buf1));
    cubedart_scramble(scramble_buf2, sizeof(scramble_buf2));

    TEST_ASSERT(strlen(scramble_buf1) > 0, "scramble 1 is not empty");
    TEST_ASSERT(strlen(scramble_buf2) > 0, "scramble 2 is not empty");
    TEST_ASSERT(strcmp(scramble_buf1, scramble_buf2) != 0, "consecutive scrambles are different");
    std::cout << "  Scramble 1: " << scramble_buf1 << std::endl;
    std::cout << "  Scramble 2: " << scramble_buf2 << std::endl;

    // Buffer edge cases (should not crash)
    cubedart_scramble(nullptr, 100);
    cubedart_scramble(scramble_buf1, 0);
    TEST_ASSERT(true, "scramble handles invalid buffer gracefully");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Running CubeDart Native C++ Tests     " << std::endl;
    std::cout << "========================================" << std::endl;

    test_init_solver();
    test_is_solved();
    test_solve();
    test_obfuscate();
    test_scramble();

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Summary: " << g_tests_passed << " passed, " << g_tests_failed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}
