/*
 *  RAM Tester for 4116, 4164 and 4532 Chips
 *  Ian Cudlip 2021 grandoldian at gmail.com
 *  
 *  Based on 4164 test code by Noel Llopis 2021
 *  https://github.com/llopis/dram-tester
 *  Based on initial code by Chris Osborn <fozztexx@fozztexx.com>
 *  http://insentricity.com/a.cl/252
 *  
 *  Code also from arduino.cc
 *  
 *  Tested on a Nano
 *  PIN spec changed to match Stephen Vickers' 4116 & 4164 DRAM Tester v1-1 board from https://pcbgeeks.com/ 
 *  because it's great.
 *  No longer uses PORTD as a result, so it's slower. It uses the buttons to initiate the tests.
 *  Please see the #define section for the wiring.
 *  If no buttons are connected on start it will initiate the 4164 test. Reset your Arduino to start again.
 *  The change of pins also frees up the serial port, so there's serial output at 9600 baud.
 *  The 4532 test is within the 4164 test.
 *  Stephen switches his lights and buttons to ground.
 *  
 *  A warning if you don't know, the 4116 also uses 12v and -5v. Be careful not to get things mixed up 
 *  if you go there.
 *  
 *  Now using the internal thermistor and 2 analog pins as a seed for the random test. 
 *  I don't have an Arduino without a thermistor to test on there.
 *  
 *  LEDs
 *  For 4116 and 4164 pass means passed, fail means failed. 
 *  For 4532 fail will light on a pass, but pass will flash 3 times for a TMS4532-XXNL3 and 4 times for 
 *  a TMS4532-XXNL4.
 *  
 *  If a chip fails, try it a again, might be a false fail.
 *  
 *  I don't have a TMS4532-XXNL4 out of a board or one that's socketed, so tested by inverting that pin 
 *  with a 74LS04 which seems to prove it.
 *  
 *  Would like to add:
 *    Quick initial tests
 *    OKI3732X Chips
 *    
 *  Noel licensed it with the MIT License, so here's that. I concur.
 *  
 *  MIT License
 *
 *  Copyright (c) 2021 Ian Cudlip
 *  Copyright (c) 2021 Noel Llopis
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *    
 */

#define VERSION         "v0.3"
// 24/10/2021

#define DIN             A1
#define DOUT            8
#define CAS             9
#define RAS             A3
#define WE              A2

#define STATUS    A0
#define FAILED    11
#define SUCCESS   12

#define RANDOMPIN A1

#define BUT4116   10
#define BUT4164   13

#define ROW_START 0
#define COL_START 0

#define ROW_END 255
#define COL_END 255
#define ROW_HALF 127

#define OD0 A4
#define OD1 2
#define OD2 A5
#define OD3 6
#define OD4 5
#define OD5 4
#define OD6 7
#define OD7 3

#define NHIGH     LOW
#define NLOW      HIGH

int rowstart=ROW_START;
int rowend=ROW_END;
int rowhalf=ROW_HALF;
int colstart=COL_START;
int colend=COL_END;

int c4532=0;
int rAbad=0;
int rBbad=0;

int ready=0;
int flash=0;
int fc=0;

void setup()
{
  Serial.begin(9600); 

  pinMode(BUT4116, INPUT);
  pinMode(BUT4164, INPUT);
 
  pinMode(DIN, OUTPUT);
  pinMode(DOUT, INPUT);

  pinMode(CAS, OUTPUT);
  pinMode(RAS, OUTPUT);
  pinMode(WE, OUTPUT);

  pinMode(OD0, OUTPUT);
  pinMode(OD1, OUTPUT);
  pinMode(OD2, OUTPUT);
  pinMode(OD3, OUTPUT);
  pinMode(OD4, OUTPUT);
  pinMode(OD5, OUTPUT);
  pinMode(OD6, OUTPUT);
  pinMode(OD7, OUTPUT);

  DDRD = 0xff;
  
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(WE, HIGH);

  pinMode(STATUS, OUTPUT);
  pinMode(FAILED, OUTPUT);
  pinMode(SUCCESS, OUTPUT);

  digitalWrite(STATUS, HIGH);
  digitalWrite(FAILED, NLOW);
  digitalWrite(SUCCESS, NLOW);
}

void init4116()
{
  rowstart=0;
  rowend=127;
  colstart=0;
  colend=127;
  c4532=0;
}

void init4164()
{
  rowstart=0;
  rowend=255;
  rowhalf=127;
  colstart=0;
  colend=255;
  c4532=1;
  rAbad=0;
  rBbad=0;
}

