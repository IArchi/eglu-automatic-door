/*******************************/
/*     VENDORS DEPENDENCIES    */
/*******************************/
// Install library "Low-Power" made by Rocket Scream Electronics
// If you get troubles uploading the code to the Nano, go to Tools and swicth Processor to Old Bootloader

/*******************************/
/*   CONFIGURATION VARIABLES   */
/*******************************/
#define DEBUG                           // Uncomment to enable logs (Might be required to calibrate the light sensor and the servo angles)
#define SERVO_PIN                 9     // You probably don't need to change this value
#define LIGHT_SENSOR_ANALOG_PIN   A0    // You probably don't need to change this value
#define LOW_LIGHT_THRESHOLD       500   // Maximum light until closing
#define HIGH_LIGHT_THRESHOLD      210   // Minimum light before opening
#define LOW_LIGHT_MINUTES         15    // Minutes before closing the door
#define HIGH_LIGHT_MINUTES        15    // Minutes before opening the door
#define SERVO_OPEN_ANGLE          25    // The angle at which the door is considered as open
#define SERVO_CLOSED_ANGLE        140   // The angle at which the door is considered as open
#define SERVO_PULSE_WDITH_MIN     500   // Based on SF3218MG specifications
#define SERVO_PULSE_WDITH_MAX     2500  // Based on SF3218MG specifications

/*****************************************************************************************/
/*                              DO NOT EDIT BELOW THIS LINE                              */
/*****************************************************************************************/

/*******************************/
/*     Import dependencies     */
/*******************************/
#include <Servo.h>
#ifndef DEBUG
  #include "LowPower.h"
#endif

/*******************************/
/*       Global variables      */
/*******************************/
enum DoorState { OPEN, CLOSED, UNKNOWN };
int low_light_count = 0;
int high_light_count = 0;
DoorState door_state = UNKNOWN;
Servo door_servo;

/*******************************/
/*        Setup function       */
/*******************************/
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("-------------------");
    Serial.println("|  Eglu is ready  |");
    Serial.println("-------------------");
  #endif

  // Set default motor position to OPEN
  door_servo.write(SERVO_OPEN_ANGLE);
  door_servo.attach(SERVO_PIN, SERVO_PULSE_WDITH_MIN, SERVO_PULSE_WDITH_MAX);
  delay(500);
  door_servo.detach();
  door_state = OPEN;

  // Enable builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

/*******************************/
/*    Infinite loop function   */
/*******************************/
void loop() {
  // LED indicates the program is running
  digitalWrite(LED_BUILTIN, HIGH);

  // Read LM393 light sensor
  int16_t current_light_level = analogRead(LIGHT_SENSOR_ANALOG_PIN);
  set_light_level(current_light_level);

  // Check if the light level has been the same during X minutes
  if (high_light_count >= HIGH_LIGHT_MINUTES) {
    if (door_state != OPEN) {
      #ifdef DEBUG
        Serial.println("> Opening door");
      #endif

      // Open the Door
      open_door();
    }
    else {
      #ifdef DEBUG
        Serial.println("> Door was already open");
      #endif
    }
  }
  else if (low_light_count >= LOW_LIGHT_MINUTES) {
    if (door_state != CLOSED) {
      #ifdef DEBUG
        Serial.println("> Closing door");
      #endif

      // Close the Door
      close_door();
    }
    else {
      #ifdef DEBUG
        Serial.println("> Door was already closed");
      #endif
    }
  }
  
  // Wait 500ms before turning LED off (Code is too fast otherwise)
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);

  
  #ifdef DEBUG
    // In DEBUG mode, light level is read every second (0.5+0.5)
    delay(500);
  #else
    // Power down the board to save battery
    // Light level is read every minute
    //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,SPI_OFF, USART0_OFF, TWI_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  #endif
}

/*******************************/
/*  Sensor and Servo functions */
/*******************************/
void set_light_level(int16_t level) {
  #ifdef DEBUG
    Serial.println("----------");
    Serial.print("Light sensor: ");
    Serial.println(level);
  #endif

  // Check if thresholds have been reached
  if (level >= LOW_LIGHT_THRESHOLD) {
    high_light_count = 0;
    if (low_light_count < LOW_LIGHT_MINUTES) low_light_count = low_light_count + 1;

    #ifdef DEBUG
      Serial.print("Low light minutes: ");
      Serial.print(low_light_count);
      Serial.print("/");
      Serial.println(LOW_LIGHT_MINUTES);
    #endif
  }
  else if (level <= HIGH_LIGHT_THRESHOLD) {
    low_light_count = 0;
    if (high_light_count < HIGH_LIGHT_MINUTES) high_light_count = high_light_count + 1;

    #ifdef DEBUG
      Serial.print("High light minutes: ");
      Serial.print(high_light_count);
      Serial.print("/");
      Serial.println(HIGH_LIGHT_MINUTES);
    #endif
  }
}
void open_door() {
  // Move servo from 160 to 25 degrees
  door_servo.attach(SERVO_PIN, SERVO_PULSE_WDITH_MIN, SERVO_PULSE_WDITH_MAX);
  for (int pos=SERVO_CLOSED_ANGLE; pos>=SERVO_OPEN_ANGLE; pos -= 1) {
    door_servo.write(pos);
    delay(50);
  }
  door_servo.detach();
  door_state = OPEN;
}
void close_door() {
  // Move servo from 25 to 160 degrees
  door_servo.attach(SERVO_PIN, SERVO_PULSE_WDITH_MIN, SERVO_PULSE_WDITH_MAX);
  for (int pos=SERVO_OPEN_ANGLE; pos<=SERVO_CLOSED_ANGLE; pos += 1) {
    door_servo.write(pos);
    delay(50);
  }
  door_servo.detach();
  door_state = CLOSED;
}
