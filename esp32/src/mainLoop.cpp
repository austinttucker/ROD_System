/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/
#include <Arduino.h>
#define timeSeconds 10

//----Set GPIOs for LED and PIR Motion Sensor
const int led = 23;
const int PIRSensor = 4;

// -----Timer: Auxiliary variables
#define timeSeconds 10
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

//---Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement()
{
    Serial.println(" MOTION DETECTED ");
    Serial.println("Turning ON the LED");
    digitalWrite(led, HIGH);
    startTimer = true;
    lastTrigger = millis();
}

// void setup()
// {
//     Serial.begin(115200); // Serial port for debugging purposes
//     connect_wifi("your_wifi_ssid", "your_wifi_password");
//     update_dynamodb(101, true);
//     //     pinMode(PIRSensor, INPUT_PULLUP); // PIR Motion Sensor mode INPUT_PULLUP
//     //     pinMode(led, OUTPUT);
//     //     digitalWrite(led, LOW);
//     //     attachInterrupt(digitalPinToInterrupt(PIRSensor), detectsMovement, FALLING); // Set PIRSensor pin as interrupt, assign interrupt function and set RISING mode
// }

void loop()
{
    now = millis();
    if (startTimer && (now - lastTrigger > (timeSeconds * 500)))
    {
        Serial.println(" Turning OFF the LED ");
        digitalWrite(led, LOW);
        startTimer = false;
    }
}