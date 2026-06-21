import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

import 'cubedart_bindings_generated.dart';

const String _libName = 'cubedart';

/// The dynamic library in which the symbols for [CubedartBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final CubedartBindings _bindings = CubedartBindings(_dylib);

class CubeDart {
  /// Initializes the solver tables. Must be called once before solving.
  static void initSolver() {
    _bindings.cubedart_init_solver();
  }

  /// Checks if a facelet string represents a solved cube.
  static bool isSolved(String facelets) {
    final faceletsPtr = facelets.toNativeUtf8();
    try {
      final result = _bindings.cubedart_is_solved(faceletsPtr);
      if (result == -1) throw Exception('Invalid facelets string');
      return result == 1;
    } finally {
      calloc.free(faceletsPtr);
    }
  }

  /// Solves the cube and returns the algorithm, or null if no solution.
  static String? solve(String facelets, {int maxDepth = 22}) {
    final faceletsPtr = facelets.toNativeUtf8();
    final outPtr = calloc<Uint8>(1024);
    try {
      final result = _bindings.cubedart_solve(faceletsPtr, maxDepth, outPtr.cast(), 1024);
      if (result == -1) throw Exception('Failed to solve or invalid facelets');
      if (result == 0) return null;
      return outPtr.cast<Utf8>().toDartString();
    } finally {
      calloc.free(faceletsPtr);
      calloc.free(outPtr);
    }
  }

  /// Solves the cube upright (U-top, F-front) and returns the algorithm.
  static String? solveUpright(String facelets, {int maxDepth = 22}) {
    final faceletsPtr = facelets.toNativeUtf8();
    final outPtr = calloc<Uint8>(1024);
    try {
      final result = _bindings.cubedart_solve_upright(faceletsPtr, maxDepth, outPtr.cast(), 1024);
      if (result == -1) throw Exception('Failed to solve upright or invalid facelets');
      if (result == 0) return null;
      return outPtr.cast<Utf8>().toDartString();
    } finally {
      calloc.free(faceletsPtr);
      calloc.free(outPtr);
    }
  }

  /// Obfuscates an algorithm.
  static String obfuscate(String alg, {int numPremoves = 3, int minLength = 16}) {
    final algPtr = alg.toNativeUtf8();
    final outPtr = calloc<Uint8>(1024);
    try {
      final result = _bindings.cubedart_obfuscate(algPtr, numPremoves, minLength, outPtr.cast(), 1024);
      if (result == -1) throw Exception('Failed to obfuscate algorithm');
      return outPtr.cast<Utf8>().toDartString();
    } finally {
      calloc.free(algPtr);
      calloc.free(outPtr);
    }
  }

  /// Generates a random scramble algorithm.
  static String scramble() {
    final outPtr = calloc<Uint8>(1024);
    try {
      _bindings.cubedart_scramble(outPtr.cast(), 1024);
      return outPtr.cast<Utf8>().toDartString();
    } finally {
      calloc.free(outPtr);
    }
  }
}
