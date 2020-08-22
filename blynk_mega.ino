
#define BLYNK_PRINT Serial


#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

char auth[] = "9HOWDzqDjl5nkBj8Q_ZqbP1TIJpz1qkT";

// Your WiFi credentials.

//char ssid[] = "penpleum1";
//char pass[] = "12345678901";

char ssid[] = "iPhone";
char pass[] = "ink123456789";

// Hardware Serial on Mega, Leonardo, Micro...
//#define EspSerial Serial1

// or Software Serial on Uno, Nano...
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10, 11); // RX, TX

#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

void setup()
{
  // Debug console
  Serial.begin(115200);

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  Blynk.begin(auth, wifi, ssid, pass);
}

void loop()
{
  Blynk.run();
}