void runTest()
{
  int passed=0;
  lightsOut();
  
  if (checkRandom()) 
   if (checkSame(0))
    if (checkSame(1))
      if (checkAlternating(0))
       if (checkAlternating(1)) {
         digitalWrite(STATUS, LOW);
         digitalWrite(SUCCESS, NHIGH);

         if (rAbad || rBbad) 
           digitalWrite(FAILED, NHIGH);
         else
           digitalWrite(FAILED, NLOW);

         Serial.println("Passed");
         flash=0;
         if (rAbad) {
           Serial.println("Chip passes as a TMS4532-XXNL4");
           flash=4;
         }
         if (rBbad) {     
           Serial.println("Chip passes as a TMS4532-XXNL3");
           flash=3;
         }
         passed=1;
       }

  if (!passed) {
         digitalWrite(STATUS, LOW);
         digitalWrite(SUCCESS, NLOW);
         digitalWrite(FAILED, NHIGH);
  }
}

void loop()
{
  Serial.print("RAM Tester 4116/4164/4532 ");
  Serial.println(VERSION);
 
  buttonLoop();
}

void setODOutput(int val)
{
  digitalWrite(OD0,(val & 1)>>0);
  digitalWrite(OD1,(val & 2)>>1);
  digitalWrite(OD2,(val & 4)>>2);
  digitalWrite(OD3,(val & 8)>>3);
  digitalWrite(OD4,(val & 16)>>4);
  digitalWrite(OD5,(val & 32)>>5);
  digitalWrite(OD6,(val & 64)>>6);
  digitalWrite(OD7,(val & 128)>>7);
}

static inline void writeToRowCol(int row, int col)
{
  setODOutput(row);
  digitalWrite(RAS, LOW);
  setODOutput(col);
  digitalWrite(CAS, LOW);

  digitalWrite(WE, LOW);
  
  digitalWrite(WE, HIGH);
 
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
}


static inline int readFromRowCol(int row, int col)
{
  setODOutput(row);
  digitalWrite(RAS, LOW);
  setODOutput(col);
  digitalWrite(CAS, LOW);

  int val = digitalRead(DOUT);
   
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);

  return val;
}

void oldFail(int col,int row)
{
  Serial.print("Failed at (col,row) (");
  Serial.print(col,BIN);
  Serial.print(",");
  Serial.print(row,BIN);
  Serial.println(")");  
  digitalWrite(STATUS, LOW);
  digitalWrite(SUCCESS, NLOW);
  digitalWrite(FAILED, NHIGH);
}

int fail(int col,int row)
{
  if (!c4532) {
    Serial.print("Failed at (col,row) (");
    Serial.print(col,BIN);
    Serial.print(",");
    Serial.print(row,BIN);
    Serial.println(")");  
  
    digitalWrite(STATUS, LOW);
    digitalWrite(SUCCESS, NLOW);
    digitalWrite(FAILED, NHIGH);

    return 1;
  }

  // So, we must be trying to check a 4532

  if (row<=rowhalf) {
      // Row 'A'
      if (!rAbad) {
        Serial.print("Row A Bad at row ");
        Serial.println(row,DEC);
        rAbad=1;
      }
  }

  if (row>rowhalf) {
      // Row 'B'
      if (!rBbad) {
        Serial.print("Row B Bad at row ");
        Serial.println(row,DEC);
        rBbad=1;
      }
  }

  if (rAbad && rBbad) {
    Serial.println("Failed. Row A and B Bad.");
    return 1; 
  } else {
    return 0;
  }

}

void modBlink(unsigned int val)
{
  unsigned int rmod;

  if (val==0) {
    digitalWrite(FAILED, NHIGH);
    digitalWrite(SUCCESS, NHIGH);
  }
  if (val==50) {
    digitalWrite(FAILED, NLOW);
  }

  rmod=(val%500);

  if (c4532) {
    if (rAbad^rBbad) {
      if (rmod==100) {
       digitalWrite(FAILED, NHIGH); 
      }
      if (rmod==110) {
       digitalWrite(FAILED, NLOW); 
      }
    }
  }
  
  if (rmod==250) {
    digitalWrite(SUCCESS, NHIGH);
  }
  if (rmod==499) {
    digitalWrite(SUCCESS, NLOW);
  }
}

void lightsOut()
{
    digitalWrite(FAILED, NLOW);
    digitalWrite(SUCCESS, NLOW);
}

void releaseButton(int button) {
  int i;
  while ((i=digitalRead(button))==0) {
     delay(50);
  }
}

