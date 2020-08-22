#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>

Servo lock;

SoftwareSerial mySerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id; //for enrolling

#define SS_PIN 53
#define RST_PIN 5
#define BLYNK_PRINT Serial

const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
char keypressed;
char keymap[numRows][numCols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};


byte rowPins[numRows] = {22,23,24,25};
byte colPins[numCols] = {26,27,28,29};   


Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

LiquidCrystal_I2C lcd(0x27, 20, 4);

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 


String password="2580"; //Variable to store the current password
String tempPassword=""; //Variable to store the input password
int doublecheck;    //Check twice the new passoword
boolean armed = false;  //Variable for system state (armed:true / unarmed:false)
boolean input_pass;   //Variable for input password (correct:true / wrong:false)
boolean storedPassword = true;
boolean changedPassword = false;
boolean checkPassword = false;
int i = 1; //variable to index an array
int c = 0;

int code[] = {88,156,63,233}; //This is the stored UID
int codeRead = 0;
String uidString;

/*----------------------------------------------------------------------------*/

void setup(){
  pinMode(40,OUTPUT);
  
  pinMode(45,OUTPUT); //red led
  pinMode(44,OUTPUT); //green led
  
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  Serial.println("RFID DOOR LOCK");

  Serial.println("wassup");
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  
  lcd.begin(); //Setup the LCD's number of columns and rows 
  //Print welcome message...
  lcd.setCursor(0,0);
  lcd.print("MORTAL COMBAT");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("<3 kawaii");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" open doors ");
  lcd.setCursor(0,1);
  lcd.print(" with passcode  ");
  delay(2000);
  lcd.clear();
  lock.attach(9);
  lock.write(110);
  lcd.setCursor(2,0);
}

/*---------------------------------------MAIN LOOP-----------------------------------*/
void loop() 
{ //Main loop
  delay(500);
  if(  rfid.PICC_IsNewCardPresent())
  {
      readRFID();
  }
  unlockTheDoor();
  delay(500);
}
/*----------------------------------------------------------------------------------*/

/********************************FUNCTIONS*************************************/

void unlockTheDoor(){
  lockAgain: //goto label
  tempPassword="";
  lcd.clear();
  i=2;
  while(!checkPassword){
    lcd.setCursor(0,0);
    lcd.print("enter passcode:  ");
    lcd.setCursor(0,1);
    lcd.print(">");
    keypressed = myKeypad.getKey();   //Read pressed keys
    if(  rfid.PICC_IsNewCardPresent())
    {
      readRFID();
    }
    if (keypressed != NO_KEY){    //Accept only numbers and * from keypad
      if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
      keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
      keypressed == '8' || keypressed == '9' ){
        tempPassword += keypressed;
        lcd.setCursor(i,1);
        lcd.print("*");       //Put * on lcd
        i++;
      }
      else if (keypressed == 'A')
      {
        changePassword();
        goto lockAgain;
      }
      else if (keypressed=='#')
      {
        break;
      }
      else if (keypressed == '*')
      {  //Check for password
        if (password==tempPassword)
        {//If it's correct...
          c = 0;
          lcd.clear();      
          lcd.setCursor(0,0);
          lcd.print("Correct password");
          lcd.setCursor(0,1);
          lcd.print("Door UNLOCKED");
          lock.write(0);
          delay(5000);
          lock.write(110);
          goto lockAgain;
        }
        else{           //if it's false, retry
          c++;
          tempPassword="";
          lcd.clear();
          lcd.setCursor(0,0);
          if ( c == 3)
          {
            tone(40, 2000, 300);
            delay(4000);
            digitalWrite(40,LOW);
            lcd.setCursor(0,0);
            lcd.print("wait 3 mins");
            delay(180000);
            c = 0;
          }
          else if(c==2)
          {
            tone(40, 2000, 300);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("LAST TIME!!!");
            delay(1000);
            digitalWrite(40,LOW);
          }
          else
          {
            tone(40, 2000, 300);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("try again");
            delay(2000);
            digitalWrite(40,LOW);
          }
          goto lockAgain;
        }
      }
      else if (keypressed == 'B')
      {
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("FINGERPRINT");
        delay(1000);
        lcd.clear();
        getFingerprintIDez();
      }
      else if (keypressed == 'D')
      {
        finger.emptyDatabase(); //empty database of fingerprint
        Serial.println("database of fingerprint is empty");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("database is empty");
        delay(3000);
      }
      else if (keypressed == 'C')
      {
        Serial.println("you are about to enroll a fingerprint");
        getFingerprintEnroll();
      }
    }
  }
}

