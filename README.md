18549 Capstone Project - 2048 + Hand Motions
=====

This project consists of 3 major parts:

JavaScript
1. 2048 Javascript game (source code from GitHub)
   * Modified to move tiles in response to hand motion input

C++
2. Custom hand tracker - created from OpenNI's sample MWClosestPoint app
   * Looking at the closest point within a window and tracking the direction of its motion
   * Event-triggered - asynchronously sending direction upon hand motion to proxy

JavaScript (NodeJS)
3. Proxy 
   * TCP client listenting to incoming input from the C++ server (regarding hand motion direction)
   * HTTP server publishing latest direction upon being contacted by front-end AJAX requests

Ultimately, the user can control the 2048 board using 4 hand motions: up, down, left, right, just as he would with the keyboard.
