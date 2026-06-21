# CubeDart

**CubeDart** is a Flutter FFI plugin that provides a Dart client library for Cube.js by wrapping the `cubecpp` C++ client. This package allows you to seamlessly integrate Cube.js analytics into your Flutter and Dart applications across multiple platforms using native bindings.

---

**CubeDart** é um plugin Flutter FFI que fornece uma biblioteca cliente Dart para Cube.js através de um wrapper do cliente C++ `cubecpp`. Este pacote permite integrar perfeitamente análises do Cube.js nas suas aplicações Flutter e Dart em múltiplas plataformas, utilizando bindings nativos.

## Features / Funcionalidades

- **Dart FFI Integration:** Direct and efficient communication with the native C++ library (`cubecpp`).
- **Multi-platform:** Supports Android, iOS, macOS, Windows, and Linux.
- **Cube.js API:** Provides the tools needed to construct and execute queries against a Cube.js backend.

## Project Structure / Estrutura do Projeto

* `src`: Contains the native C++ wrapper (`cubedart.cpp`, `cubedart.h`) and `cubecpp` submodule, plus a `CMakeLists.txt` for building into a dynamic library.
* `lib`: Contains the Dart code that defines the API of the plugin, using `dart:ffi` to call into the native code.
* Platform folders (`android`, `ios`, `windows`, etc.): Build files for bundling the native library with platform applications.

## Getting Started / Como Começar

### Prerequisites / Pré-requisitos

To use or build this plugin, you'll need the Flutter SDK and the native build tools for your target platforms (e.g., CMake for Windows/Linux, Xcode for iOS/macOS, Android NDK for Android).

Para usar ou compilar este plugin, precisará do SDK do Flutter e das ferramentas de compilação nativa para as suas plataformas alvo (ex: CMake para Windows/Linux, Xcode para iOS/macOS, Android NDK para Android).

### Installation / Instalação

Add `cubedart` to your Flutter project's `pubspec.yaml`:

```yaml
dependencies:
  cubedart:
    path: /path/to/cubedart
```

### Generating FFI Bindings / Gerando Bindings FFI

If you modify the C++ header (`src/cubedart.h`), you need to regenerate the Dart bindings:

```bash
dart run ffigen --config ffigen.yaml
```

## Usage / Utilização

Import the library in your Dart code and call the available FFI functions.

```dart
import 'package:cubedart/cubedart.dart';

// Example usage of cubedart
```

## Contributing / Contribuição

Contributions are welcome! Please make sure to update tests and bindings as appropriate.
Contribuições são bem-vindas! Certifique-se de atualizar os testes e os bindings conforme necessário.
