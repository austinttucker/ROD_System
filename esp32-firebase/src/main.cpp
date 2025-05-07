/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#include <esp_sleep.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Connor5g"
#define WIFI_PASSWORD "Feb262021"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBEaP90MeuJJOKCU3DfstvMPwW6f1PVvs0"

// Insert RTDB URL
#define DATABASE_URL "https://rod-system-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// Define a constant for the room number
const char *ROOM_KEY = "200-A"; // Change this to the desired room key

// Define a boolean for the room's occupancy status
bool isEmpty = true;

const int PIR_PIN = 4;
RTC_DATA_ATTR bool motionDetected = false; // Keep value during deep sleep

// Define update interval for database (in microseconds)
const unsigned long UPDATE_INTERVAL = 10000000; // 30 seconds (30,000,000 microseconds)

// Interrupt service routine for motion detection
void IRAM_ATTR motionDetectedISR()
{
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_EXT0);
}

void connectToWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/rtc_io.h> // Include RTC GPIO functions

void updateDatabase()
{
  // Update room occupancy status based on motion detection
  if (motionDetected)
  {
    isEmpty = false; // Set room to occupied if motion is detected
    Serial.println("Motion detected! Room is occupied.");
  }
  else
  {
    isEmpty = true; // Set room to empty if no motion is detected
    Serial.println("No motion detected. Room is empty.");
  }

  // Construct the URL to update the Firebase Realtime Database
  String url = String(DATABASE_URL) + "/rooms/" + ROOM_KEY + "/isEmpty.json?auth=" + API_KEY;

  // Initialize the HTTP client
  HTTPClient http;

  // Make a PUT request to the Firebase Database with the boolean value directly
  http.begin(url);                                    // Start connection to the URL
  http.addHeader("Content-Type", "application/json"); // Set content type to JSON

  // Send the PUT request with the boolean value directly
  int httpCode = http.PUT(isEmpty ? "true" : "false");

  // Check for a successful response
  if (httpCode == HTTP_CODE_OK)
  {
    Serial.println("Database updated successfully!");
    Serial.println("HTTP Response: " + String(httpCode));
  }
  else
  {
    Serial.println("Failed to update database:");
    Serial.println("HTTP Response Code: " + String(httpCode));
    Serial.println("Error: " + http.errorToString(httpCode));
  }

  // Close the HTTP connection
  http.end();

  motionDetected = false; // Reset motion detected flag
}

const int MOSFET_PIN = 12; // GPIO pin connected to the MOSFET

void setup()
{

  Serial.begin(115200);
  Serial.println("Initializing...");
  // connectToWiFi(); // Connect to Wi-Fi
  // updateDatabase();

  // Check wake-up cause
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0)
  {
    // Woke up due to PIR sensor
    Serial.println("EXT0 wake (motion) → skip WiFi and database update");
    motionDetected = true; // Set motion detected flag to true
    Serial.println(motionDetected);
    // Configure RTC GPIO to retain MOSFET state during deep sleep
    gpio_hold_dis(static_cast<gpio_num_t>(MOSFET_PIN)); // Disable hold on the MOSFET pin
    rtc_gpio_set_level((gpio_num_t)MOSFET_PIN, LOW);
    gpio_hold_en(static_cast<gpio_num_t>(MOSFET_PIN)); // Enable hold on the MOSFET pin

    Serial.println("Mosfet powered off...");
  }
  else if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER)
  {
    // Woke up due to timer
    Serial.println("TIMER wake → connect to WiFi and update database");
    connectToWiFi();  // Connect to Wi-Fi
    updateDatabase(); // Update the database

    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionDetectedISR, HIGH);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1); // Wake up on HIGH signal from PIR sensor

    gpio_hold_dis(static_cast<gpio_num_t>(MOSFET_PIN)); // Disable hold on the MOSFET pin
    rtc_gpio_set_level((gpio_num_t)MOSFET_PIN, HIGH);
    gpio_hold_en(static_cast<gpio_num_t>(MOSFET_PIN)); // Enable hold on the MOSFET pin

    // Re-enable the interrupt and wake-up source before entering deep sleep
  }
  else
  {
    // Cold boot: power on PIR and set up ISR
    Serial.println("Cold boot: enabling PIR.");

    // Wake up on HIGH signal from PIR sensor
    rtc_gpio_init((gpio_num_t)MOSFET_PIN);                                     // Initialize RTC GPIO
    rtc_gpio_set_direction((gpio_num_t)MOSFET_PIN, RTC_GPIO_MODE_OUTPUT_ONLY); // Set as output
    rtc_gpio_set_level((gpio_num_t)MOSFET_PIN, HIGH);
    gpio_hold_en(static_cast<gpio_num_t>(MOSFET_PIN)); // Enable hold on the MOSFET pin
    // Set the pin to LOW before deep sleep
    Serial.println("Mosfet powered...");
    // attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionDetectedISR, HIGH);
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);

    // // Re-enable the interrupt and wake-up source before entering deep sleep
  }

  // Configure timer wake-up
  esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL);

  // Enter deep sleep
  Serial.println("Entering deep sleep...");
  Serial.flush(); // Ensure all serial output is sent before deep sleep

  esp_deep_sleep_start();
}

void loop()
{
  // The ESP32 will not execute this loop because it will be in deep sleep.
}