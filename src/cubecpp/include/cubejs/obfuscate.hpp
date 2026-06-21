// cubejs/obfuscate.hpp
//
// Port of a utility the user originally wrote in JavaScript on top of
// cube.js: given an algorithm, produces a different-looking sequence of
// moves with the exact same net effect on the cube -- useful for
// generating OLL/PLL-style practice scrambles that don't visually give
// away which case they set up (the same trick official WCA scramblers
// use).
//
// The original JS version carried its own hand-rolled cube engine
// (`RubiksCube`) just to apply moves and convert to/from facelet
// strings, only reaching for cube.js to call `.solve()`. This port
// drops that duplicate engine entirely and builds directly on
// cubejs::Cube, which already does everything it needs (applying all 18
// move types, facelet conversion, and orientation-agnostic solving).
//
// The one piece that *is* kept faithfully is the explicit orientation
// correction (the original's wcaOrient() step). It's tempting to assume
// Cube::solve()'s built-in orientation handling makes it redundant, but
// it doesn't: face turns -- and therefore premoves and every solve()
// output -- never touch the cube's centers (only rotations like x/y/z
// do, by construction of the move tables). So if `algorithm` itself has
// a net whole-cube rotation (e.g. it starts with "y"), the only way to
// reproduce that in the final disguised algorithm is to explicitly
// figure out what rotation is needed and append it. Dropping this step
// silently produces an algorithm with the right corners/edges but the
// wrong centers whenever the input algorithm rotates the cube -- caught
// by tests/selftest.cpp using an F-perm written starting with "y".


#pragma once

#include "cubejs/cube.hpp"

#include <string>
#include <vector>

namespace cubejs {

// Generates `length` random quarter turns (single or inverse only, never
// double) of U/R/F/D/L/B, such that no two consecutive moves share a
// face or sit on opposite faces. These are the "decoy" moves that get
// folded into obfuscate()'s re-solve so they don't show up in the final
// result, while still making the scramble structurally different from
// `algorithm` itself.
std::vector<int> getPremoves(int length);

// Returns an algorithm with the exact same net effect as `algorithm`
// (verified: applying either to a solved cube yields an identical
// resulting state), built by conjugating it with random premoves and
// re-solving, then simplifying. If the result comes out shorter than
// `minLength` moves, retries with one more premove. If it comes out
// longer than `maxLength`, retries with one fewer premove (or reduces
// minLength). This allows trading off between "long enough to disguise"
// and "short enough to fit on screen".
//
// Throws std::invalid_argument if `algorithm` isn't a valid alg string.
std::string obfuscate(const std::string& algorithm, int numPremoves = 3,
                       int minLength = 16, int maxLength = 999, int maxDepth = 22);

}  // namespace cubejs