//Change current password
/////////////////////////////////////////////////////////CHANGE PASSWORD//////////////////////////////////////////////////////////////
void changePassword(){
  retry: //label for goto
  tempPassword="";
  lcd.clear();
  i=1;
  while(!changedPassword){        //Waiting for current password
    keypressed = myKeypad.getKey();   //Read pressed keys
    lcd.setCursor(0,0);
    lcd.print("CURRENT PASSWORD");
    lcd.setCursor(0,1);
    lcd.print(">");
    if (keypressed != NO_KEY){
      if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
      keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
      keypressed == '8' || keypressed == '9' ){
        tempPassword += keypressed;
        lcd.setCursor(i,1);
        lcd.print("*");
        i++;     
      }
      else if (keypressed=='#'){
        break;
      }
      else if (keypressed == '*'){
        i=1;
        if (password==tempPassword){
          storedPassword=false;
          newPassword();          //Password is corrent, so call the newPassword function
          break;
        }
        else{               //Try again
          tempPassword="";
          lcd.clear();
          lcd.setCursor(0,1);
          lcd.print("try again! XC");
          goto retry;
        }
      }
    }
  }
}
String firstpass;
//Setup new password
void newPassword(){
  tempPassword="";
  changedPassword=false;
  lcd.clear();
  i=1;
  while(!storedPassword){
    keypressed = myKeypad.getKey();   //Read pressed keys
    if (doublecheck==0){
      lcd.setCursor(0,0);
      lcd.print("set new password");
      lcd.setCursor(0,1);
      lcd.print(">");
    }
    else{
      lcd.setCursor(0,0);
      lcd.print("confirm pls :D");
      lcd.setCursor(0,1);
      lcd.print(">");
    }
    if (keypressed != NO_KEY){
      if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
      keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
      keypressed == '8' || keypressed == '9' ){
        tempPassword += keypressed;
        lcd.setCursor(i,1);
        lcd.print("*");
        i++;
      }
      else if (keypressed=='#'){
        break;
      }
      else if (keypressed == '*'){
        if (doublecheck == 0){
          firstpass=tempPassword;
          doublecheck=1;
          newPassword();
        }
        if (doublecheck==1){
          doublecheck=0;
          if (firstpass==tempPassword){
            i=1;
            firstpass="";
            password = tempPassword; // New password saved
            tempPassword="";//erase temp password
            lcd.setCursor(0,0);
            lcd.print("password changed XD");
            lcd.setCursor(0,1);
            lcd.print("----------------");
              storedPassword=true;
              delay(2000);
              lcd.clear();
              break;
          }
          else{
            tone(40, 2000, 300);
            firstpass="";
            newPassword();
          }
        }
      } 
    }
  }
}

///////////////////////////////////////////////////////////////UNLOCK DOOR WITH RFID TAG/////////////////////////////////////////////////////////////////////////////////

void readRFID()
{
  
  rfid.PICC_ReadCardSerial();
  Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("tag is not of type MIFARE Classic."));
    return;
  }

   
    Serial.println("Scanned UID:");
    printDec(rfid.uid.uidByte, rfid.uid.size);

    uidString = String(rfid.uid.uidByte[0])+" "+String(rfid.uid.uidByte[1])+" "+String(rfid.uid.uidByte[2])+ " "+String(rfid.uid.uidByte[3]);
    

    int i = 0;
    boolean match = true;
    while(i<rfid.uid.size)
    {
      if(!(rfid.uid.uidByte[i] == code[i]))
      {
           match = false;
      }
      i++;
    }

    if(match)
    {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("DOOR UNLOCKED!");
      Serial.println("\nI know this card!");
      delay(1000);
      lcd.clear();
      lock.write(0);
      delay(5000);
      lock.write(110);
    }else
    {
      tone(40, 2000, 300);
      lcd.clear();
      lcd.setCursor(1,1);
      lcd.print("WRONG CARD");
      delay(3000);
      lcd.clear();
      Serial.println("\nUnknown Card");
    }


    // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

//////////////////////////////////////////////////UNLOCK DOOR WITH FINGERPRINT/////////////////////////////////////////////////////////
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

uint8_t getFingerprintID() 
{
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  lock.write(100);
  delay(5000);
  lock.write(0);
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  while(true)
  {
    uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
  {
    tone(40, 2000, 300);
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("UNKNOWN FINGER");
    delay(2000);
    lcd.clear();
    return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
  {
    tone(40, 2000, 300);
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("UNKNOWN FINGER");
    delay(2000);
    lcd.clear();
    return -1;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
  {
    tone(40, 2000, 300);
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("UNKNOWN FINGER");
    delay(2000);
    lcd.clear();
    return -1;
  }
  
  // found a match!
  lock.write(0);
  delay(5000);
  lock.write(110);
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
  } 
}

/////////////////////////////////////////////////////////ENROLL NEW FINGERPRINTS/////////////////////////////////////////////////////////
uint8_t readnumber(void) 
{
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll() 
{
//
  Serial.println("Ready to enroll a fingerprint!");
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("READY TO ENROLL");
  lcd.setCursor(0,2);
  lcd.print("NEW FINGERPRINT");
  delay(3000);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("TYPE IN FINGER ID");
  lcd.setCursor(1,1);
  lcd.print(">");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
/////
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}
