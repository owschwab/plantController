/*
 * Project plantSystem
 * Description: Plant watering system and environment reader
 * Author: Owen S 
 * Date: 3/17/2023
 */

#include <math.h>
#include "neopixel.h"
#include "DFRobotDFPlayerMini.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"
#include "Grove_Air_quality_Sensor.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"

const int PIXELPIN = D6;
const int PIXELCOUNT = 12;
const int AQPIN = A0;
const int SMPIN = A3;
const int PUMPPIN = 12; 
const int OLED_RESET = D3;
const int DUSTPIN = D8;

unsigned long duration;
unsigned long startTime;
unsigned long startTime2;
unsigned long startTime3;
unsigned long startTime4;
unsigned long sampleTime_ms = 30000;
unsigned long lowPulseOccupancy = 0;
float ratio;
float concentration;
int current_quality=-1;
int sensorVal;
int soilMoisture;
int i;
int v;

int currentTime;
int lastTime;
int currentTime2;
int lastTime2;
int currentTime3;
int lastTime3;
int onOFF;
int crTime;
int lasSec;

const int hexAddress = 0x76;
int degSim;
float tempC;
int tempF;
float pressPA;
int pressMG;
int humidRH;

bool subValue;
bool status;

void MQTT_connect();
bool MQTT_ping();

Adafruit_BME280 bme;
TCPClient TheClient;
DFRobotDFPlayerMini myDFPlayer;

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_NeoPixel pixel(PIXELCOUNT, PIXELPIN, WS2812B);
AirQualitySensor airqualitysensor(A0);

void pixelFill(int startingPixel, int endingPixel, int color);


Adafruit_MQTT_Publish pubAirQual = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/airQUALITY");
Adafruit_MQTT_Publish pubDustVal = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dustVALUE");
Adafruit_MQTT_Publish pubSoilMoist = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moistVALUE");
Adafruit_MQTT_Publish pubTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempVal");
Adafruit_MQTT_Publish pubHumid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidVal");
Adafruit_MQTT_Subscribe subFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttonOnOff"); 


SYSTEM_MODE(MANUAL);

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600);

    if (!myDFPlayer.begin(Serial1)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
    }

    myDFPlayer.volume(20);

    Serial.println(F("DFPlayer Mini online."));
  

    pinMode(DUSTPIN, INPUT);
    pinMode(SMPIN, INPUT);
    pinMode(PUMPPIN, OUTPUT);
    airqualitysensor.init();

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.clearDisplay();
    //pixel.setBrightness(15);

    display.setCursor(10,10);
    display.printf("/////////");
    display.display();

    pixel.setBrightness(30);

    status = bme.begin(hexAddress);
    if(status == false) {
        Serial.printf("BME280 at adress 0x%02X faield to start", hexAddress);
    }

    WiFi.on();
    

    //WiFi.setCredentials("XXX_Finititty", "StraightHaus");
    WiFi.connect();
    while(WiFi.connecting()) {
        Serial.printf(".");
    }

    mqtt.subscribe(&subFeed);   

    startTime = millis();

    display.clearDisplay();
    display.setCursor(0,20);
    display.printf("LETS PARTY");
    display.display();
    myDFPlayer.play(5);
    pixelFill(0,5,0xDAF7A6);
    pixelFill(6,9,0x000000);
    pixelFill(10,12,0xDAF7A6);
}


void loop() {
    MQTT_connect();
    MQTT_ping();


    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(100))) {
        if (subscription == &subFeed) {
        subValue = atoi((char *)subFeed.lastread);
        Serial.printf("\nswitch revieved\n Value: %d\n", subValue);
        }

        onOFF=subValue;
        Serial.printf("\nswitch revieved\n onOFF value; %d\n", onOFF);
        
        if(onOFF) {
            digitalWrite(PUMPPIN, HIGH);
            display.clearDisplay();
            display.setCursor(10,10);
            display.printf("PUMP IT!"); 
            Serial.printf("PUMP IT!\n");  
            display.display();
            pixelFill(0,12,0x00FF00);
            currentTime = millis();
        }
    }

    tempC = bme.readTemperature(); 
    pressPA = bme.readPressure();
    humidRH = bme.readHumidity();

    pressMG = pressPA*0.000295;
    tempF = 1.8*tempC+32;
    degSim = 0xF8;

    duration = pulseIn(DUSTPIN, LOW);
    lowPulseOccupancy = lowPulseOccupancy+duration;

    if((millis()-startTime) > sampleTime_ms) {
        ratio = lowPulseOccupancy/(sampleTime_ms+10.0);
        concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
        Serial.printf("Concentration %f\n", concentration);  
        lowPulseOccupancy = 0;
        startTime=millis();
        if(mqtt.Update()) {
            pubAirQual.publish(sensorVal);
            pubDustVal.publish(concentration);
            pubTemp.publish(tempF);
            pubHumid.publish(humidRH);
            pubSoilMoist.publish(soilMoisture);
        }
    }

    currentTime2=millis();   
    if((currentTime2-lastTime2) > 15000) {
        soilMoisture = analogRead(SMPIN);
        Serial.printf("Soil Moisture: %d\n", soilMoisture);
        current_quality=airqualitysensor.slope();
        sensorVal = airqualitysensor.getValue();
        Serial.printf("Air Quality: %d\n", sensorVal);
        lastTime2 = millis();
    }

    if(sensorVal>300 ) {
        display.clearDisplay();
        display.setCursor(10,10);
        display.printf("BAD AIR\n GET OUT");
        display.display();
        pixelFill(0,12,0xFF0000);
        myDFPlayer.play(4);
        startTime3 = millis();
    }
    if((millis()-startTime3)>3000) {
        display.clearDisplay();
        display.display();
        pixel.clear();
        pixel.show();
        startTime3 = millis();
    }

    if(soilMoisture > 3000) {
        digitalWrite(PUMPPIN, HIGH);
        myDFPlayer.play(2);
        display.clearDisplay();
        display.setCursor(10,10);
        display.printf("DONT \n WORRY \n PAUL!");
        display.display();
        pixelFill(0,12,0x00FF00);
        soilMoisture = 2900;
        currentTime = millis();
        currentTime3 = millis();
    }

    if((millis()-currentTime)>3000) {
        digitalWrite(PUMPPIN, LOW);
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0); 
        display.printf("T:%2d%c \nP:%2d \nH:%2d%%", tempF, degSim, pressMG, humidRH);
        display.display();
        pixel.clear();
        pixel.show();

    }
    
    if(soilMoisture < 2500 && sensorVal < 500) {
        // digitalWrite(PUMPPIN, LOW);

        // pixel.clear();
        // pixel.show();
        //display.display();
        //display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0); 
        display.printf("T:%2d%c \nP:%2d \nH:%2d%%", tempF, degSim, pressMG, humidRH);
        display.display();
    }

    if((millis()-currentTime3)>30000){
        myDFPlayer.play(3);
        display.clearDisplay();
        display.setCursor(10,10);
        display.printf("DO YOU \n STILL \n CARE?");
        display.display();
        pixelFill(6,9,0xFF5733);
        currentTime3=millis();

    }
}

void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

void pixelFill(int startingPixel, int endingPixel, int color) {
    for(i=startingPixel; i<=endingPixel; i++) { 
        pixel.setPixelColor(i, color);
    }
    pixel.show();
}