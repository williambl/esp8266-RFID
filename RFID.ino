#include "MFRC522.h"
#define RST_PIN  5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  D8 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 
#define OUTPUT_PIN D1
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
const String allowed_card = String(" b5 43 d7 1b");

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);   // turn off LED with voltage HIGH
  digitalWrite(OUTPUT_PIN, HIGH);
  Serial.begin(9600);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
}

void loop() { 
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    digitalWrite(BUILTIN_LED, HIGH);   // turn off LED with voltage HIGH
    digitalWrite(OUTPUT_PIN, HIGH);
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    digitalWrite(BUILTIN_LED, HIGH);   // turn off LED with voltage HIGH
    digitalWrite(OUTPUT_PIN, HIGH);
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  String picc_uid = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.print(F("Card UID:"));
  Serial.print(picc_uid);
  Serial.println();

  if (picc_uid  == allowed_card) {
    digitalWrite(BUILTIN_LED, LOW);  // turn on LED with voltage LOW
    digitalWrite(OUTPUT_PIN, LOW);
  }
}

// Helper routine to dump a byte array as hex values to a string
String dump_byte_array(byte *buffer, byte bufferSize) {
  String string;
  for (byte i = 0; i < bufferSize; i++) {
    string += buffer[i] < 0x10 ? " 0" : " ";
    string += String(buffer[i], HEX);
  }
  return string;
}
