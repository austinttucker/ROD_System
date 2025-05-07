#include <Arduino.h>
#define LED 2

// Time interval in milliseconds
const unsigned long CHECK_INTERVAL = 30000;
const int pirPin = 0;  //pin of PIR sensor
volatile bool motionDetected = false;


// put function declarations here:
void setup();
void loop();
void motionDetectedISR();
void updateDynamoDB(int roomNumber, bool isOccupied);

  void setup() {
    // Initialize serial communication for testing
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Set pin mode
    // pinMode(pirPin, INPUT);

    // Attach interrupt to PIR sensor pin
    attachInterrupt(digitalPinToInterrupt(pirPin), motionDetectedISR, RISING);
}

void loop() {
  if (motionDetected) {
      // updateDynamoDB(1000000, true);
      motionDetected = false;
      digitalWrite(LED, HIGH);
  }
  else{
    digitalWrite(LED, LOW);
  }

  // Wait for the next check interval
  delay(CHECK_INTERVAL);
}

void motionDetectedISR() {
  motionDetected = true;
}

void updateDynamoDB(int roomNumber, bool isOccupied) {
  // Placeholder function to simulate DynamoDB update
  Serial.print("Room number: ");
  Serial.print(roomNumber);
  Serial.print(" is occupied: ");
  Serial.println(isOccupied);
}