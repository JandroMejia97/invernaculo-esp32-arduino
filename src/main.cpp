#include "UbidotsEsp32Mqtt.h"
#include <HardwareSerial.h>
#include "main.h"

#include "ubidots.h"
#include "uart.h"

void setup() {
  uartInit();
  ubidotsSetup();
}

void loop() {
  if (!clientIsConnected()) {
    clientReconnect();
  }

  clientLoop();
}
