import 'package:flutter/material.dart';
import 'package:cubedart/cubedart.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _scramble = 'Not generated yet';
  String _solution = '';
  bool _isSolved = false;
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _initSolver();
  }

  Future<void> _initSolver() async {
    // Initializing the solver might take a moment
    CubeDart.initSolver();
    if (mounted) {
      setState(() {
        _isLoading = false;
      });
    }
  }

  void _generateScramble() {
    setState(() {
      _scramble = CubeDart.scramble();
      _solution = '';
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('CubeDart Example')),
        body: Center(
          child: _isLoading
              ? const CircularProgressIndicator()
              : Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Text(
                      'Scramble Algorithm:',
                      style: TextStyle(fontWeight: FontWeight.bold),
                    ),
                    Padding(
                      padding: const EdgeInsets.all(8.0),
                      child: Text(_scramble, textAlign: TextAlign.center),
                    ),
                    ElevatedButton(
                      onPressed: _generateScramble,
                      child: const Text('Generate Scramble'),
                    ),
                    const SizedBox(height: 20),
                    if (_solution.isNotEmpty) ...[
                      const Text(
                        'Solution:',
                        style: TextStyle(fontWeight: FontWeight.bold),
                      ),
                      Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: Text(_solution, textAlign: TextAlign.center),
                      ),
                    ],
                  ],
                ),
        ),
      ),
    );
  }
}
