/*
utils.ino, part of StratoSoar MK2, for an autonomous glider.
Copyright (C) 2024 Charles Nicholson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "src/settings.h"
#include <Arduino.h>

double toRadians(double deg) {
  return deg * pi / 180.0;
}

double toDegrees(double rad) {
  return rad * 180.0 / pi;
}

double azimuth(double cLat, double cLon, double tLat, double tLon) {
  double dLon = toRadians(tLon - cLon);
  double dPhi = log(tan(toRadians(tLat) / 2 + pi / 4) / tan(toRadians(cLat) / 2 + pi / 4));
  if (fabs(dLon) > pi) {
    dLon = dLon > 0 ? -(2 * pi - dLon) : (2 * pi + dLon);
  }
  return fmod((atan2(dLon, dPhi) + 2 * pi), (2 * pi));
}

double turningAngle(double cLat, double cLon, double head, double tLat, double tLon) {
  double a = azimuth(cLat, cLon, tLat, tLon);
  double h = toRadians(head);
  double angle = a - h;
  if (angle < -pi)
    angle += 2 * pi;
  if (angle > pi)
    angle -= 2 * pi;
  return toDegrees(angle);
}

// Haversine formula to calculate distance between two coordinates.
double calculateDistance(double cLat, double cLon, double tLat, double tLon) {
  double dLat = toRadians(tLat - cLat);
  double dLon = toRadians(tLon - cLon);
  double a = sin(dLat / 2) * sin(dLat / 2) + cos(toRadians(tLat)) * cos(toRadians(tLat)) * sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double distance = 6371000 * c;
  return distance;
}

int pidMagicElevator() {
  errorElevator = SETPOINT_ELEVATOR - data.pitch; // Calculate the error.

  int outputElevator = KP_ELEVATOR * errorElevator + KI_ELEVATOR * integralElevator + KD_ELEVATOR * (errorElevator - prevErrorElevator); // Calculate the output.

  int servoPositionElevator2 = 90 - outputElevator; // Adjust servo position based on the output.

  // Update previous error and integral.
  prevErrorElevator = errorElevator;
  integralElevator += errorElevator;

  // int servoPositionElevatorNew = map(servoPositionElevator2, 0, 180, 750, 2250); // For servos that take microseconds not degrees.

  return servoPositionElevator2;
}

int pidMagicRudder() {
  setpointRudder = data.yaw + data.turnAngle;
  inputRudder = data.yaw;

  errorRudder = setpointRudder - data.yaw; // Calculate the error.

  int outputRudder = KP_RUDDER * errorRudder + KI_RUDDER * integralRudder + KD_RUDDER * (errorRudder - prevErrorRudder); // Calculate the output.

  int servoPositionRudder = 180 + outputRudder; // Adjust servo position based on the output.

  // Update previous error and integral.
  prevErrorRudder = errorRudder;
  integralRudder += errorRudder;

  // int servoPositionRudder = map(servoPositionRudder, 0, 180, 1250, 1750); // For servos that take microseconds not degrees.

  return servoPositionRudder / 2;
}

void longPulse(int pin, int sleep = 1) { // Pass 1 to sleep, 0 to delay.
  digitalWrite(pin, HIGH);
  if (sleep == 1) {
#ifdef LOW_POWER
    LowPower.sleep(250);
#endif
#ifndef LOW_POWER
    delay(250);
#endif
  } else {
    delay(250);
  }
  digitalWrite(pin, LOW);
  if (sleep == 1) {
#ifdef LOW_POWER
    LowPower.sleep(250);
#endif
#ifndef LOW_POWER
    delay(500);
#endif
  } else {
    delay(500);
  }
  digitalWrite(pin, HIGH);
  if (sleep == 1) {
#ifdef LOW_POWER
    LowPower.sleep(250);
#endif
#ifndef LOW_POWER
    delay(250);
#endif
  } else {
    delay(250);
  }
  digitalWrite(pin, LOW);
}

void shortPulse(int pin) {
  digitalWrite(pin, HIGH);
#ifdef LOW_POWER
  LowPower.sleep(10);
#endif
#ifndef LOW_POWER
  delay(10);
#endif
  digitalWrite(pin, LOW);
}

#ifdef NEED_RUDDER
void moveRudder(int degrees, int sleep) {
  digitalWrite(RUDDER_FET, HIGH); // Turn servo on.
  digitalWrite(RUDDER_BJT, LOW);  // Turn signal line on.
  int rudderLast = rudderServo.read();
  int rudderChange = abs(rudderLast - degrees);
  int rudderTime = rudderChange * 170 / 60;
  if (rudderTime < 40) {
    rudderTime = rudderTime + 40;
  }
  if (rudderTime < 60) {
    rudderTime = rudderTime + 20;
  }
  rudderTime = 200;
  rudderServo.write(degrees);
#ifndef SERVO_NONBLOCKING
  if (sleep == 1) {
#ifdef LOW_POWER
    LowPower.sleep(rudderTime);
#endif
#ifndef LOW_POWER
    delay(rudderTime);
#endif
  } else {
    delay(rudderTime);
  }
  digitalWrite(RUDDER_BJT, HIGH);
  digitalWrite(RUDDER_FET, LOW);
#endif
#ifdef SERVO_NONBLOCKING
  // nonBlockingDelay(rudderTime, turnOffRudder);
  nonBlockingDelay(300, turnOffRudder);
#endif
}
#endif

void turnOffRudder() {
  digitalWrite(RUDDER_BJT, HIGH);
  digitalWrite(RUDDER_FET, LOW);
}

#ifdef NEED_ELEVATOR
void moveElevator(int degrees, int sleep) {
  digitalWrite(ELEVATOR_FET, HIGH); // Turn servo on.
  digitalWrite(ELEVATOR_BJT, LOW);  // Turn signal line on.
  elevatorServo.write(degrees);
  if (sleep == 1) {
#ifdef LOW_POWER
    LowPower.sleep(300);
#endif
#ifndef LOW_POWER
    delay(300);
#endif
  } else {
    delay(300);
  }
  digitalWrite(ELEVATOR_BJT, HIGH);
  digitalWrite(ELEVATOR_FET, LOW); // servo.detach() saves ~75 mA per servo. MOSFET saves an  additional ~4 mA per servo.
}
#endif

void readVoltage() {
  int rawVolt = analogRead(BAT_VOLTAGE_PIN);
  rawVolt = rawVolt * 2;
  data.volts = rawVolt * (3.3 / 1023.0);
}

void I2CScan() {
  int nDevices = 0;

#ifdef DEVMODE
  SerialUSB.println("Scanning...");
#endif

  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
#ifdef DEVMODE
      SerialUSB.print("I2C device found at address 0x");
      if (address < 16) {
        SerialUSB.print("0");
      }
      SerialUSB.print(address, HEX);
      SerialUSB.println("!");
#endif
      ++nDevices;
    } else if (error == 4) {
#ifdef DEVMODE
      SerialUSB.print("Unknown error at address 0x");
      if (address < 16) {
        SerialUSB.print("0");
      }
      SerialUSB.println(address, HEX);
#endif
    }
  }
#if (!defined(TEST_COORD) && !defined(USE_EEPROM)) || (defined(TEST_COORD) && defined(USE_EEPROM))
  if (nDevices >= 1) {
#ifdef DEVMODE
    SerialUSB.println("All I2C devices found successfully.");
#endif
  } else {
#ifdef DEVMODE
    SerialUSB.println("One I2C device is not responding.");
#endif
    longPulse(ERR_LED);
  }
#endif
#if !defined(TEST_COORD) && defined(USE_EEPROM)
  if (nDevices == 2) {
#ifdef DEVMODE
    SerialUSB.println("All I2C devices found successfully.");
#endif
  } else {
#ifdef DEVMODE
    SerialUSB.println("One or more I2C devices are not responding.");
#endif
    longPulse(ERR_LED);
  }
#endif
#if defined(TEST_COORD) && !defined(USE_EEPROM) && defined(DEVMODE)
  SerialUSB.println("Whether or not any devices were found, no I2C devices are needed.");
#endif
}

void streamData() {
  // Used for debugging the communication between the SAMD and the ATMega.
  digitalWrite(WRITE_PIN, HIGH);
  delay(40);
  digitalWrite(WRITE_PIN, LOW);
  delay(100);

  int available = Serial1.available();
  SerialUSB.print(available);
  SerialUSB.print(" available bytes ");
  while (Serial1.available() > 0) {
    SerialUSB.print(Serial1.read());
  }
}

void getIMUData() {
  Serial1.flush();

  // Ask ATMega for data.
  digitalWrite(WRITE_PIN, HIGH);
#ifdef LOW_POWER
  LowPower.sleep(40);
#endif
#ifndef LOW_POWER
  delay(40);
#endif
  digitalWrite(WRITE_PIN, LOW);

  // Wait a little bit before reading the data.
  // 20 bytes equal 160 bits. 1 stop + start bit for every 8 bits, so 40 stop and start bits equal 200 bits. 9600/200 =~ 20 ms. Add 10 ms for safety.
#ifdef LOW_POWER
  LowPower.sleep(200);
#endif
#ifndef LOW_POWER
  delay(50);
#endif

  if (Serial1.available()) {
    Serial1.readBytes((byte *)&receivedData, sizeof(receivedData));
    data.yaw = receivedData.yaw;
    data.pitch = receivedData.pitch;
    data.roll = receivedData.roll;
    data.temp = receivedData.temp / 100;         // In Celsius.
    data.humidity = receivedData.humidity / 100; // In relative humidity.
    data.pressure = receivedData.pressure / 100; // In hPa.
  } else {
#ifdef DEVMODE
    SerialUSB.println("We got nothing :(");
    SerialUSB.print("Number of bytes in buffer: ");
    SerialUSB.println(Serial1.available());
#endif
    shortPulse(ERR_LED);
  }
}

void calculate() {
  data.turnAngle = turningAngle(data.lat, data.lon, data.yaw, targetLat, targetLon);

  data.distanceMeters = calculateDistance(data.lat, data.lon, targetLat, targetLon); // Find the distance between the current location and the target.

  data.servoPositionElevator = pidMagicElevator(); // Change PID values in "settings.h" if you want.
  data.servoPositionRudder = pidMagicRudder();     // Change PID values in "settings.h" if you want.

  readVoltage();
}

void getGPSData() {
  data.lat = gps.getLatitude();
  data.lat = data.lat / 10000000;
  data.lon = gps.getLongitude();
  data.lon = data.lon / 10000000;
  data.alt = gps.getAltitude();
  data.alt = data.alt / 1000;
  data.sats = gps.getSIV();
  data.fixType = gps.getFixType();
  data.speed = gps.getGroundSpeed();
  data.seconds = gps.getSecond();
  data.minutes = gps.getMinute();
  data.hours = gps.getHour();
  data.day = gps.getDay();
  data.month = gps.getDay();
  data.year = gps.getYear();
  data.hdop = gps.getDOP();
}

void displayData() {
  SerialUSB.print("Lat: ");
  SerialUSB.print(data.lat, 9);
  SerialUSB.print(" Lon: ");
  SerialUSB.print(data.lon, 9);
  SerialUSB.print(" Alt: ");
  SerialUSB.print(data.alt);
  SerialUSB.print(" Sats: ");
  SerialUSB.print(data.sats);
  SerialUSB.print(" Fix Type: ");
  SerialUSB.print(data.fixType);
  SerialUSB.print(" HDOP: ");
  SerialUSB.print(data.hdop);
  SerialUSB.print(" Speed: ");
  SerialUSB.print(data.speed);
  SerialUSB.print(" Date/Time: ");
  SerialUSB.print(data.year);
  SerialUSB.print("-");
  SerialUSB.print(data.month);
  SerialUSB.print("-");
  SerialUSB.print(data.day);
  SerialUSB.print(" ");
  SerialUSB.print(data.hours);
  SerialUSB.print(":");
  SerialUSB.print(data.minutes);
  SerialUSB.print(":");
  SerialUSB.print(data.seconds);
  SerialUSB.print(" Yaw: ");
  SerialUSB.print(data.yaw);
  SerialUSB.print(" Last Yaw: ");
  SerialUSB.print(lastYaw);
  SerialUSB.print(" Yaw Difference: ");
  SerialUSB.print(yawDifference);
  SerialUSB.print(" Pitch: ");
  SerialUSB.print(data.pitch);
  SerialUSB.print(" Roll: ");
  SerialUSB.print(data.roll);
  SerialUSB.print(" Temp: ");
  SerialUSB.print(data.temp);
  SerialUSB.print(" Pressure: ");
  SerialUSB.print(data.pressure);
  SerialUSB.print(" Humidity: ");
  SerialUSB.print(data.humidity);
  SerialUSB.print(" Elevator Pos: ");
  SerialUSB.print(data.servoPositionElevator);
  SerialUSB.print(" Rudder Pos: ");
  SerialUSB.print(data.servoPositionRudder);
  SerialUSB.print(" Turn Angle: ");
  SerialUSB.print(data.turnAngle);
  SerialUSB.print(" Distance to Target: ");
  SerialUSB.print(data.distanceMeters);
  SerialUSB.print(" Voltage: ");
  SerialUSB.println(data.volts);
}

void waitForFix() {
  data.sats = 0;
  data.fixType = 0;
  while ((data.fixType < 3) && (data.sats < 4)) { // Make sure location is valid before continuing. (Changed data.sats from < 5 to < 3).
    if (gps.getPVT()) {
      getGPSData();
#ifdef DEVMODE
      SerialUSB.println("Waiting for fix...");
      displayData();
#endif
    } else {
#ifdef DEVMODE
      SerialUSB.println("No PVT data received. Retrying...");
#endif
    }
  }
}

void onWake() {
  // Do nothing, as this will never happen.
}

void gpsWakeup() {
  digitalWrite(WAKEUP_PIN, LOW);
#ifdef LOW_POWER
  LowPower.sleep(1000);
#endif
#ifndef LOW_POWER
  delay(1000);
#endif
  digitalWrite(WAKEUP_PIN, HIGH);
#ifdef LOW_POWER
  LowPower.sleep(1000);
#endif
#ifndef LOW_POWER
  delay(1000);
#endif
  digitalWrite(WAKEUP_PIN, LOW);
}

void gpsConfig() {
  // gps.factoryDefault();                                                        // Clear any saved configuration. Also calls hardReset(), so all the ephemeris data is lost.
  if (gps.setDynamicModel(DYN_MODEL_AIRBORNE1g, VAL_LAYER_RAM_BBR) == false) { // Set the dynamic model to airborne mode with one g of thrust allowance.
#ifdef DEVMODE
    SerialUSB.println(F("*** Warning: setDynamicModel failed ***"));
#endif
    longPulse(ERR_LED);
  } else {
#ifdef DEVMODE
    SerialUSB.println(F("Dynamic platform model changed successfully!"));
#endif
  }
}

void writeFloatToEEPROM(int address, float value) {
  byte *p = (byte *)(void *)&value; // Pointer to the float value
  for (int i = 0; i < sizeof(value); i++) {
    eeprom.write(address + i, *p++);
  }
  // Pad remaining bytes with 0
  for (int i = sizeof(value); i < 4; i++) {
    eeprom.write(address + i, 0);
  }
}

void nonBlockingDelay(unsigned long interval, void (*callback)()) {
  unsigned long currentMillis = millis(); // Get the current time.
  unsigned long previousMillis;

  // Check if the interval has passed.
  if (currentMillis - previousMillis >= interval) {
    // Save the current time as the last update time
    previousMillis = currentMillis;

    // Execute the callback function.
    callback();
  }
}

// Altitude in meters.
float bme280Altitude(float referencePressure) {
  return ((float)-45846.2) * (pow(((float)(data.pressure) / (float)referencePressure), 0.190263) - (float)1); // referencePressure is the pressure in hPa at zero altitude; for example, 1013.250.
}