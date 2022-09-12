
// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h> 
#include <math.h> //library includes mathematical functions



//flex sensor ports 
int fing[4] = {A7, A6, A3, A0}; 


//MPU testing 
const int MPU=0x68; //I2C address of the MPU-6050
int AcXcal,AcYcal,AcZcal,GyXcal,GyYcal,GyZcal,tcal; //calibration variables
double t,tx,tf,pitch,roll;
// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0xF5, 0xC7, 0x44};


// Define a data structure
typedef struct Data {
  int x[4]; //data strucutre to store value of fingers. 
  int16_t AcX,AcY,AcZ,GyX,GyY,GyZ; //16-bit integers

}Data;

Data myData;

// Peer info
esp_now_peer_info_t peerInfo;

// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  
  // Set up Serial Monitor
  Serial.begin(9600);
  
  //joystick
  //pinMode(A0,INPUT);
  
  //GY-521 SETUP
    Wire.begin(); //initiate wire library and I2C
    Wire.beginTransmission(MPU); //begin transmission to I2C slave device
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)  
    Wire.endTransmission(true); //ends transmission to I2C slave device
    Serial.begin(9600); //serial communication at 9600 bauds

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {

    Wire.beginTransmission(MPU); //begin transmission to I2C slave device
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false); //restarts transmission to I2C slave device
    Wire.requestFrom(MPU,14,true); //request 14 registers in total  

    //Acceleration data correction
    AcXcal = -950;
    AcYcal = -300;
    AcZcal = 0;

    //Gyro correction
    GyXcal = 480;
    GyYcal = 170;
    GyZcal = 210;


    //read accelerometer data
    myData.AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) 0x3C (ACCEL_XOUT_L)  
    myData.AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) 0x3E (ACCEL_YOUT_L) 
    myData.AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) 0x40 (ACCEL_ZOUT_L)
    
    //read gyroscope data
    myData.GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) 0x44 (GYRO_XOUT_L)
    myData.GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) 0x46 (GYRO_YOUT_L)
    myData.GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) 0x48 (GYRO_ZOUT_L) 


   //get pitch/roll
   getAngle(myData.AcX,myData.AcY,myData.AcZ);
  
    //printing values to serial port
    // Serial.print("Angle: ");
    // Serial.print("Pitch = "); Serial.print(pitch);
    // Serial.print(" Roll = "); Serial.println(roll);
  
    // Serial.print("Accelerometer: ");
    // Serial.print("X = "); Serial.print(myData.AcX + AcXcal);
    // Serial.print(" Y = "); Serial.print(myData.AcY + AcYcal);
    // Serial.print(" Z = "); Serial.println(myData.AcZ + AcZcal); 

    
    // Serial.print("Gyroscope: ");
    // Serial.print("X = "); Serial.print(myData.GyX + GyXcal);
    // Serial.print(" Y = "); Serial.print(myData.GyY + GyYcal);
    // Serial.print(" Z = "); Serial.println(myData.GyZ + GyZcal);

    //FINGERS 
    for(int i = 0; i <= 3; i++)
      myData.x[i] = analogRead(fing[i]);
       


  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  }
  else {
    Serial.println("Sending error");
  }
 //delay(200);
}

// convert accelerometer values into pitch and roll
void getAngle(int Ax,int Ay,int Az) 
{
    double x = Ax;
    double y = Ay;
    double z = Az;

    pitch = atan(x/sqrt((y*y) + (z*z))); //pitch calculation
    roll = atan(y/sqrt((x*x) + (z*z))); //roll calculation

    //converting radians into degrees
    pitch = pitch * (180.0/3.14);
    roll = roll * (180.0/3.14) ;
}
