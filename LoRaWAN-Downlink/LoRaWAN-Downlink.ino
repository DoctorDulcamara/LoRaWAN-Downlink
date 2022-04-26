#include <MKRWAN.h>
#include <Arduino_MKRENV.h>
#include "arduino_secrets.h"

LoRaModem modem;

void setup() {  
  Serial.begin(115200);
  while (!Serial);

  ENV.begin();
  // We will be using the LED on the Arduino board as output, so we
  // shall configure it with the line below.
  pinMode(LED_BUILTIN, OUTPUT);  
  
  // change this to your regional band (eg. US915, AS923, ...)
  // This will now initialise the modem so we can connect to the network.
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  
  if (modem.version() != ARDUINO_FW_VERSION) {
    Serial.println("Please make sure that the latest modem firmware is installed.");
    Serial.println("To update the firmware upload the 'MKRWANFWUpdate_standalone.ino' sketch.");
  }
  
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  Serial.println("Connecting...");
  // Here we join the network using 'over the air authorisation'.
  int connected = modem.joinOTAA(APP_EUI, APP_KEY);

  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  // The network works in such a way that it is only possible to
  // receive a downlink if a communication window has been opened
  // by sending an uplink. So, to receive downlinks, we need to send
  // a 'dummy' uplink to open the window. The default is 300 seconds
  // which is 5 minutes. Therefore, after the downlink has been queued,
  // you may have to wait that long to receive the downlink. For a
  // faster response, decrease the poll interval.  
  modem.minPollInterval(60); // Default is 300s
  Serial.println("Waiting for messages...");
}

void loop() {
  delay(60 * 1000); // Wait 60 secs before polling again. Change this delay if you change the poll interval.

  // The line below sends the 'dummy' data to open the downlink window.
  modem.poll();

  // On The Things Stack the RX1 window is 5s which is the earliest moment
  // to receive any downlink data. We add 1500ms to allow for the transmission
  // of the data to complete.
  delay(5000 + 1500);

  // Check to see if any downlink has been received.
  if (!modem.available()) {
    Serial.println("No downlink message received at this time.");    
    return;
  }

  // If there is downlink data, initialise a 64 byte variable 'dataBuffer' in which to store it.
  char dataBuffer[64];
  int i = 0;
  // Now store the data in the variable you have initialised.
  while (modem.available()) {
    dataBuffer[i++] = (char)modem.read();
  }

  // The lines below just print what we have received to the serial monitor.
  Serial.print("Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(dataBuffer[j] >> 4, HEX);
    Serial.print(dataBuffer[j] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Now we can act on the information. If the buffer contains the
  // number 1, we turn on the LED, and if it is 0, we turn the LED off.
  if(dataBuffer[0] == 1){
    digitalWrite(LED_BUILTIN, HIGH);
  } else if(dataBuffer[0] == 0) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
}
