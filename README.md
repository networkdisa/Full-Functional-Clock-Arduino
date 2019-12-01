# Arduino Full Clock
### **Sketch** 
![Arduino Full Clock](https://raw.githubusercontent.com/networkdisa/Full-Functional-Clock-Arduino/master/fritzing%20sketch_bb.jpg)

### **Component list for build test clock** 
<ol>
  <li>Arduino Uno compatible board.</li>
  <li>WS2811B color LED or Pixel LED NOS 86 (you can adjust this quantity according to your plan and modification of code).</li>
  <li>Jump wires.</li>
  <li>RTC (Real-Time Clock) module.</li>
  <li>3 LEDs(Different color), Here used White,Green,Blue.</li>
  <li>MP3-TF-16 pin module.</li>
  <li>Memory Chip (at least 1GB).</li>
  <li>330 Ohm 3 resisters.</li>
  <li>1Kohm 6 resistors.</li>
  <li>5 push buttons.</li>
  <li>3ohm Speaker.</li>
</ol>

### **Functionality** 
  <ul>
    <li>Time can set manually through the push buttons.</li>
    <li>Date can set manually through the push buttons.</li>
    <li>Alarm can set manually through the push buttons.</li>
    <li>Display LED color change automatically</li>
    <li>Time can display 24hour format or 12 hour AM/PM format</li>
    <li>Every hour time tells in clear sound</li>
  </ul>

### **Main Code Modification** 
Here code modification done relevent to [12-11-2019_FritzingCompatiable_fullClock.ino](https://github.com/networkdisa/Full-Functional-Clock-Arduino/blob/master/12-11-2019_FritzingCompatiable_fullClock.ino "compatiable file")
Only main and customized code parts explained, all code well commented above linked file.

<ul>
    <li>Add required libraries </li>
</ul>

```C++
#include <RTClib.h>  //for RTC module
#include <Wire.h>  //L2C communication with RTC module and Arduino
#include <FastLED.h> //for WS2811
#include "SoftwareSerial.h" //for cummunicate with MP3 module
```
<ul>
    <li>WS2811 related values</li>
</ul>
(3*7)*4 + 2 = 86</br>
3 - number of LEDs in seven segment one segment.</br>
7 - above segments seven need to complete seven segment.</br>
4 - digits four (two for hour/month, two for minute/date),</br>
2 - seconds indicator.

```C++
#define NUM_LEDS 86 //(3*7)*4 + 2 = 86
#define DATA_PIN 7 //data pin for pixel led(OUTPUT)
CRGB leds[NUM_LEDS]; // Define Pixel LEDs
```
<ul>
    <li>Initial date time set (this can be done trough push buttons too)</li>
</ul>
Need to run in void setup code block and after first run this code need be commented.

```C++
//RTC.adjust(DateTime(2019, 10, 19, 00, 59, 0)); //YYYY,MM,DD,HH,MIN,Sec ,set RTC time first time(method-1)
//RTC.adjust(DateTime(__DATE__, __TIME__));  //set RTC time first time,take system time(method-2)
```
<ul>
    <li>Define seven segment in two dimentiional array</li>
</ul>
this array need to be modified according to number of LEDs you use in seven segment.

```C++
/*******LED Array******/
byte digits[11][21] = { //2D array for numbers on seven segments
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  //Digit 0
    {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},  //Digit 1
    {1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0},  //Digit 2
    {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1},  //Digit 3
    {1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1},  //Digit 4
    {1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1},  //Digit 5
    {1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  //Digit 6
    {0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},  //Digit 7
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  //Digit 8
    {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1},  //Digit 9
    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0}   //Celsius character(C)
};
```
<ul>
    <li>Add colors you wish to change while date/time/tempeture display in LEDs</li>
</ul>
I changed color rotation execute every 35 seconds while changing time to date and tempeture.

```C++
/*******LED Color Array******/
int colorRGB[12][3] = {
    {255,0,255},//Green
    {255,0,0},//Red
    {0,0,255},//Blue                  
    {255,255,0},//Yellow                  
    {255,0,128},//Spring Green           
    {0,255,128},//Rose                  
    {255,128,0},//Azure                    
    {128,0,255},//Chartryuse               
    {0,128,255},//Orange
    {255,0,0},//Cyan             
    {0,255,0},//Magenta                  
    {128,255,0}//Violet                  
}; 
```
<ul>
    <li>show time in LEDs 24 hour format</li>
</ul>
Here "ledDisplayGenerator(nowTime, 0, 0);" function sends LEDs to dispaly array parameters.

```C++
void timeGetNow(){
    byte Hour = dateTimeData[0];    //store date/time/seconds to the veriables.
    byte Minute = dateTimeData[1];
    byte Second = dateTimeData[2];
    if(Second % 2 == 0){  //seconds inditor (2 leds) high 
        leds[42] = CRGB(0,0,0);
        leds[43] = CRGB(0,0,0);
    }else{   //seconds inditor (2 leds) low  
        leds[42] = CRGB(red,green,blue);
        leds[43] = CRGB(red,green,blue);
    }
    int nowTime = Hour * 100 + Minute;  //eg-: hour-11, minite-25 (11*100+25=1125) take Hour & Minute in a single value.
    ledDisplayGenerator(nowTime, 0, 0);  //send taken single value for generate LEDs display array. 
}
```
<ul>
    <li>show time in LEDs 12 hour AM/PM format.</li>
</ul>

```C++
int timeGetNow(){
    byte Hour = dateTimeData[0];    //store date/time/seconds to the veriables.
    byte Minute = dateTimeData[1];
    byte Second = dateTimeData[2];
    if(Second % 2 == 0){  //seconds inditor (2 leds) high 
        leds[42] = CRGB(0,0,0);
        leds[43] = CRGB(0,0,0);
    }else{   //seconds inditor (2 leds) low  
        leds[42] = CRGB(red,green,blue);
        leds[43] = CRGB(red,green,blue);
    }
    if(Hour > 12){
        if(Hour > 19){
            Hour = Hour % 10 + 8; 
        }else{
           Hour = Hour %10 - 2; 
        }
    }
    if(Hour == 0){
        Hour = 12;
    }
    int nowTime = Hour * 100 + Minute;  //eg-: hour-11, minite-25 (11*100+25=1125)
    ledDisplayGenerator(nowTime, 0, 0);   
}
```
<ul>
    <li>Most importent function, send taken parameters converting array values to LEDs</li>
</ul>

```C++
void ledDisplayGenerator(int Now, byte firstCuple, byte secndCuple){
    int nowVal = Now;
    for(byte i = 0; i < 4; i++){  //4-four digits (two for hour/month & two for minute/date).
        if(Now == 0 && i == 0){  //if Now parameter empty led shows edit mode minute/date (3 and 4 digit)
            nowVal = secndCuple;
        }
        if(Now == 0 && i == 2){  //if Now parameter empty led shows edit mode Hour/month (1 and 2 digit)
            nowVal = firstCuple;
        }
        byte digit = nowVal % 10;   //only filter each last digits in queue (Eg: 1245 - take 5)
        //Serial.print("Digit Value--");
        //Serial.println(digit);
        if(i == 0){  //4th digit lightup.
            if(firstCuple == 99){ //99 for identyfy purpose only. when I passed tempeture to "Now" parameter I sent "firstCuple" parameter value as 99
                digit = 10; //since "C" letter represent in digits[11][21] array 10th index.
            }
            byte digitStart = 65;  //4th digit starting point
            //Serial.print("__Digit 4--");
            for(byte k = 0; k < 21; k++){  //k is one digit include nos LEDs in one seven segment.(21=3*7)
                //Serial.print(digits[digit][k]);    
                if(digits[digit][k] == 1){  //
                    leds[digitStart] = CRGB(red,green,blue); //color of LED lighted
                }else{
                    leds[digitStart] = CRGB(0,0,0);  //off colored LED(black)
                }
                digitStart++; 
            } //Serial.println();
        }else if(i == 1){  //3rd digit lightup
            byte digitStart = 44;  //3rd digit starting point
            //Serial.print("__Digit 3--");
            for(byte k = 0; k < 21; k++){  
                //Serial.print(digits[digit][k]);      
                if(digits[digit][k] == 1){
                    leds[digitStart] = CRGB(red,green,blue);
                }else{
                    leds[digitStart] = CRGB(0,0,0);
                }
                digitStart++; 
            }  //Serial.println();
        }else if(i == 2){  //2nd digit lightup
            byte digitStart = 21;  //2nd digit starting point
            //Serial.print("__Digit 2--"); 
            for(byte k = 0; k < 21; k++){ 
                //Serial.print(digits[digit][k]);      
                if(digits[digit][k] == 1){
                    leds[digitStart] = CRGB(red,green,blue);
                }else{
                    leds[digitStart] = CRGB(0,0,0);
                }
                digitStart++; 
            } //Serial.println();
        }else if(i == 3){  //1st digit lightup
            byte digitStart = 0;  //1st digit starting point
            //Serial.print("__Digit 1--");
            for(byte k = 0; k < 21; k++){ 
                //Serial.print(digits[digit][k]);      
                if(digits[digit][k] == 0 || (digit == 0 && dateTimeData[0] != 0)){ //if 1st digit 0 it belongs all LEDs off (leading zero not showing)
                    leds[digitStart] = CRGB(0,0,0);                                      //but midnight 24 hour showing as 00
                    //Serial.print("OFF"); 
                }else{
                    leds[digitStart] = CRGB(red,green,blue); 
                }
                digitStart++;  
            } //Serial.println();
        }
        nowVal = (firstCuple == 99 && i == 0) ? Now:  nowVal/10; //4digit not taken value from tempeture.it showing "C" letter. thats why this filter nextval for 4th digit.
        //Serial.print("Next Now----");   Serial.print(nowVal);
        //Serial.println();
    }
}
```
<ul>
    <li>Usable MP3 module serial commands and method of command execute</li>
</ul>

```C++
/******MP3 module command executer*******/
/*
General DF Player mini command structure (only byte 3, 5 and 6 to be entered in the serial monitor):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Byte Function Value
==== ================ ====
(0) Start Byte 0x7E
(1) Version Info 0xFF (don't know why it's called Version Info)
(2) Number of bytes 0x06 (Always 6 bytes)
(3) Command 0x__
(4) Command feedback 0x__ If enabled returns info with command 0x41 [0x01: info, 0x00: no info]
(5) Parameter 1 [DH] 0x__
(6) Parameter 2 [DL] 0x__
(7) Checksum high 0x__ See explanation below. Is calculated in function: execute_CMD
(8) Checksum low 0x__ See explanation below. Is calculated in function: execute_CMD
(9) End command 0xEF

Checksum calculation.
~~~~~~~~~~~~~~~~~~~~
Checksum = -Sum(byte(1..6)) (2 bytes, notice minus sign!)
*/
void execute_CMD(byte CMD, byte Par1, byte Par2){
    // Calculate the checksum (2 bytes)
    word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    // Build the command line
    byte Command_line[10] = { 
                                Start_Byte, 
                                Version_Byte, 
                                Command_Length, 
                                CMD, 
                                Acknowledge,
                                Par1, 
                                Par2, 
                                highByte(checksum), 
                                lowByte(checksum), 
                                End_Byte
                             };
    //Send the command line to the module
    for (byte k=0; k<10; k++){
        mySerial.write( Command_line[k]);
    }
}
```


