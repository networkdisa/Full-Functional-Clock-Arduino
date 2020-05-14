
/*******************************************************************************************
Owner: NetworkDisa(Nuwan Dissanayake) http://networkdisa.blogspot.com
Last Modified: 20/09/2019
Phone: +94775522254

/***NOTE***
01: When set the alarm you have to set both alarm hour and minute. if you set only hour alarm wont be work.
**********
change log:
26-03-2020 - line 173
01-04-2020 - line 280, 339
/*******************************************************************************************/

#include <RTClib.h>
#include <Wire.h>
#include <FastLED.h>
#include "SoftwareSerial.h"
/***MP3 module default values***/
# define Start_Byte 0x7E  //Each command feedback begin with $ , that is0x7E 
# define Version_Byte 0xFF //FF --- Version Information
# define Command_Length 0x06 //the number of bytes after ?Len?, Data length is 6, which are 6 bytes [FF 06 09 00 00 04]. Not counting the start, end, and verification
# define End_Byte 0xEF //End bit 0xEF, indicate command ended
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info] If need for feedback, 1: feedback, 0: no feedback 
SoftwareSerial mp3Serial(12, 11); //define SoftwareSerial RX TX pin for mp3 module
SoftwareSerial btSerial(9,8); //define SoftwareSerial RX TX pin for Bluetooth module
 
RTC_DS3231 RTC; //create RTC object
/***WS2811B related values***/
#define NUM_LEDS 86 //(3*7)*4 + 2 = 86
#define DATA_PIN 10 //data pin for pixel led(OUTPUT)
CRGB leds[NUM_LEDS]; // Define Pixel LEDs
#define ldrInput A6 //for set Brightness og LEDs
int ldrValue = 200; //initial value of brightness

//char* btPass = "\"1234\""; //for bluetooth password

void setup() {
    Serial.begin(9600);
    mp3Serial.begin (9600); //begin SoftwareSerial communication with MP module
    btSerial.begin (9600); //begin SoftwareSerial communication with Bluetooth module
    Wire.begin(); // start the wire communication I2C
    RTC.begin();  // start the RTC-module
    //RTC.adjust(DateTime(2019, 10, 19, 00, 59, 0)); //YYYY,MM,DD,HH,MIN,Sec ,set RTC time first time(method-1)
    //RTC.adjust(DateTime(__DATE__, __TIME__));  //set RTC time first time,take system time(method-2)

    FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);  //Set Pixel LED type
    LEDS.setBrightness(ldrValue);  //set initial brighness max:255 : done by chkBrightness()
   
    execute_CMD(0x3F, 0, 0); // Send request for initialization parameters
    delay(500); //for mp3 module to work for the command sent
    execute_CMD(0x06, 0, 10); // Set the volume level(0x00~0x30)
    delay(500);

    //set bluetooth module name
    //btSerial.print("AT+NAME:"); 
    //btSerial.println("Disa");
    //delay(1000);
    
    //set bluetooth password
    //btSerial.print("AT+PSWD:");
    //btSerial.println(btPass);
    //delay(1000);
    
}

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
/*******LED Color Array******/
bool autoColor = true; //auto color change off or on(default on)
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
byte colorRGBIndex = 0; //ColorArray indexNo. increment every 5 seconds
byte red = 0;       //default color defined- Magenta
byte green = 255;
byte blue = 0; 

