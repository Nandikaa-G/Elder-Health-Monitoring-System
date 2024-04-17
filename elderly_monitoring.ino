#include <SoftwareSerial.h>
#include <dht11.h>
#include <Wire.h>
#include "MPU6050.h"

#define RX 2
#define TX 3
#define dht_apin 11 // Analog Pin sensor is connected to

dht11 dhtObject;
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

String AP = "ReverGenie";       // AP NAME
String PASS = "Passpass"; // AP PASSWORD
String API = "GC9XYHZF9A7PM1FD";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";

int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
  
SoftwareSerial esp8266(RX,TX); 
  
struct MyData {
  double X;
  double Y;
  double Z;
};

MyData data;

void setup() {
  Serial.begin(9600);
  mpu.initialize();
  pinMode(A0, INPUT);

  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}

void loop() {
 String getData = "GET /update?api_key="+ API +"&field1="+getTemperatureValue()+"&field2="+getHumidityValue()+"&field3="+getHeartRateValue()+"&field4="+getAccelerationMag();
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);
 delay(16000);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
}


String getTemperatureValue(){

   dhtObject.read(dht_apin);
   Serial.print(" Temperature(C)= ");
   int temp = dhtObject.temperature;
   Serial.println(temp); 
   delay(50);
   return String(temp); 
  
}


String getHumidityValue(){

   dhtObject.read(dht_apin);
   Serial.print(" Humidity in %= ");
   int humidity = dhtObject.humidity;
   Serial.println(humidity);
   delay(50);
   return String(humidity); 
  
}

String getHeartRateValue(){

  float pulse;
  int sum = 0;
  for (int i = 0; i < 20; i++)
    sum += analogRead(A0);
  pulse = sum / 200.00;
  Serial.print(" Heart rate in bpm = ");
  Serial.println(pulse);
  delay(50);
  return String(pulse);

}

String getAccelerationMag(){
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  data.X = map(ax, -17000, 17000, 0, 255 ); // X axis data
  data.Y = map(ay, -17000, 17000, 0, 255); 
  data.Z = map(az, -17000, 17000, 0, 255);  // Y axis data
  Serial.print("Axis X = ");
  Serial.print(data.X);
  Serial.print("  ");
  Serial.print("Axis Y = ");
  Serial.print(data.Y);
  Serial.print("  ");
  Serial.print("Axis Z  = ");
  Serial.println(data.Z);
  float mag = sqrt(data.X*data.X + data.Y*data.Y + data.Z*data.Z);
  Serial.print("Magnitude of acceleration: ");
  Serial.println(mag);
  delay(50);
  return String(mag);
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
