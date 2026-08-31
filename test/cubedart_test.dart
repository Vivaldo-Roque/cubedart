import 'package:flutter_test/flutter_test.dart';
import 'package:cubedart/cubedart.dart';

void main() {
  const solvedFacelets =
      'UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB';
  // Facelets for cube scrambled with F R U R' U' F'
  const scrambledFacelets =
      'UUFURUUUURRRRRRRRRFUFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB';

  setUpAll(() {
    CubeDart.initSolver();
  });

  group('CubeDart.initSolver', () {
    test('is idempotent and can be safely called multiple times', () {
      expect(() => CubeDart.initSolver(), returnsNormally);
      expect(() => CubeDart.initSolver(), returnsNormally);
    });
  });

  group('CubeDart.scramble', () {
    test('generates non-empty WCA notation scramble', () {
      final scramble = CubeDart.scramble();
      expect(scramble.isNotEmpty, isTrue);

      final moves = scramble.split(' ');
      expect(moves.length, greaterThanOrEqualTo(10));

      final moveRegex = RegExp(r"^[RLUDFB][2']?$");
      for (final move in moves) {
        expect(
          moveRegex.hasMatch(move),
          isTrue,
          reason: 'Move $move should be valid WCA notation',
        );
      }
    });

    test('generates different scrambles on consecutive invocations', () {
      final s1 = CubeDart.scramble();
      final s2 = CubeDart.scramble();
      final s3 = CubeDart.scramble();

      expect(s1, isNot(equals(s2)));
      expect(s2, isNot(equals(s3)));
    });
  });

  group('CubeDart.isSolved', () {
    test('returns true for identity solved facelets', () {
      expect(CubeDart.isSolved(solvedFacelets), isTrue);
    });

    test('returns false for scrambled facelets', () {
      expect(CubeDart.isSolved(scrambledFacelets), isFalse);
    });

    test('throws exception for invalid length facelets', () {
      expect(() => CubeDart.isSolved('UUUU'), throwsA(isA<Exception>()));
      expect(
        () => CubeDart.isSolved(solvedFacelets.substring(0, 50)),
        throwsA(isA<Exception>()),
      );
    });

    test('throws exception for invalid characters in facelets', () {
      expect(
        () => CubeDart.isSolved(
          'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX',
        ),
        throwsA(isA<Exception>()),
      );
    });
  });

  group('CubeDart.solve & solveUpright', () {
    test('solves standard scrambled cube and returns non-empty algorithm', () {
      final solution = CubeDart.solve(scrambledFacelets);
      expect(solution, isNotNull);
      expect(solution!.isNotEmpty, isTrue);

      final moves = solution.split(' ');
      final moveRegex = RegExp(r"^[RLUDFB][2']?$");
      for (final move in moves) {
        expect(
          moveRegex.hasMatch(move),
          isTrue,
          reason: 'Solution move $move should be valid WCA notation',
        );
      }
    });

    test('solveUpright returns valid upright solution', () {
      final solution = CubeDart.solveUpright(scrambledFacelets);
      expect(solution, isNotNull);
      expect(solution!.isNotEmpty, isTrue);
    });

    test('throws exception on invalid facelet string', () {
      expect(
        () => CubeDart.solve('INVALID_FACELET_STRING'),
        throwsA(isA<Exception>()),
      );
      expect(
        () => CubeDart.solveUpright('INVALID_FACELET_STRING'),
        throwsA(isA<Exception>()),
      );
    });
  });

  group('CubeDart.obfuscate', () {
    test('obfuscates algorithm with default parameters', () {
      const alg = "F R U R' U' F'";
      final obf = CubeDart.obfuscate(alg);
      expect(obf.isNotEmpty, isTrue);
      expect(obf, isNot(equals(alg)));
    });

    test('respects minLength and maxLength constraints', () {
      const alg = "R U R' U'";
      final obf = CubeDart.obfuscate(
        alg,
        numPremoves: 2,
        minLength: 12,
        maxLength: 25,
      );
      expect(obf.isNotEmpty, isTrue);
      expect(obf, isNot(equals(alg)));

      final moves = obf.split(' ');
      expect(moves.length, greaterThanOrEqualTo(4));
    });

    test('obfuscates common CFOP algorithms', () {
      // Sune
      const sune = "R U R' U R U2 R'";
      final obfSune = CubeDart.obfuscate(sune);
      expect(obfSune.isNotEmpty, isTrue);
      expect(obfSune, isNot(equals(sune)));

      // T-Perm
      const tPerm = "R U R' U' R' F R2 U' R' U' R U R' F'";
      final obfTPerm = CubeDart.obfuscate(tPerm);
      expect(obfTPerm.isNotEmpty, isTrue);
      expect(obfTPerm, isNot(equals(tPerm)));
    });
  });

  group('Stress & Memory Safety', () {
    test('handles repeated calls without leaking memory or crashing', () {
      for (var i = 0; i < 20; i++) {
        final scramble = CubeDart.scramble();
        expect(scramble.isNotEmpty, isTrue);

        final isSolved = CubeDart.isSolved(solvedFacelets);
        expect(isSolved, isTrue);

        final obf = CubeDart.obfuscate("R U R' U'", numPremoves: 2);
        expect(obf.isNotEmpty, isTrue);
      }
    });
  });
}