/***INPUT***/
#define btnModeInput A0  //9Mode button-A0
#define btnPlusInput A1   //8+Plus+ button-A1
#define btnMinusInput A2  //7-Minus- button-A2
#define btnSetInput 6  //6Set button-A6
#define btnAlrmOffInput 5  //5Alrm Off button-A7
/***OUTPUT***/
#define ledModDate 4  //Date edit mode indicate LED
#define ledModTime 3  //Time edit mode indicate LED       //this 3LEDs on Same time when going to Year edit mode.
#define ledModAlarm 2   //Alarm edit mode indicate LED
/***defined veriables***/
int dateTimeData[6]; //{[0]Hour, [1]Minute, [2]Seconds, [3]Month, [4]Date, [5]Year} srore real date/time values. 
int setData[] = {-1, -1, 0, 0, 0, 2017, -1, -1, 255, 255, 255, 0, 1}; //{[0]Hour, [1]Minute, [2]Seconds, [3]Month, [4]Date, [5]Year, elarmHour[6],elarmMinute[7], RED[8], GREEN[9], BLUE[10], AdjustBackLight[11],AdjustVolume[12]} store set values. first time values increment and decremt use this values.
int minMaxVal[5][2] = { //for take maxand min values.
    {0,24}, //for take 0 and <24 means 0-23 range(Hour/Min=[0][0]||Max=[0][1]) 0 is 24
    {0,60}, //for take 0 and <60 means 0-59 range(Minute/Min=[1][0]||Max=[1][1]) 0 is 60
    {1,13}, //for take 1 and <13 means 1-12 range(Month/Min=[2][0]||Max=[2][1])
    {1,32}, //for take 1 and <32 means 1-31 range(Date/Min=[3][0]||Max=[3][1])
    {2017,2050} //for year-limit year edit from 2017 to 2018. you can increment this as you wish.
};
//String symbols[] = {" : "," : ","-------"," / "," /",""};  //use for serial monitor time display
unsigned long millisForClock = 0;  //this millis()value increment every 1second and store date time values in array
unsigned long millisForClockDisplay = 0; //clock values represent in LEDs every 1 seconds.
unsigned long lastPressTimeBtnMode = millis(); //update everytime after press mode button. use for disable Mode button active time(reset)
int btnModVal = -1; //starting Mode button value  
bool editMode = false; //if this mode true loop not display time and date/tempeture
bool setPosition = true; //in edit mode this value thrue means, First two digit values(Hour/Month) consider. if false (Minute/Date) values consider.
int setIndex = 0; //this val use in setValToRTC() function for direct the particular case in Switch statement.
bool alarmSet = false; //this value true when alarm set.
bool alarmBeepUntillStop = false; //become true untill press the alarm stop button.
String hexVal; //store hexadecimal value for mp module tospecify track number
byte hexaCMD[] = {0x18, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17}; //mp3 track number for in hexadecimal
bool alarmPlayExecute = false;  //track alarmexecutestatus

String txtMsg; //serial buffer(from bluetooth)data collector
byte btSetDataIndex = 0; //index of setData[] store data send by bluetooth module
byte bluetoothSetupMode = 0;//for identify bluetooth setup mode
bool timeFormat = true; //time format 12/24 hour(true = 12 hour)
bool voiceTime = true; //telling time on /off- true means ON
bool autoBacklight = true; //backlight auto adjustment


