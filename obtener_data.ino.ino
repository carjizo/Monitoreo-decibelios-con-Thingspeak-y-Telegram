#include <CTBot.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
 
const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

#define SENSOR_PIN A0
#define LED_VERDE D3
#define LED_AMARILLO D4
#define LED_ROJO D5
 
const char *apiKey = ""; // Enter your Write API key from ThingSpeak
const char *ssid = "";     // replace with your wifi ssid and wpa2 key
const char *pass = "";
const char *token_telegrambot = "";   // Enter your token Telegrambot
unsigned long myChannelNumber = ;  // Enter your ID ChannelNumber telegrambot
 
WiFiClient client;
CTBot Decibelimetro_bot;
 
void setup ()  
{   
  pinMode (SENSOR_PIN, INPUT); // Set the signal pin as input  
  pinMode (LED_VERDE, OUTPUT);
  pinMode (LED_AMARILLO, OUTPUT);
  pinMode (LED_ROJO, OUTPUT);
  
  Serial.begin(115200);
  ThingSpeak.begin(client);
 
  Serial.println("Connecting to ");
  Serial.println(ssid);

  Decibelimetro_bot.wifiConnect(ssid, pass);
  Decibelimetro_bot.setTelegramToken(token_telegrambot);

  if (Decibelimetro_bot.testConnection())
    Serial.println("\ntestConnection OK");
  else
    Serial.println("\ntestConnection NOK"); 
    Serial.println("");
    Serial.println("WiFi connected");
}  
 
   
void loop ()  
{ 
  TBMessage msg;
   unsigned long startMillis= millis();                   // Start of sample window
   float peakToPeak = 0;                                  // peak-to-peak level
   unsigned int signalMax = 0;                            //minimum value
   unsigned int signalMin = 1024;                         //maximum value
                                                          // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(SENSOR_PIN);                    //get reading from microphone
      if (sample < 1024)                                  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // save just the min levels
         }
      }
   }
 
   peakToPeak = signalMax - signalMin;                    // max - min = peak-peak amplitude
   int db = map(peakToPeak,20,1750,38,115);             //calibrate for deciBels

    if (db < 50)
    {
      Serial.print("Nivel: Bajo");
      digitalWrite(LED_VERDE, HIGH);
      Serial.print(" ");
      Serial.println(db);    
    }
    else if (db >=55  && db<=75)
    {
      Serial.print("Nivel: Considerable");
      digitalWrite(LED_AMARILLO, HIGH);
      Serial.print(" ");
      Serial.println(db);
    }
    else if (db>75)
    {
      Serial.print("Nivel: Alto");
      digitalWrite(LED_ROJO, HIGH);
      Serial.print(" ");
      Serial.println(db);
    }

  // Envío de data al telegram
    if (Decibelimetro_bot.getNewMessage(msg)) {
      if (msg.messageType == CTBotMessageText) {
       if (msg.text.equalsIgnoreCase("nivel")) {
          Decibelimetro_bot.sendMessage(msg.sender.id, String(db) + " dB"); 
       }
      }
    }
  //Envío de data al Thingspeak
    ThingSpeak.setField(1, db);
    ThingSpeak.writeFields(myChannelNumber, apiKey);
   //delay(300);      // thingspeak needs minimum 3 sec delay between updates.
}
