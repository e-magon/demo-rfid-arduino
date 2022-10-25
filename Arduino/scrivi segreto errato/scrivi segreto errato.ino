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

// Pin Rfid module
#define RFID_SS_PIN 53
#define RFID_RESET_PIN 5

// Misc
#define DATALEN 18   // a data block on the PICC is 18 bytes: 16 bytes of data, 2 bytes for CRC
#define DATABLOCK 1  // the block containing the secret data

MFRC522 mfrc522(RFID_SS_PIN, RFID_RESET_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;                       // Key used in the Crypto1 when communicating with the PICC
MFRC522::StatusCode status;                    // Struct used to indicate the current status of the RFID operations

byte data[] = "WRONG_SECRET_KEY";

void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 reader

  // Prepare the CRYPTO1 key - the default keys are FFFFFFFFFFFFh (set from the factory)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("\nSCRITTURA SEGRETO ERRATO\nAvvicinare il tag RFID da riscrivere"));
}

void loop() {
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
  status = mfrc522.MIFARE_Write(DATABLOCK, data, DATALEN - 2);  // len is 18: 16 bytes for data, 2 bytes for CRC
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() fallito: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  byte len = DATALEN;  // the MIFARE_Read function needs the address of the len, so we need to save it as a variable
  byte readData[DATALEN];
  status = mfrc522.MIFARE_Read(DATABLOCK, readData, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lettura fallita: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  printReadData(readData);

  Serial.println(F("Tag RFID scritto correttamente. Riavvio...\n"));
  Serial.println(F("SCRITTURA SEGRETO ERRATO\nAvvicinare il tag RFID da riscrivere"));

  mfrc522.PICC_HaltA();       // Tell the PICC to stop until it's removed from the reader
  mfrc522.PCD_StopCrypto1();  // stop the authentication instance (or the card won't be readable anymore)
}

void printReadData(byte readData[]) {
  Serial.print(F("Segreto scritto: "));
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