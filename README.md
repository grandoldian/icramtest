RAM Tester for 4116, 4164 and 4532 Chips
Ian Cudlip 2021 grandoldian at gmail.com
 
Based on 4164 test code by Noel Llopis 2021
https://github.com/llopis/dram-tester
Based on initial code by Chris Osborn <fozztexx@fozztexx.com>
http://insentricity.com/a.cl/252
 
Code also from arduino.cc

Tested on a Nano
PIN spec changed to match Stephen Vickers' 4116 & 4164 DRAM Tester v1-1 board from https://pcbgeeks.com/ 
because it's great.
No longer uses PORTD as a result, so it's slower. It uses the buttons to initiate the tests.
Please see the #define section for the wiring.
If no buttons are connected on start it will initiate the 4164 test. Reset your Arduino to start again.
The change of pins also frees up the serial port, so there's serial output at 9600 baud.
The 4532 test is within the 4164 test.
Stephen switches his lights and buttons to ground.

A warning if you don't know, the 4116 also uses 12v and -5v. Be careful not to get things mixed up 
if you go there.

Now using the internal thermistor and 2 analog pins as a seed for the random test.
I don't have an Arduino without a thermistor to test on there.

LEDs
For 4116 and 4164 pass means passed, fail means failed. 
For 4532 fail will light on a pass, but pass will flash 3 times for a TMS4532-XXNL3 and 4 times for 
a TMS4532-XXNL4.
 
If a chip fails, try it a again, might be a false fail.

I don't have a TMS4532-XXNL4 out of a board or one that's socketed, so tested by inverting that pin 
with a 74LS04 which seems to prove it.

Would like to add:
  Quick initial test
  OKI3732X Chips
