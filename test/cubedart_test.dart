import 'package:flutter_test/flutter_test.dart';
import 'package:cubedart/cubedart.dart';

void main() {
  setUpAll(() {
    CubeDart.initSolver();
  });

  test('scramble generates string', () {
    final scramble = CubeDart.scramble();
    expect(scramble.isNotEmpty, true);
  });

  test('isSolved identifies solved cube', () {
    const solvedFacelets = 'UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB';
    expect(CubeDart.isSolved(solvedFacelets), true);
  });

  test('solve finds a solution', () {
    // F R U R' U' F'
    const scrambledFacelets = 'UUFURUUUURRRRRRRRRFUFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB';
    expect(CubeDart.isSolved(scrambledFacelets), false);
    
    final solution = CubeDart.solve(scrambledFacelets);
    expect(solution, isNotNull);
    expect(solution!.isNotEmpty, true);
  });
}
