
// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
#include<Wire.h>

//motor controls 
const int IN1 = 18; 
const int IN2 = 5;  
const int IN3 = 17; 
const int IN4 = 16; 


// Define a data structure
typedef struct Data {
  int x[4]; //fingers 
  int16_t AcX,AcY,AcZ,GyX,GyY,GyZ; //16-bit integers
}Data;

Data myData;

//RTOS using Two cores
TaskHandle_t Task1; 
TaskHandle_t Task2; 


// Callback function executed when data is received this function used kinda like our loop 
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
 
  //function calls 
   xTaskCreatePinnedToCore(IMU,"Task1",1000,NULL,0,&Task1,0); //IMU core 1 
   xTaskCreatePinnedToCore(flex_sensor,"Task2",1000,NULL,0,&Task2,1);  //flex_sensor core 2 
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(9600);

  //Set up motors as output
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
}

void flex_sensor(void *parameters)
{
  while(1)
  {
    //flex sensor
    for(int i = 0; i <= 3; i++)
    {
      if(myData.x[i] < 3600) //Forwards
        {
          digitalWrite(IN3,HIGH);
          digitalWrite(IN4,LOW);
          digitalWrite(IN1,HIGH);
          digitalWrite(IN2,LOW); 

          //Testing purposes
          //Serial.println("All flex ");
          //Serial.println(myData.x[i]);  
        }
      else if(myData.x[3] < 3700 && myData.x[2] < 3700) //Backwards
        {
          digitalWrite(IN3,LOW);
          digitalWrite(IN4,HIGH);
          digitalWrite(IN1,LOW);
          digitalWrite(IN2,HIGH);

          //Testing purposes 
          // Serial.println("last two flex ");
          // Serial.println(myData.x[3]);
          // Serial.println(myData.x[2]);
        }
      else
        {
          digitalWrite(IN3,LOW);
          digitalWrite(IN4,LOW);
          digitalWrite(IN1,LOW);
          digitalWrite(IN2,LOW);
        }
    }
    //delay(100); 
  }
}

//IMU function may need to be looked at 
void IMU(void *parameters)
{
  while(1)
  {
    if(myData.AcY > 1920) //turns right 
      {
        digitalWrite(IN3,HIGH);
        digitalWrite(IN4,LOW);
        Serial.println("testing IMU if move ");
      }
    else
      {
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
      }

    if(myData.AcY < -9000) //turns left 
      {
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
      }
    else
      {
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
      }
    // delay(50);
  }

}

//no need to use this loop function for wifi communication 
void loop() {
  
}