void buttonLoop() {
  int but4116=0;
  int but4164=0;
  int nobut=0;
  int die=0;

  while (1) {

    but4116=digitalRead(BUT4116);
    but4164=digitalRead(BUT4164);

    if (fc==0) {
      // We're at the start
      if ((!but4116) || (!but4164)) {
        // Go into no button mode
        nobut=1;
      }
    }

    if (!nobut) {

      // Buttons
      
      if (!ready) {
        Serial.println("Ready");
        ready=1;
      }
  
      if (!but4116) {
         Serial.println("Button 4116 Pressed");
         releaseButton(BUT4116);
         init4116();
         runTest();
         ready=0;
      } else {
         // Serial.println("Button 4116 Not Pressed");
      }
  
      if (!but4164) {
         Serial.println("Button 4164 Pressed");
         releaseButton(BUT4164);
         init4164();
         runTest();
         ready=0;
         fc=40;
      } else {
         //Serial.println("Button 4164 Not Pressed");
      }
      
    } else {

      // No buttons

      if (!die) {
        Serial.println("Running 4164/4532 Test");
        init4164();
        runTest();
        fc=40;      
      }

      // Loop forever
      die=1;
      
    }

    delay(100);

    // flasher    
    if (flash!=0) {
       if ((fc%60)<(flash*8))
          if ((fc%8)==0)
            digitalWrite(SUCCESS, NHIGH);
       if ((fc%4)==2)
         digitalWrite(SUCCESS, NLOW); 
    }   
    fc++;
    if (fc==32766) fc=1;

  }
}

int checkSame(int val)
{
  int f;
  unsigned int col,row;

  Serial.print("Check Same ");
  Serial.println(val,DEC);
  
  digitalWrite(DIN, val);
 
  for (col=colstart; col<=colend; col++) {
    for (row=rowstart; row<=rowend; row++) {
      modBlink((col*(rowend-rowstart))+row);
      writeToRowCol(row, col); 
    }
  }

  /* Reverse DIN in case DOUT is floating */
  digitalWrite(DIN, !val);

  for (col=colstart; col<=colend; col++) {
    for (row=rowstart; row<=rowend; row++) {
      modBlink((col*(rowend-rowstart))+row);
      f=readFromRowCol(row, col);
      if (f != val) {
        if (fail(col,row))
           return 0;
      }
    }
  }
    
  lightsOut();
  return 1;
}

int checkAlternating(int start)
{
  int i = start;
  unsigned int col,row;

  Serial.print("Check Alternating ");
  Serial.println(start,DEC);

  for (col=colstart; col<=colend; col++) 
  {
    for (row=rowstart; row<=rowend; row++) 
    {
      modBlink((col*(rowend-rowstart))+row);
      digitalWrite(DIN, i);
      i = !i;
      writeToRowCol(row, col);
    }
  }
  
  i = start;
  for (col=colstart; col<=colend; col++) 
  {
    for (row=rowstart; row<=rowend; row++) 
    { 
      modBlink((col*(rowend-rowstart))+row);
      if (readFromRowCol(row, col) != i) {
        if (fail(col,row))
           return 0;
      }
  
      i = !i;
    }
  }
  
  lightsOut();
  return 1;
}

// printDouble function from https://forum.arduino.cc/t/printing-a-double-variable/44327
void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

    Serial.print (int(val));  //prints the int part
    Serial.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
        frac = (val - int(val)) * precision;
    else
        frac = (int(val)- val ) * precision;
    Serial.println(frac,DEC) ;
} 

// GetTemp function from https://playground.arduino.cc/Main/InternalTemperatureSensor/
double GetTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}

int checkRandom()
{
  unsigned long seed;
  double temp;
  unsigned int col,row;

  temp=GetTemp();
  
  // It doesn't matter that this overflows
  seed=(round(temp-(float)round(temp)*(1000+fc))+1)*(analogRead(A6)+1)*(analogRead(A7)+1);
  Serial.print("Check Random with seed ");
  Serial.println(seed,DEC);
  
  randomSeed(seed);
  for (col=colstart; col<=colend; col++) 
  {
    for (row=rowstart; row<=rowend; row++) 
    {
      const int value = (int)random(2);    
      modBlink((col*(rowend-rowstart))+row);
      digitalWrite(DIN, value);
      writeToRowCol(row, col);
    }
  }

  // Set the same seed as for the write to know what value to get
  randomSeed(seed);
  for (col=colstart; col<=colend; col++) 
  {
    for (row=rowstart; row<=rowend; row++) 
    {
      const int value = (int)random(2);    
      modBlink((col*(rowend-rowstart))+row);
      if (readFromRowCol(row, col) != value) {
        if (fail(col,row))
           return 0;
      }
    }
  }
  
  lightsOut();
  return 1;
}