void loop() {
    //get time date values from RTC object & store dateTimeData[] array
    if(millis() - millisForClock >= 1000){ 
        millisForClock = millis();  //update time intervalcalculated value.
        DateTime now = RTC.now();  //Get the real time/date info from RTC object

        dateTimeData[0] = now.hour();   //store all the info to dateTimeData[] array. 
        dateTimeData[1] = now.minute();
        dateTimeData[2] = now.second();
        dateTimeData[3] = now.month();
        dateTimeData[4] = now.day();
        dateTimeData[5] = now.year();
        /*****only for serial monitor display only*****
        for(byte i = 0; i < 6; i++){
            Serial.print(dateTimeData[i]); Serial.print(symbols[i]);  
        }
        Serial.print(" __AlarmData__"); Serial.print(setData[6]);Serial.print(":");Serial.print(setData[7]);
        Serial.print(" ----Edit Position="); Serial.print(btnModVal);
        Serial.println(); 
        ******************************************/chkBrightness();
    }

    if((millis() - millisForClockDisplay >= 1000) && !editMode){//here check edit mode too. if editMode=true time not displays.
        millisForClockDisplay = millis(); 
        if(autoBacklight){
            chkBrightness();
        }
        
        if(dateTimeData[2] == 35 && autoColor){ //color change time related 35 second. if "autoColor" bool flase, color not changed
            colorRGBIndex = (colorRGBIndex < 11) ? colorRGBIndex+1 : 0; //chk colorRGB[] array max index val exceeded or not. if exceeded reset to 0.
            red = colorRGB[colorRGBIndex][0];  //apply RGB values
            green = colorRGB[colorRGBIndex][1];
            blue = colorRGB[colorRGBIndex][2];
        }
        if(dateTimeData[2] >= 35 && dateTimeData[2] <= 38){ //every time related 35 second date display continiusely 5 seconds.
            dateGetNow(); //display date in LEDs 
            FastLED.show();   
        }else if(dateTimeData[2] >= 39 && dateTimeData[2] <= 41){ //every time related 41 second Tempeture display continiusely 5 seconds.
            getTempeture(); //display tempeture in LEDs.
            FastLED.show(); 
        }else{
            timeGetNow(); //display time in LEDs  
            FastLED.show(); 
        } 
    }

    //check time for play MP3 module stored voice
    if((dateTimeData[1] == 0 && dateTimeData[2] == 0) && (!alarmBeepUntillStop && voiceTime)){ //every 1 hour change voice track played relevant to it.
        execute_CMD(0x0F,1,hexaCMD[dateTimeData[0]]); //play 01 folder 00X (X = Hour) given track
        //Serial.println("Run Voice command");
        delay(3000);
    }

    //mode selection
    byte btnMode = digitalRead(btnModeInput);
    if(btnMode == HIGH){  //check Mode button pressed.
        //Serial.println("Button Pressed");
        lastPressTimeBtnMode = millis(); 
        btnModVal++;   //mode button value increment till 4.(0-timeSet(),1-dateSet(),2-alrmSet(),3-yearSet())
    }

    //bluetooth module serial communicathio grabber
    btSerialCmd();
    
    bluetoothSetup(bluetoothSetupMode); //check bluetooth edit status and take action

    //every time check below function activities.
    timeSet();
    dateSet();
    alrmSet();
    yearSet();
    checkAlarm();  

      //Serial.print(setData[6]); Serial.print(" : "); Serial.println(setData[7]);
                    
    
    
    
}//end of loop

/******chkBrightness()*******///Check the outside light level and set btightness og LEDsaccording to it.
void chkBrightness(){
  ldrValue = analogRead(ldrInput); 
  Serial.print("LRD Value = "); Serial.println(ldrValue);
  ldrValue = map(ldrValue, 0, 1024, 0, 255);
  //ldrValue = constrain(ldrValue, 80, 200);
  Serial.print("LED Brightness = "); Serial.println(ldrValue);
  LEDS.setBrightness(ldrValue);
  
}

/******btSerialCmd()*******///Bluetooth serial command send by mobile.
void btSerialCmd(){
    while (btSerial.available() > 0){
        char inChar = btSerial.read();
        //Serial.print ("Date Time: ");
        if(inChar == ',' && txtMsg != ""){ //when serial String buffer met "," the stored string store in to the setData[]array while conveting to the intiger
            //Serial.print ("Date Time: ");
            //Serial.println(txtMsg);
            setData[btSetDataIndex] =  txtMsg.toInt();
            //Serial.print("DateTimeData :"); Serial.println(setData[btSetDataIndex]);
            txtMsg = "";
            btSetDataIndex++; //increment index to store another value in array.
        }else{
            txtMsg += inChar;
            switch(inChar){
                case 'A': //for sysnc with phone date time
                    btSetDataIndex = 0;
                    txtMsg = "";
                    bluetoothSetupMode = 1; //for purpose of bluetoothSetup()
                    break;    
                case 'B': //for 12 hour date format change
                    timeFormat = true;
                    txtMsg = "";
                    break;
                case 'C': //for 24 hour date format change
                    timeFormat = false;
                    txtMsg = "";
                    break;
                case 'D': //for alarm set via bluetooth
                    btSetDataIndex = 6;
                    txtMsg = "";
                    alarmSet = true;
                    break;
                case 'E': //for alarm Reset via bluetooth
                    //btSetDataIndex = 6;
                    setData[6] = -1;
                    setData[7] = -1;
                    alarmSet = false;
                    txtMsg = "";
                    break;
                case 'F': //for alarm Off via bluetooth
                    alarmStopTasks();
                    txtMsg = "";
                    break;
                case 'G': //for time set via bluetooth
                    btSetDataIndex = 0;
                    txtMsg = "";
                    bluetoothSetupMode = 2;
                    break;
                case 'H': //for Date set via bluetooth
                    btSetDataIndex = 3;
                    txtMsg = "";
                    bluetoothSetupMode = 3;
                    break;
                case 'I': //for Auto color mode ON
                    autoColor = true;
                    txtMsg = "";
                    break;
                case 'J': //Set manual color for LED and Auto color mode off
                    autoColor = false;
                    btSetDataIndex = 8; //RED[8], GREEN[9], BLUE[10]
                    bluetoothSetupMode = 4;
                    txtMsg = "";
                    break;
                case 'K': //for time tell voice on
                    voiceTime = true;
                    txtMsg = "";
                    break;
                case 'L': //for time tell voice off
                    voiceTime = false;
                    txtMsg = "";
                    break;
                case 'M': //for Adjust backlight
                    btSetDataIndex = 11;
                    txtMsg = "";
                    bluetoothSetupMode = 5; //for purpose of bluetoothSetup()
                    break; 
                case 'N': //for Adjust volume of voice
                    btSetDataIndex = 12;
                    txtMsg = "";
                    bluetoothSetupMode = 6; //for purpose of bluetoothSetup()
                    break; 
                case 'P': //auto backlight adjustment enable
                    autoBacklight = true;
                    txtMsg = "";
                    break; 
            } 
        }
    }//end while
}


