import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:registration_app/calendar_screen.dart';
import 'package:registration_app/model/user.dart';
import 'package:registration_app/profile_screen.dart';
import 'package:registration_app/today_screen.dart';

class Homescreen extends StatefulWidget {
  const Homescreen({super.key});

  @override
  State<Homescreen> createState() => _HomescreenState();
}

class _HomescreenState extends State<Homescreen> {
  double screenHeight = 0;
  double screenWidth = 0;

  String id = '';

  Color primary = const Color(0xFFEEF444C);

  int currentIndex =1;

  List<IconData> navigationIcons = [
    FontAwesomeIcons.calendarAlt,
    FontAwesomeIcons.check,
    FontAwesomeIcons.user
  ];

  @override
  void initState() {
    super.initState();

    getId();
  }

  void getId() async {
    QuerySnapshot snap = await FirebaseFirestore.instance.collection("Students").where('Student Number', isEqualTo: User.studentNumber).get();

    setState(() {
      User.id =snap.docs[0].id;
    });
  }


  @override
  Widget build(BuildContext context) {
    screenHeight = MediaQuery.of(context).size.height;
    screenWidth = MediaQuery.of(context).size.width;

    return Scaffold(
      body: IndexedStack(
        index: currentIndex,
        children: const [
          CalendarScreen(),
          TodayScreen(),
          ProfileScreen(),
          ], 
      ),
      bottomNavigationBar: Container(
        height: 70,
        margin: const EdgeInsets.only(
          left: 12,
          right: 12,
          bottom: 24,
        ),
        decoration: const BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.all(Radius.circular(40)),
          boxShadow: [
            BoxShadow(
              color: Colors.black26,
              blurRadius: 19,
              offset: Offset(2, 2),
            )
          ]
        ),
        child: ClipRRect(
          borderRadius: const BorderRadius.all(Radius.circular(40)),
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              for(int i = 0; i < navigationIcons.length; i++)...<Expanded>{
                Expanded(
                  child: GestureDetector(
                    onTap: () {
                      setState(() {
                        currentIndex = i;
                      });
                    },
                    child: Container(
                      height: screenHeight,
                      width: screenWidth,
                      color: Colors.white,
                      child: Center(
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            Icon(
                              navigationIcons[i],
                              color: i == currentIndex ? primary : Colors.black54,
                              size: i == currentIndex ? 30: 26,
                            ),
                            i == currentIndex ? Container(
                              margin: const EdgeInsets.only(top: 6),
                              height: 3,
                              width: 22,
                              decoration: BoxDecoration(
                                borderRadius: const BorderRadius.all(Radius.circular(40)),
                                color: primary,
                              ),
                            ) : const SizedBox(),
                          ],
                        ),
                      ),
                    ),
                  ),
                ),
              }
            ],
          ),
        ),
      ),
    );
  }
}