/*
loopback..ino, part of StratoSoar MK2, for an autonomous glider.
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

// Use this code to test the serial on the SAMD.

// https://github.com/crnicholson/StratoSoar-MK2/.

#define BAUD_RATE 9600

void setup() {
  SerialUSB.begin(115200);
  while (!SerialUSB) {
  }
  delay(2000);
  SerialUSB.println("SerialUSB is ready.");
  Serial1.begin(BAUD_RATE);
  while (!Serial1) {
  }
  SerialUSB.println("Serial1 is ready.");
}

void loop() {
  Serial1.println("testing serial.");

  if (Serial1.available()) {
    String incomingData = Serial1.readStringUntil('\n');
    SerialUSB.print("Received via serial: ");
    SerialUSB.println(incomingData);
  }

  delay(1000);
}