/******bluetoothSetup()*******/
void bluetoothSetup(byte btMode){
    if(btMode > 0){
        if(btMode == 1 && btSetDataIndex == 6){ //btSetDataIndex == 6 since wait untill bluetooth receive data write to array completely. +1
          //Serial.print("bluetoothSetupMode :");  Serial.println(bluetoothSetupMode);
            bluetoothSetupMode = 0; //RESET SETUP MODE
            btSetDataIndex = 0;  //RESET ARRAY INDEX
            RTC.adjust(DateTime(setData[5], setData[3], setData[4], setData[0], setData[1], setData[2]));//YYYY[0],MM[1],DD[2],HH[3],MIN[4],Sec[5]
            delay(100);
        }else if(btMode == 2 && btSetDataIndex == 2){ //adjust time
            bluetoothSetupMode = 0;
            btSetDataIndex = 0;
            RTC.adjust(DateTime(dateTimeData[5], dateTimeData[3], dateTimeData[4], setData[0], setData[1], dateTimeData[2]));//YYYY[0],MM[1],DD[2],HH[3],MIN[4],Sec[5]
            delay(100);
        }else if(btMode == 3 && btSetDataIndex == 6){ //adjust date
            bluetoothSetupMode = 0;
            btSetDataIndex = 0;
            RTC.adjust(DateTime(setData[5], setData[3], setData[4], dateTimeData[0], dateTimeData[1], dateTimeData[2]));//YYYY[0],MM[1],DD[2],HH[3],MIN[4],Sec[5]
            delay(100);
        }else if(btMode == 4 && btSetDataIndex == 11){ //RED[8], GREEN[9], BLUE[10]
            red = setData[8];  //apply RGB values
            green = setData[9];
            blue = setData[10];
            bluetoothSetupMode = 0;
            btSetDataIndex = 0;
        }else if(btMode == 5 && btSetDataIndex == 12){ //adjust backlight
            bluetoothSetupMode = 0;
            btSetDataIndex = 0;
            autoBacklight = false; //auto backlight off
            LEDS.setBrightness(setData[11]); // Set the backlight brightness 30-255
        }else if(btMode == 6 && btSetDataIndex == 13){ //adjust volume of voice  // Set the volume level(0x00~0x30)
            bluetoothSetupMode = 0;
            btSetDataIndex = 0;
            execute_CMD(0x06, 0, setData[12]); // Set the volume level(0x00~0x30)
        }
    }
}

/******timeGetNow()********///show time in LEDs 24 hour format.
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
    if(timeFormat){ //show time in LEDs 12 hour format.
        if(Hour > 12){
            /*if(Hour > 19){
                Hour = Hour % 10 + 8; 
            }
            else{
                Hour = Hour %10 - 2; 
            }*/
            Hour = Hour % 12;
        }
        if(Hour == 0 || Hour == 12){
            Hour = 12;
        }
    }
    int nowTime = Hour * 100 + Minute;  //eg-: hour-11, minite-25 (11*100+25=1125) take Hour & Minute in a single value.
    ledDisplayGenerator(nowTime, 0, 0);  //send taken single value for generate LEDs display array. 
}

