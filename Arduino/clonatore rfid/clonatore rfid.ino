/*
 * Pin layout for SPI communication:
 * -----------------------------------------------------------------------------------------
 * Arduino Mega
 * Signal           Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset        5
 * SPI SS/SDA       53
 * SPI MOSI         51
 * SPI MISO         50
 * SPI SCK          52
 */

#include <SPI.h>
#include <MFRC522.h>

// Pin breadboard
#define GREENLED_PIN 9
#define REDLED_PIN 8
#define BUZZER_PIN 7

// Pin Rfid module
#define RFID_SS_PIN 53
#define RFID_RESET_PIN 5

// Misc
#define DATALEN 18        // a data block on the PICC is 18 bytes: 16 bytes of data, 2 bytes for CRC
#define DATABLOCK 1       // the block containing the secret data
#define LED_DELAY 1000    // how many ms the leds should stay on
#define BUZZER_DELAY 100  // how many ms the buzzer should stay on

MFRC522 mfrc522(RFID_SS_PIN, RFID_RESET_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;                       // Key used in the Crypto1 when communicating with the PICC
MFRC522::StatusCode status;                    // Struct used to indicate the current status of the RFID operations

bool waitingForOriginalCard;
byte readData[DATALEN];

void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 reader

  // Prepare the CRYPTO1 key - the default keys are FFFFFFFFFFFFh (set from the factory)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  pinMode(GREENLED_PIN, OUTPUT);    // The pin connected to the green led is an output (0-5 volt)
  pinMode(REDLED_PIN, OUTPUT);      // The pin connected to the red led is an output (0-5 volt)
  pinMode(BUZZER_PIN, OUTPUT);      // The pin connected to the active buzzer is an output (0-5 volt)
  digitalWrite(GREENLED_PIN, LOW);  // Turn off the green led
  digitalWrite(REDLED_PIN, HIGH);   // Turn on the red led
  digitalWrite(BUZZER_PIN, LOW);    // Turn off the active buzzer
  waitingForOriginalCard = true;    // This become false when the PICC has been read and it's ready to clone it in another PICC

  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("\nAVVIO CLONATORE\nAvvicinare il tag RFID da copiare"));
}

void loop() {
  if (waitingForOriginalCard) {
    loopReadData();  // Wait for a original PICC to be read
  } else {
    loopCloneData();  // Wait for an attacker PICC to be writable
  }
}

void loopReadData() {
  // Look for new cards, and select one if present
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(100);  // Wait a bit and then restart the loop
    return;
  }

  // Authentication with the CRYPTO1 key
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, DATABLOCK, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Autenticazione fallita: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read the secret bytes and save them in the `readData` array
  byte len = DATALEN;  // the MIFARE_Read function needs the address of the len, so we need to save it as a variable
  status = mfrc522.MIFARE_Read(DATABLOCK, readData, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lettura fallita: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  printReadData();

  // Now a card is selected. The UID and SAK are in `mfrc522.uid`
  // Turn on green led
  digitalWrite(GREENLED_PIN, HIGH);
  buzz(1, BUZZER_DELAY * 3);

  mfrc522.PICC_HaltA();       // Tell the PICC to stop until it's removed from the reader
  mfrc522.PCD_StopCrypto1();  // stop the authentication instance (or the card won't be readable anymore)

  waitingForOriginalCard = false;  // From the next loop: wait for the attacker PICC
  Serial.println(F("Avvicinare il tag RFID nel quale salvare il segreto clonato"));
}

void loopCloneData() {
  // Look for new cards, and select one if present
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(100);  // Wait a bit and then restart the loop
    return;
  }


  // Authentication with the CRYPTO1 key
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, DATABLOCK, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Autenticazione fallita: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write the cloned data
  status = mfrc522.MIFARE_Write(DATABLOCK, readData, DATALEN - 2);  // len is 18: 16 bytes for data, 2 bytes for CRC
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() fallito: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println(F("Tag RFID clonato correttamente. Riavvio...\n"));
  Serial.println(F("AVVIO CLONATORE\nAvvicinare il tag RFID da copiare"));

  // Turn off green led
  digitalWrite(GREENLED_PIN, LOW);
  buzz(3, BUZZER_DELAY);

  mfrc522.PICC_HaltA();       // Tell the PICC to stop until it's removed from the reader
  mfrc522.PCD_StopCrypto1();  // stop the authentication instance (or the card won't be readable anymore)
  waitingForOriginalCard = true;
}

void printReadData() {
  Serial.print(F("Segreto letto: "));
  for (int i = 0; i < DATALEN - 2; i++) {  // len is 18: 16 bytes for data, 2 bytes for CRC
    Serial.print((char)readData[i]);
  }
  Serial.print(" (");
  for (int i = 0; i < DATALEN - 2; i++) {  // len is 18: 16 bytes for data, 2 bytes for CRC
    Serial.print(readData[i] < 0x10 ? " 0" : " ");
    Serial.print(readData[i], HEX);
  }
  Serial.println(" )");
}

void buzz(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    delay(duration);
  }
}