/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <FastLED.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <timer.h>
#include <TimeLib.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>

//number of leds used
#define NUM_LEDS 50
//data and clock pins used
#define DATA_PIN 13
#define CLOCK_PIN 12



// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "xxxx";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "abc123";
char pass[] = "abc123";
auto timer = timer_create_default();
//own variables

bool wakeUpActive=false;
bool wakeUpAlarm=false;
bool rainbow=false;
int alarmWeekday[7];
int alarmMinute;
int alarmHour;
unsigned int lightUpAllIntensity=0;
unsigned int lightUpAllIntensityVal;

//other variables/constructors
CRGB leds[NUM_LEDS];
WidgetRTC rtc;
WidgetLED wakeUpLed(V101); 

//make all leds black and reset intensity values
void allLedsOff(){
  lightUpAllIntensity=0;
  lightUpAllIntensityVal=0;
  rainbow=false;
  Blynk.virtualWrite(V50,0); //set slider to 0 in app
  Blynk.virtualWrite(V51,0); //set slider to 0 in app
  Blynk.virtualWrite(V52,0); //set slider to 0 in app
  //set all leds to "black"
  for(int i=0;i<NUM_LEDS;i++){
    leds[i] = CRGB::Black;
  }
  FastLED.show();
 }

//function get current time and print thru serial for debug use
void getCurrentDateTime(){
  Serial.println(day());
  Serial.println(hour()+" "+minute());
  }

//rainbow spin for eye candy original function: https://gist.github.com/jasoncoon/fa1e7efd8726223c2b5c5eb5f7256d65
bool spinningRainbow(void *) {
  if(rainbow){
  // variable used for the initial hue of the rainbow
  // we start it out at 0
  // but since it's static, it'll keep it's value as we change it
  static byte initialHue = 0;
  
  // increase the hue by 1 each time
  initialHue = initialHue + 1;
  
  // the amount we want the hue to change between each LED
  // by dividing the number of hues (255), by the number of LEDs,
  // this code makes each LED a different color
  // and covers the entire rainbow spectrum (red, orange, yellow, green, blue, indigo, violet)
  byte changeInHue = 255 / NUM_LEDS;
  
  // use FastLED to fill the LEDs with a rainbow
  fill_rainbow(leds, NUM_LEDS, initialHue, changeInHue);
  FastLED.show();
  }
  return true;
} 

//wake up function based on timer
bool ligthUpAll(void *){
    if(0<lightUpAllIntensity and lightUpAllIntensity < 256){
    //for debug
    //Serial.println(lightUpAllIntensity);
    //Serial.println("Ligts up started");
    for(int i=0;i<NUM_LEDS;i++){
      leds[i].setRGB(lightUpAllIntensity,lightUpAllIntensity,lightUpAllIntensity);
      }
    FastLED.show();
    lightUpAllIntensity++;
    lightUpAllIntensityVal=lightUpAllIntensity;   
    return true;
      }else{
           lightUpAllIntensity=0;
           wakeUpAlarm=false;
           return true;
      }
  }

//function to check connection to server
bool checkConnection(void *){
  if(!Blynk.connected())
  {
   if(!wakeUpActive){
      allLedsOff();
    }
    Serial.println("NOT connected to Blynk!"); //for debug
    return true;
   }else{
    //Serial.println("Connected to Blynk!"); //for debug
    return true;
  } 
}
//function to print current time to serial port, only for debug use
bool sendTime(void *)
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
  Serial.println(weekday());
  return true;
}

//function to update widget in app
bool sendValuesToApp(void *){
  Blynk.virtualWrite(V100, lightUpAllIntensityVal*100/255);
  return true;
}

//function to compare current time and set alarm time
bool checkAlarm(void *){
if(wakeUpActive and !wakeUpAlarm){
  //Serial.println("Häly päällä");//for debug
  //Serial.println(weekday()); //for debug
    for(int d=0;d<=6;d++){
    //Serial.println(alarmWeekday[d]); //for debug
      if(alarmWeekday[d] == weekday()){
       // Serial.println("Alarm day match!"); //for debug
        Serial.println(hour()); //for debug
        Serial.println(alarmHour); //for debug
        if(alarmHour == hour()){
          Serial.println("Alarm hour match!");//for debug
          Serial.println(minute()); //for debug
          Serial.println(alarmMinute); //for debug
          if(alarmMinute == minute()){
          Serial.println("Alarm minute match!");//for debug
          wakeUpAlarm=true;
          lightUpAllIntensity=1;
        }
      }
    }
   }
 }
 return true;
}