/******timeGetNow()*******show time in LEDs 12 hour AM/PM format.
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
}*/



/******dateGetNow()********///show date in LEDs.
void dateGetNow(){
    byte Month = dateTimeData[3];  //store Month & Day to the veriables.
    byte Date = dateTimeData[4];
    int monthDate = Month * 100 + Date; //eg-: month-11, date-25 (11*100+25=1125) take Month & Date in a single value.
    leds[42] = CRGB(red,green,blue); leds[43] = CRGB(red,green,blue);  //Month and Date Separete 2LEDs continuesly high
    ledDisplayGenerator(monthDate, 0, 0);
}

/******getTempeture()********///shoe tempeture in LEDs.
void getTempeture(){
     DateTime now = RTC.now(); //get realtime tempeture from RTC object
     float temp = RTC.getTemperature(); //value store in float veriable.(Eg:- 28.90) 
     int displayTemp = (temp * 100) / 10; //get tempeture in single val, Have bug when three nuber tempeture like 3.20(bug fixed 20/09/2019 through ledDisplayGenerator())
     //Serial.print(displayTemp);
     leds[42] = CRGB(0,0,0); 
     leds[43] = CRGB(red,green,blue);  //showing separetor.
     ledDisplayGenerator(displayTemp, 99, 0); //val 99 for identification purpose only."ledDisplayGenerator90" know 99 send tempeture.
}

/******timeSet()********///Set the Time
void timeSet(){
    if(btnModVal == 0){  //chk btnModVal status, 0 is Edit Time.
        editMode = true;  //since true, Time/Date/Tempeture not updating in LEDs. 
        //Serial.println("Edit Position -- 0");
        digitalWrite(ledModDate,HIGH);  //on LED relevant Set Time mode(1st LED).
        delay(400); //delay for clear button press.
        byte btnPlus = digitalRead(btnPlusInput);   //take +Plus+ button status.
        byte btnMinus = digitalRead(btnMinusInput); //take -Minus- button status.
        if(setPosition){ //if "setPosition = true" edit Hour
            setIndex = 1;  //use to setValToRTC() function direct to switch statement
            if(btnPlus == HIGH){
                increaseVal(0, 0, 1); //0 = setData[0](hour), 0 = minMaxVal[0][1](hour max val), 1 = setData[1](minute-cuple for led disply)
            }
            if(btnMinus == HIGH){
                decreaseVal(0, 0, 1); //0 = setData[0](hour), 0 = minMaxVal[0][0](hour min val), 1 = setData[1](minute-cuple for led disply)
            }
        }else{   //edit Minute
            setIndex = 2;
            if(btnPlus == HIGH){
                increaseVal(1, 1, 0); //1 = setData[1](minute), 1 = minMaxVal[1][1](minute max val), 0 = setData[0](hour-cuple for led disply)
            }
            if(btnMinus == HIGH){
                decreaseVal(1, 1, 0);
            }
        }
    } 
    setValToRTC();  //write updated values to RTC.
    reset();  //reset edit mode status.
}

/******dateSet()********///Set the Date
void dateSet(){
    if(btnModVal == 1){  //chk btnModVal status, 1 is Edit Date.
        editMode = true;
        //Serial.println("Edit Position -- 1");
        digitalWrite(ledModTime,HIGH); //on LED relevant Set Date mode(2nd LED).
        delay(400);
        byte btnPlus = digitalRead(btnPlusInput);   
        byte btnMinus = digitalRead(btnMinusInput); 
        if(setPosition){ //if "setPosition = true" edit Month
            setIndex = 3;  //use to setValToRTC() function direct to switch statement  
            if(btnPlus == HIGH){
                increaseVal(3, 2, 4);  //3 = setData[3](month), 2 = minMaxVal[2][1](month max val), 4 = setData[4](date-cuple for led disply)
            }
            if(btnMinus == HIGH){
                decreaseVal(3, 2, 4);
            }
        }else{  //edit Date
            setIndex = 4;
            if(btnPlus == HIGH){
                increaseVal(4, 3, 3);//3
            }
            if(btnMinus == HIGH){
                decreaseVal(4, 3, 3); //4 = setData[4](day), 3 = minMaxVal[1][1](minute min val), 3 = setData[3](month-cuple for led disply)
            }
        }
    } 
    setValToRTC();
    reset();
}

