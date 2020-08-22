
#define BLYNK_PRINT Serial


#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <Servo.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "b08f9992c0fe47b2914c04c22bb7779a";

// Your WiFi credentials.
// Set password to "" for open networks.

//char ssid[] = "penpleum1";
//char pass[] = "12345678901";

//char ssid[] = "Samorn-SmartHome";
//char pass[] = "raspberrypi1234";

char ssid[] = "iPad";
char pass[] = "shit1234";

// Hardware Serial on Mega, Leonardo, Micro...
#define EspSerial Serial1

// or Software Serial on Uno, Nano...
//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

Servo servo;

BLYNK_WRITE(V3)
{
  servo.write(param.asInt());
  Blynk.notify("door opened");
}

void setup()
{
  // Debug console
  pinMode(45,OUTPUT); //red led
  pinMode(44,OUTPUT); //green led
  Serial.begin(9600);

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  Blynk.begin(auth, wifi, ssid, pass);


  servo.attach(9);
}

void loop()
{
  Blynk.run();
}
