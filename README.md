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

```C
#include <RTClib.h>
#include <Wire.h>
#include <FastLED.h>
#include "SoftwareSerial.h"
```
`$ npm install marked`