/******alrmSet()********///Setting Alrm
void alrmSet(){
    if(btnModVal == 2){ //chk btnModVal status, 2 is Set alarm.
        editMode = true;
        //Serial.println("Edit Position -- 2");
        digitalWrite(ledModAlarm,HIGH); 
        delay(400);
        byte btnPlus = digitalRead(btnPlusInput);   
        byte btnMinus = digitalRead(btnMinusInput); 
        if(setPosition){ //if "setPosition = true" edit elarm Hour
            setIndex = 5; //this go to setValToRTC()but not write to RTC, just store valu in setData[].
            if(btnPlus == HIGH){
                increaseVal(6, 0, 7); 
            } 
            if(btnMinus == HIGH){
                decreaseVal(6, 0, 7);
            }
        }else{ //edit Minute
            setIndex = 6;
            if(btnPlus == HIGH){
                increaseVal(7, 1, 6);
            }
            if(btnMinus == HIGH){
                decreaseVal(7, 1, 6);
            }  
        }
    }
    setValToRTC(); 
    reset();
}

/******yearSet()********///srtting year
void yearSet(){
    if(btnModVal == 3){
        editMode = true;
        setIndex = 7;
        digitalWrite(ledModDate,HIGH);
        digitalWrite(ledModTime,HIGH);
        digitalWrite(ledModAlarm,HIGH);
        delay(400);
        //Serial.println("------Year Edit Mode------");
        byte btnPlus = digitalRead(btnPlusInput);   
        byte btnMinus = digitalRead(btnMinusInput); 
        if(btnPlus == HIGH){
            increaseVal(5, 4, 0);
        }
        if(btnMinus == HIGH){
            decreaseVal(5,4, 0);
        }
    }
    setValToRTC();
    reset();
}

/******checkAlarm()********///Check Alrm is On & exit edit mode RESET
void checkAlarm(){
    byte btnElarmStop = digitalRead(btnAlrmOffInput);
    if(((dateTimeData[0] == setData[6]) && (dateTimeData[1] == setData[7]) && alarmSet) || alarmBeepUntillStop){
        //Serial.print(dateTimeData[0]); Serial.print("==");  Serial.print(setData[6]); Serial.print("------"); Serial.print(dateTimeData[1]); Serial.print("==");  Serial.print(setData[7]);
        //Serial.print("--------alarmBeepUntillStop=====");  Serial.print(alarmBeepUntillStop);
        //Serial.println("Alrm beeeeeeeeeeeeeeeeeep");
        alarmBeepUntillStop = true;
        if(!alarmPlayExecute){
            alarmPlayExecute = true; //value true for stop iterate play values to mp3 module
            execute_CMD(0x17,0,2); //loop play folder 002 content
            delay(100);
        }
    }
    if(btnElarmStop == HIGH && editMode){ //reset from edit mode.exit edit mode.
        btnModVal = 4; //4 is reset value.check reset().
    }
    if(btnElarmStop == HIGH){ //chk elarmStopBtn is pressed
        alarmStopTasks();
    }
}

/******alarmStopTasks()********///Alarm stopped tasks
void alarmStopTasks(){
    alarmBeepUntillStop = false;
    alarmSet = false;
    alarmPlayExecute = false; //stop iterate alarm loopingcommand execute to mp3 module
    execute_CMD(0x16,0,0); //stop playback
    delay(100);
    setData[6] = -1;  //reset alrm set values in setData[] array.
    setData[7] = -1;
}

