// Build hook (Native Assets): compiles the CubeCPP solver for the current
// target OS/architecture. Replaces the per-platform CMake / Gradle NDK /
// CocoaPods setup previously used by the `ffiPlugin` configuration.
import 'package:code_assets/code_assets.dart';
import 'package:hooks/hooks.dart';
import 'package:native_toolchain_c/native_toolchain_c.dart';

void main(List<String> args) async {
  await build(args, (input, output) async {
    if (!input.config.buildCodeAssets) return;

    final targetOS = input.config.code.targetOS;

    final builder = CBuilder.library(
      name: 'cubedart',
      // Resolves to `package:cubedart/cubedart_bindings_generated.dart`, the
      // library holding the `@Native` declarations.
      assetName: 'cubedart_bindings_generated.dart',
      sources: const [
        'src/cubedart.cpp',
        'src/cubecpp/src/cube.cpp',
        'src/cubecpp/src/solver.cpp',
        'src/cubecpp/src/obfuscate.cpp',
      ],
      includes: const ['src', 'src/cubecpp/include'],
      language: Language.cpp,
      std: 'c++17',
      defines: const {'DART_SHARED_LIB': null},
      // MSVC needs explicit C++ exception handling semantics.
      flags: targetOS == OS.windows ? const ['/EHsc'] : const [],
      // Android 16 KB page alignment is applied by native_toolchain_c itself.
    );

    await builder.run(input: input, output: output);
  });
}
