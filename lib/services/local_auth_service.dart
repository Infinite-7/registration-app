
import 'package:flutter/material.dart';
import 'package:local_auth/local_auth.dart';

class LocalAuthService {
  static final _auth = LocalAuthentication();

  static Future<bool> _canAuthenticate() async =>
    await _auth.canCheckBiometrics || await _auth.isDeviceSupported();

  static Future<bool> authenticate() async{
    try {
      if (!await _canAuthenticate()) return false;

      return await _auth.authenticate(
        // authMessages: const [
        //   AndroidAuthMessages(
        //     signInTitle: 'Sign in',
        //     cancelButton: 'No thanks',
        //   ),
        //   IOSAuthMessages(
        //     cancelButton: 'No thanks',
        //   ),
        // ],
        localizedReason: 'Use Face Id to authenticate',
        options: const AuthenticationOptions(
          useErrorDialogs: true,
          stickyAuth: true,
        ),
      );
    } catch (e) {
      debugPrint('erroe $e');
      return false;
    }
  }
}