/******reset()********///reset mode LED and some veriable values
void reset(){
    digitalWrite(ledModDate,LOW);  //Off each mode leds.
    digitalWrite(ledModTime,LOW);
    digitalWrite(ledModAlarm,LOW);
    if(btnModVal == 4 || millis() - lastPressTimeBtnMode >= 300000){  //if not reset, after 5 minute it will be reset. 4 is last mode value.it use as reset value.
        digitalWrite(ledModDate,LOW);
        digitalWrite(ledModTime,LOW);
        digitalWrite(ledModAlarm,LOW); 
        btnModVal = -1; //reset btnModVal value.
        setPosition = true; //reset setPosition value.
        editMode = false;
    }
}

/******ledDisplayGenerator()********/
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

/******setValToRTC()********///Edit functions values write to RTC.
void setValToRTC(){
    byte btnSet = digitalRead(btnSetInput); //check SET button is pressed.
    if(btnSet == HIGH){
        //Serial.println("Set Button Pressed");
        switch(setIndex){ //check what is the "setIndex" value.
            case 1:  //hour set
                setPosition = false; //allow next element (minute) to edit.
                writeToRTC(3, 0);  //send data to writeToRTC() function. 3(rtcQueue[3]-located inside writeToRTC function), 0(setData[0]-array)
                delay(500);
            break;
            case 2:  //minute set
                setPosition = true;  //set to default value (Hour) edit.
                btnModVal = 4;  //for reset btnModVal and LED off according to it.see reset() function. 
                setIndex = 0;  //back to default value.switch statement need this value.
                writeToRTC(4, 1);
                editMode = false;
                delay(500);
            break;
            case 3:  //month set
                setPosition = false; //allow next element (minute) to edit.
                writeToRTC(1, 3);  
                delay(500);
            break;
            case 4:  //date set
                setPosition = true;  //set todefault value (Hour) edit.
                btnModVal = 4;  //for reset btnModVal and LED off according to it.see reset() function. 
                setIndex = 0;  
                writeToRTC(2, 4);
                delay(500);
                editMode = false;
            break;
            case 5: //alrm hour set
                setPosition = false; //allow next element (minute) to edit.
                //alarmSet = true; //alrm set value become true
                delay(500);
            break;
            case 6:  //alarm minute set
                setPosition = true;  //set to default value (Hour) edit.
                alarmSet = true; //alrm set value become true
                btnModVal = 4;  //for reset btnModVal and LED off according to it.see reset() function. 
                setIndex = 0;  //back to default value.swich statement need thisvalue.
                delay(500);
                editMode = false;
            break;
            case 7:  //Year set
                btnModVal = 4;  //for reset btnModVal and LED off according to it.see reset() function. 
                setIndex = 0;  //back to default value.swich statement need thisvalue.
                writeToRTC(0, 5);
                delay(500);
                editMode = false;
            break;
            default:
                //Serial.println("ignor");
                editMode = false;
            break;
        }
    }
}

/******writeToRTC()********///send updated values to RTC
void writeToRTC(byte writeIndex, byte setDataIndex){  //writeIndex for rtcQueue[], setDataIndex for setData[].
    int rtcQueue[] = {dateTimeData[5], dateTimeData[3], dateTimeData[4], dateTimeData[0], dateTimeData[1]};//YYYY[0],MM[1],DD[2],HH[3],MIN[4]
    rtcQueue[writeIndex] = setData[setDataIndex]; //write updated value to the rtcQueue[] from setData[] array.
    dateTimeData[setDataIndex] = setData[setDataIndex];
    /*Serial.print("rtcQueue[i]----");
    for(byte i=0; i<5;i++){
      Serial.print(rtcQueue[i]);
      Serial.print("["); Serial.print(i);  Serial.print("]");
    } Serial.println();
    Serial.print("setData[i]----");
    for(byte i=0; i<8;i++){
      Serial.print(setData[i]);
      Serial.print("["); Serial.print(i);  Serial.print("]");
    }Serial.println();
    Serial.print("dateTimeData[i]----");
    for(byte i=0; i<6;i++){
      Serial.print(dateTimeData[i]);
      Serial.print("["); Serial.print(i);  Serial.print("]");
    }Serial.println();*/
    RTC.adjust(DateTime(rtcQueue[0], rtcQueue[1], rtcQueue[2], rtcQueue[3], rtcQueue[4], 0)); //YYYY[0],MM[1],DD[2],HH[3],MIN[4],Sec[5]
}

