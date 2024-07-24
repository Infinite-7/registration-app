import 'dart:async' show Future;
import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:registration_app/firebase_options.dart';
import 'package:registration_app/homescreen.dart';
import 'package:registration_app/login_screen.dart';
import 'package:flutter_keyboard_visibility/flutter_keyboard_visibility.dart';
import 'package:shared_preferences/shared_preferences.dart';

Future<void> main() async{
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Flutter Demo',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const KeyboardVisibilityProvider(
        child: AuthCheck(),
      ),
    );
  }
}


class AuthCheck extends StatefulWidget {
  const AuthCheck({super.key});

  @override
  State<AuthCheck> createState() => _AuthCheckState();
}

class _AuthCheckState extends State<AuthCheck> {
  bool userAvailable = false;
  late SharedPreferences sharedPreferences;
  @override
  void initState() {
    super.initState();

    _getCurrentUser();
  }

  Future<void> _getCurrentUser() async {
    sharedPreferences = await SharedPreferences.getInstance();

    try {
      if(sharedPreferences.getString('student_number') != null) {
        setState(() {
          userAvailable = true;
        });
      }
    } catch (e) {
      setState(() {
        userAvailable = true;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return userAvailable ? const Homescreen() : const LoginScreen();
  }
}
