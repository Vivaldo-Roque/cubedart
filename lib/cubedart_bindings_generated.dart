// Bindings for `src/cubedart.h`.
//
// Resolved at runtime through Native Assets: the code asset is built by
// `hook/build.dart` with asset id
// `package:cubedart/cubedart_bindings_generated.dart`, which is the default
// asset for `@Native` declarations in this library.
//
// (Hand-written: ffigen requires libclang, which is not available on the
// development machine. Keep in sync with `src/cubedart.h`.)
// ignore_for_file: non_constant_identifier_names
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart' as pkg_ffi;

/// Initializes the solver. Must be called once before solve or solveUpright.
@ffi.Native<ffi.Void Function()>()
external void cubedart_init_solver();

/// Returns 1 if solved, 0 otherwise, -1 on error.
@ffi.Native<ffi.Int32 Function(ffi.Pointer<pkg_ffi.Utf8>)>()
external int cubedart_is_solved(ffi.Pointer<pkg_ffi.Utf8> facelets);

/// Returns 1 on success, 0 if no solution found, -1 on error.
@ffi.Native<
  ffi.Int32 Function(
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
  )
>()
external int cubedart_solve(
  ffi.Pointer<pkg_ffi.Utf8> facelets,
  int max_depth,
  ffi.Pointer<pkg_ffi.Utf8> out_solution,
  int out_max_len,
);

/// Solves assuming the cube is upright (U-top, F-front).
/// Returns 1 on success, 0 if no solution found, -1 on error.
@ffi.Native<
  ffi.Int32 Function(
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
  )
>()
external int cubedart_solve_upright(
  ffi.Pointer<pkg_ffi.Utf8> facelets,
  int max_depth,
  ffi.Pointer<pkg_ffi.Utf8> out_solution,
  int out_max_len,
);

/// Returns 1 on success, -1 on error.
@ffi.Native<
  ffi.Int32 Function(
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
    ffi.Int32,
    ffi.Int32,
    ffi.Int32,
    ffi.Int32,
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
  )
>()
external int cubedart_obfuscate(
  ffi.Pointer<pkg_ffi.Utf8> alg,
  int num_premoves,
  int min_length,
  int max_length,
  int absolute_max_length,
  int max_depth,
  ffi.Pointer<pkg_ffi.Utf8> out_obfuscated,
  int out_max_len,
);

/// Obfuscates using the optimal solver. Returns 1 on success, -1 on error.
@ffi.Native<
  ffi.Int32 Function(
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
    ffi.Int32,
    ffi.Int32,
    ffi.Int32,
    ffi.Int32,
    ffi.Pointer<pkg_ffi.Utf8>,
    ffi.Int32,
  )
>()
external int cubedart_obfuscate_optimal(
  ffi.Pointer<pkg_ffi.Utf8> alg,
  int num_premoves,
  int min_length,
  int max_length,
  int absolute_max_length,
  int max_depth,
  ffi.Pointer<pkg_ffi.Utf8> out_obfuscated,
  int out_max_len,
);

/// Generates a random scramble into [out_scramble].
@ffi.Native<ffi.Void Function(ffi.Pointer<pkg_ffi.Utf8>, ffi.Int32)>()
external void cubedart_scramble(
  ffi.Pointer<pkg_ffi.Utf8> out_scramble,
  int out_max_len,
);