/******increaseVal()*******/
void increaseVal(byte setDataArrayPos, byte arrayIndex, byte cupleValIndex){  //setDataArrayPos(location of"setData[]"),arrayIndex(relevant max value of index"minMaxVal[5][2]" & its position "arrayPos")
    setData[setDataArrayPos] = setData[setDataArrayPos] + 1; //increase value according to btnPlus pressed.
    setData[setDataArrayPos] = (setData[setDataArrayPos] < minMaxVal[arrayIndex][1]) ? setData[setDataArrayPos] : minMaxVal[arrayIndex][0]; //chk value greater than max value and store to setData array.
    //Serial.println(setData[setDataArrayPos]);
    if(setPosition){
        if(setDataArrayPos == 5){ //this is for year increase only,Since year has 4numbers. it fitted to ledDisplayGenerator() perfectly:)
            ledDisplayGenerator(setData[setDataArrayPos], 0, 0);
            FastLED.show();  
            //Serial.print("----------------------------------");
            //Serial.println(setData[setDataArrayPos]);
        }else{
            byte minusValChk = (setData[cupleValIndex] == -1) ? 0 : setData[cupleValIndex];
            leds[42] = CRGB(red,green,blue); leds[43] = CRGB(red,green,blue);
            ledDisplayGenerator(0, setData[setDataArrayPos], minusValChk); //0 send as 1st parameter since this is not date/time value. 2nd parameter represent 4th and 3rd digit, 3rd parameter represent 2nd and 1st digit 
            FastLED.show(); 
            //Serial.print("----------------------------------");
            //Serial.print(setData[setDataArrayPos]);  //Serial.println(minusValChk);
        }
    }else{
        byte minusValChk = (setData[cupleValIndex] == -1) ? 0 : setData[cupleValIndex];
        leds[42] = CRGB(red,green,blue); leds[43] = CRGB(red,green,blue);
        ledDisplayGenerator(0, minusValChk, setData[setDataArrayPos]);
        FastLED.show(); 
        //Serial.print("----------------------------------");
        //Serial.print(minusValChk);  //Serial.println(setData[setDataArrayPos]); 
    }
    delay(30);  
}

/******decreaseVal()*******/
void decreaseVal(byte setDataArrayPos, byte arrayIndex, byte cupleValIndex){
    setData[setDataArrayPos] = (setData[setDataArrayPos] <= minMaxVal[arrayIndex][0]) ?  minMaxVal[arrayIndex][1] : setData[setDataArrayPos]; //chk value less than min value and store to setData array.
    setData[setDataArrayPos] = setData[setDataArrayPos] - 1; //decrease value according to btnMinus pressed.
    //Serial.println(setData[setDataArrayPos]);
    if(setPosition){
        if(setDataArrayPos == 5){ //this is for year decrease only
            ledDisplayGenerator(setData[setDataArrayPos], 0, 0);
            FastLED.show();  
            //Serial.print("----------------------------------");
            //Serial.println(setData[setDataArrayPos]);
        }else{
            byte minusValChk = (setData[cupleValIndex] == -1) ? 0 : setData[cupleValIndex];
            leds[42] = CRGB(red,green,blue); leds[43] = CRGB(red,green,blue); //hour/month and minute/date separetor allways high in edit mode
            ledDisplayGenerator(0, setData[setDataArrayPos], minusValChk);
            FastLED.show(); 
            //Serial.print("----------------------------------");
            //Serial.print(setData[setDataArrayPos]);  //Serial.println(minusValChk);
        }
    }else{
        byte minusValChk = (setData[cupleValIndex] == -1) ? 0 : setData[cupleValIndex];
        leds[42] = CRGB(red,green,blue); leds[43] = CRGB(red,green,blue);
        ledDisplayGenerator(0, minusValChk, setData[setDataArrayPos]);
        FastLED.show(); 
        //Serial.print("----------------------------------");
        //Serial.print(minusValChk);  //Serial.println(setData[setDataArrayPos]);
    }
    delay(30);
}

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
        mp3Serial.write( Command_line[k]);
    }
}
