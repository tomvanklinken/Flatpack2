/*
 * Flatpack2 Library Test
 */

#include <Flatpack2.h>


void onUpdate(int i)
{
  Serial.print("Received status from power supply ");
  Serial.print(Flatpack2::units[i].id);
  Serial.print(" (");
  Serial.print(Flatpack2::units[i].serialStr);
  Serial.print(")");

  Serial.print(" status=");
  Serial.print(Flatpack2::units[i].status);

  Serial.print(" intake_temp=");
  Serial.print(Flatpack2::units[i].intake_temp);

  Serial.print(" output_temp=");
  Serial.print(Flatpack2::units[i].output_temp);

  Serial.print(" input_voltage=");
  Serial.print(Flatpack2::units[i].input_voltage);

  Serial.print(" output_current=");
  Serial.print(Flatpack2::units[i].output_current);

  Serial.print(" output_voltage=");
  Serial.print(Flatpack2::units[i].output_voltage);

  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LOG_INFO("Flatpack2 example");

  // Set CAN options for your board here
#ifndef ARDUINO_ARCH_ESP32
  CAN.setClockFrequency(8E6); /* 8Mhz clock frequency for Arduino UNO */
#endif

  // Start communication with Flatpack2
  Flatpack2::onUpdate = &onUpdate;
  Flatpack2::Start();
}

void loop() {
  // Nothing
}