//virtual pin V0 for shutdown the leds, connected to button on app
BLYNK_WRITE(V0){
  allLedsOff();  
}

//virtual pin V10 for activate the wake up, connected to button on app. NEEDS time to be set one time (add Blynk.virtualWrite(V60)
BLYNK_WRITE(V10){
  //Serial.println("Virtual 10"); //for debug
  Blynk.syncVirtual(V60); //read values from app if value is not changed during use
  if(!wakeUpActive){
    wakeUpLed.on();
    wakeUpActive=true;
  //Serial.println("ON"); //for debug
  }else{
    wakeUpLed.off();
    wakeUpActive=false;
  //Serial.println("OFF"); //for debug
  }
  
 }

//virtual pin V11 for wake up functional test, connected to button on app
BLYNK_WRITE(V11){
  //Serial.println("Virtual 11"); //for debug
  lightUpAllIntensity=1;
 }

//virtual pin V12 for rainbow effect, connected to button on app 
BLYNK_WRITE(V12){
  //Serial.println("Virtual 12"); //for debug
   rainbow=true;
  }

//virtual slider V50 to adjust red value of all leds, connected to slider on app
BLYNK_WRITE(V50){
  lightUpAllIntensity=0;
  int valR=param.asInt();
  for(int i=0;i<NUM_LEDS;i++){
      leds[i].r=valR;
   } 
   FastLED.show();
}
//virtual slider V51 to adjust green value of all leds, connected to slider on app
BLYNK_WRITE(V51){
  lightUpAllIntensity=0;
  int valG=param.asInt();
  for(int i=0;i<NUM_LEDS;i++){
      leds[i].g=valG;
   } 
   FastLED.show();
}
//virtual slider V52 to adjust blue value of all leds, connected to slider on app
BLYNK_WRITE(V52){
  lightUpAllIntensity=0;
  int valB=param.asInt();
  for(int i=0;i<NUM_LEDS;i++){
      leds[i].b=valB;
   }  
   FastLED.show();
}

//virtual setting of V60 to adjust wake up time, connected to time input on app
BLYNK_WRITE(V60){
  TimeInputParam t(param);
  alarmHour=t.getStartHour();
  alarmMinute=t.getStartMinute(); 
  //do conversion of days manually due difference in weekday() and isWeekdaySelected(DAY) function return values
    if(t.isWeekdaySelected(7)){
    alarmWeekday[0]=1; // 
    }else if(t.isWeekdaySelected(1)){
    alarmWeekday[1]=2;
    }else if(t.isWeekdaySelected(2)){
    alarmWeekday[1]=3;
    }else if(t.isWeekdaySelected(3)){
    alarmWeekday[1]=4;
    }else if(t.isWeekdaySelected(4)){
    alarmWeekday[1]=5;
    }else if(t.isWeekdaySelected(5)){
    alarmWeekday[1]=6;
    }else if(t.isWeekdaySelected(6)){
    alarmWeekday[1]=7;
    }
     //Serial.println(alarmHour); //for debug
  //Serial.println(alarmMinute);//for debug
  /*for(int a=0;a<=6;a++){      
   Serial.println(alarmWeekday[a]);//for debug 
  } */  
  }


BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();  
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  allLedsOff();
  Blynk.begin(auth, ssid, pass);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

  timer.every(10000, checkConnection);
  timer.every(500, ligthUpAll);
  timer.every(500, sendValuesToApp);
  //timer.every(1000, sendTime); //for debug
  timer.every(1000, checkAlarm); //check alarm every second
  timer.every(50, spinningRainbow); //timer for rainbow effect so other tasks can be done at same time
}

void loop()
{
  timer.tick(); // tick the timer
  Blynk.run(); //start the blynk
}

