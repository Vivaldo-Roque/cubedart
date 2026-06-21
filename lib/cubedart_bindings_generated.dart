// AUTO GENERATED FILE, DO NOT EDIT.
// (Manually implemented due to ffigen missing libclang)
// ignore_for_file: non_constant_identifier_names
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart' as pkg_ffi;

class CubedartBindings {
  final ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
  _lookup;

  CubedartBindings(ffi.DynamicLibrary dynamicLibrary)
    : _lookup = dynamicLibrary.lookup;

  CubedartBindings.fromLookup(
    ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName) lookup,
  ) : _lookup = lookup;

  void cubedart_init_solver() {
    return _cubedart_init_solver();
  }

  late final _cubedart_init_solverPtr =
      _lookup<ffi.NativeFunction<ffi.Void Function()>>('cubedart_init_solver');
  late final _cubedart_init_solver = _cubedart_init_solverPtr
      .asFunction<void Function()>();

  int cubedart_is_solved(ffi.Pointer<pkg_ffi.Utf8> facelets) {
    return _cubedart_is_solved(facelets);
  }

  late final _cubedart_is_solvedPtr =
      _lookup<
        ffi.NativeFunction<ffi.Int32 Function(ffi.Pointer<pkg_ffi.Utf8>)>
      >('cubedart_is_solved');
  late final _cubedart_is_solved = _cubedart_is_solvedPtr
      .asFunction<int Function(ffi.Pointer<pkg_ffi.Utf8>)>();

  int cubedart_solve(
    ffi.Pointer<pkg_ffi.Utf8> facelets,
    int max_depth,
    ffi.Pointer<pkg_ffi.Utf8> out_solution,
    int out_max_len,
  ) {
    return _cubedart_solve(facelets, max_depth, out_solution, out_max_len);
  }

  late final _cubedart_solvePtr =
      _lookup<
        ffi.NativeFunction<
          ffi.Int32 Function(
            ffi.Pointer<pkg_ffi.Utf8>,
            ffi.Int32,
            ffi.Pointer<pkg_ffi.Utf8>,
            ffi.Int32,
          )
        >
      >('cubedart_solve');
  late final _cubedart_solve = _cubedart_solvePtr
      .asFunction<
        int Function(
          ffi.Pointer<pkg_ffi.Utf8>,
          int,
          ffi.Pointer<pkg_ffi.Utf8>,
          int,
        )
      >();

  int cubedart_solve_upright(
    ffi.Pointer<pkg_ffi.Utf8> facelets,
    int max_depth,
    ffi.Pointer<pkg_ffi.Utf8> out_solution,
    int out_max_len,
  ) {
    return _cubedart_solve_upright(
      facelets,
      max_depth,
      out_solution,
      out_max_len,
    );
  }

  late final _cubedart_solve_uprightPtr =
      _lookup<
        ffi.NativeFunction<
          ffi.Int32 Function(
            ffi.Pointer<pkg_ffi.Utf8>,
            ffi.Int32,
            ffi.Pointer<pkg_ffi.Utf8>,
            ffi.Int32,
          )
        >
      >('cubedart_solve_upright');
  late final _cubedart_solve_upright = _cubedart_solve_uprightPtr
      .asFunction<
        int Function(
          ffi.Pointer<pkg_ffi.Utf8>,
          int,
          ffi.Pointer<pkg_ffi.Utf8>,
          int,
        )
      >();

  int cubedart_obfuscate(
    ffi.Pointer<pkg_ffi.Utf8> alg,
    int num_premoves,
    int min_length,
    ffi.Pointer<pkg_ffi.Utf8> out_obfuscated,
    int out_max_len,
  ) {
    return _cubedart_obfuscate(
      alg,
      num_premoves,
      min_length,
      out_obfuscated,
      out_max_len,
    );
  }

  late final _cubedart_obfuscatePtr =
      _lookup<
        ffi.NativeFunction<
          ffi.Int32 Function(
            ffi.Pointer<pkg_ffi.Utf8>,
            ffi.Int32,
            ffi.Int32,
            ffi.Pointer<pkg_ffi.Utf8>,
            ffi.Int32,
          )
        >
      >('cubedart_obfuscate');
  late final _cubedart_obfuscate = _cubedart_obfuscatePtr
      .asFunction<
        int Function(
          ffi.Pointer<pkg_ffi.Utf8>,
          int,
          int,
          ffi.Pointer<pkg_ffi.Utf8>,
          int,
        )
      >();

  void cubedart_scramble(
    ffi.Pointer<pkg_ffi.Utf8> out_scramble,
    int out_max_len,
  ) {
    return _cubedart_scramble(out_scramble, out_max_len);
  }

  late final _cubedart_scramblePtr =
      _lookup<
        ffi.NativeFunction<
          ffi.Void Function(ffi.Pointer<pkg_ffi.Utf8>, ffi.Int32)
        >
      >('cubedart_scramble');
  late final _cubedart_scramble = _cubedart_scramblePtr
      .asFunction<void Function(ffi.Pointer<pkg_ffi.Utf8>, int)>();
}
