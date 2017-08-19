#include "MFRC522.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define RST_PIN  5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  D8 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 
const int correct_output_pin = D1;
const int incorrect_output_pin = D2;
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
const String allowed_card = String(" b5 43 d7 1b");

const char* ssid     = "";
const char* password = "";
const bool isAP = true;
IPAddress apIP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
const byte DNS_PORT = 53;

ESP8266WebServer webServer(80);
DNSServer dnsServer;

bool isCorrectCard = false;
bool isIncorrectCard = false;
String response = String("Hello!");


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(correct_output_pin, OUTPUT);
  pinMode(incorrect_output_pin, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);   // turn off LED with voltage HIGH
  digitalWrite(correct_output_pin, LOW);
  digitalWrite(incorrect_output_pin, LOW);
  Serial.begin(9600);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  if (isAP) {
      WiFi.mode(WIFI_AP);
      Serial.println("Cannot connect to WiFi network. Creating access point.");
      
      Serial.print("Setting soft-AP configuration ... ");
      Serial.println(WiFi.softAPConfig(apIP, gateway, subnet) ? "Ready" : "Failed!");

      Serial.print("Setting soft-AP ... ");
      Serial.println(WiFi.softAP("ESP8266 RFID Tester") ? "Ready" : "Failed!");

      Serial.print("Soft-AP IP address = ");
      Serial.println(WiFi.softAPIP());

      Serial.print("Starting DNS server for captive portal...");
      dnsServer.start(DNS_PORT, "*", apIP);
  } else {
    WiFi.begin(ssid, password);

    int wifi_timer = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      wifi_timer++;
    }
    Serial.println(WiFi.localIP());
  }

  webServer.onNotFound([]() {
    webServer.send(200, "text/html", response);
  });
  webServer.begin();
}

void loop() {
  isCorrectCard = false;
  isIncorrectCard = false;
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
  } else {
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      delay(50);
      return;
    }
    // Show some details of the PICC (that is: the tag/card)
    String picc_uid = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.print(F("Card UID:"));
    Serial.print(picc_uid);
    Serial.println();
    response = picc_uid;

    isCorrectCard = (picc_uid  == allowed_card);
    isIncorrectCard = !isCorrectCard;
  }

  if (isCorrectCard) {
    digitalWrite(BUILTIN_LED, LOW);  // turn on LED with voltage LOW
    digitalWrite(correct_output_pin, HIGH);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage LOW
    digitalWrite(correct_output_pin, LOW);
  }

  if (isIncorrectCard) {
    digitalWrite(BUILTIN_LED, LOW);  // turn on LED with voltage LOW
    digitalWrite(incorrect_output_pin, HIGH);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage LOW
    digitalWrite(incorrect_output_pin, LOW);
  }

  dnsServer.processNextRequest();
  webServer.handleClient();
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

void showWebpage() {
  webServer.send(200, "text/html", " <meta http-equiv=\"refresh\" content=\"1\" />" + response);
}
