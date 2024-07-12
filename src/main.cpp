#include <Arduino.h>

void setup()
{
  Serial.begin();
  // No matter what is set. Measured 256k = ~2^18 ASCII characters per second
  Serial.setDebugOutput(true);

  while (!Serial)
    ; // If you want to wait for a USB connection to be established

  // put your setup code here, to run once:

  log_d("setup done");           // for debugging purposes
  Serial.printf("setup done\n"); // writes "setup done" to the usb interface
}

void loop()
{
  // put your main code here, to run repeatedly:
}