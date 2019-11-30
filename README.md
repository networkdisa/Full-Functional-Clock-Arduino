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
    <li>Every hour time tell in clear sound</li>
  </ul>

### **Main Code Modification** 
Here code modification done relevent to [12-11-2019_FritzingCompatiable_fullClock.ino](https://github.com/networkdisa/Full-Functional-Clock-Arduino/blob/master/12-11-2019_FritzingCompatiable_fullClock.ino "compatiable file")

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
(3*7)*4 + 2 = 86
3 - number of LEDs in seven segment one segment.
7 - above segments seven need to complete seven segment.
4 - digits four (two for hour/month, two for minute/date),
2 - seconds indicator.

```C++
#define NUM_LEDS 86 //(3*7)*4 + 2 = 86
#define DATA_PIN 7 //data pin for pixel led(OUTPUT)
CRGB leds[NUM_LEDS]; // Define Pixel LEDs
```
<ul>
    <li>Initial date time set (this can be done trough push buttons too)</li>
</ul>
Need to run in void setup code block and after first run tis code need be commented.

```C++
//RTC.adjust(DateTime(2019, 10, 19, 00, 59, 0)); //YYYY,MM,DD,HH,MIN,Sec ,set RTC time first time(method-1)
//RTC.adjust(DateTime(__DATE__, __TIME__));  //set RTC time first time,take system time(method-2)
```
<ul>
    <li>Add required libraries </li>
</ul>

```C++
#include <RTClib.h>  //for RTC module
#include <Wire.h>  //L2C communication with RTC module and Arduino
#include <FastLED.h> //for WS2811
#include "SoftwareSerial.h" //for cummunicate with MP3 module
```

