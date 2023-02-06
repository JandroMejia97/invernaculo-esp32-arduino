#include "ubidots.h"
#include "uart.h"

#ifndef UART_NUMBER
  #warning "UART_NUMBER not defined, using default UART 2"
  #define UART_NUMBER 2
#endif
// Serial port pins
#ifndef SERIAL_RX
  #warning "SERIAL_RX not defined, using default pin 16"
  #define SERIAL_RX 16
#endif

#ifndef SERIAL_TX
  #warning "SERIAL_TX not defined, using default pin 17"
  #define SERIAL_TX 17
#endif

// Message received from the UART
String message;

// Hardware serial port to communicate with the EDU-CIAA
HardwareSerial SerialPort(UART_NUMBER);

void uartInit() {
  Serial.begin(115200);
  // Initialize the serial port
  SerialPort.begin(115200, SERIAL_8N1, SERIAL_RX, SERIAL_TX);
  // Set the callback function to be called when a message is received
  SerialPort.onReceive(uartHandler);
  // Enable the serial port to receive data
  // SerialPort.listen();
}

void uartHandler() {
  setStatus(ESP_READING_DATA);
   // Read the message from the UART
  int i = 0;
  Serial.println("Reading data from UART");
  while (SerialPort.available() && SerialPort.peek() != '\n') {
    message += (char) SerialPort.read();
    i++;
  }
  SerialPort.read();
  // Get the type of the variable, and convert it to an integer
  const int index = message[0] - '0';
  const int value = message.substring(1).toInt();
  // Publish the data to the Ubidots MQTT broker
  Serial.printf("Publishing data:\r\nIndex: %d, Value: %d\r\n", index, value);
  publishData(index, value);
  // Clear the message
  message = "";
}

void sendToUart(String message) {
  SerialPort.println(message);
}

void sendToUart(char *message) {
  SerialPort.println(message);
}