#include <Adafruit_Thermal.h>
#include <HardwareSerial.h>
#include "lnbits_logo.h"
#include "quotes.h"

//THERMAL PRINTER SETTINGS
#define RXP 14  //define the GPIO connected TO the TX of the thermal printer
#define TXP 27   //define the GPIO connected TO the RX of the thermal printer

String qrData;

HardwareSerial printerSerial(1); // Declare HardwareSerial obj first
Adafruit_Thermal printer(&printerSerial);     // Pass addr to printer constructor

void setup() {
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup");

printerSerial.begin(9600, SERIAL_8N1, RXP, TXP);
printer.begin();

//  printer.wake();
//    printer.print("Hello people");
//    printer.sleep();

}

void loop() {
  Serial.println("Loop");
  printThing();
  delay(5000);
}

bool checkForError() {
  // Transmit the real-time transmission status command for the specified status
  const byte transmitStatusCommand[] = { 0x10, 0x04, 0x01 }; 
  const byte clearData[] = { 0x1B, 0x40 };  // clear data in buffer
  printerSerial.write(transmitStatusCommand, sizeof(transmitStatusCommand));

  delay(50);

  if (printerSerial.available()) {
    byte response = printerSerial.read();

    if (response != 0x12) {

      printerSerial.write(clearData, sizeof(clearData));
      return true;
    }
  }
  return false;  // No error condition detected
}

struct LookupTable {
  char character;
  byte value;
};

LookupTable PC437Characters[] = {
  { 'Ç', 0x80 }, { 'ü', 0x81 }, { 'é', 0x82 }, { 'â', 0x83 }, { 'ä', 0x84 }, { 'à', 0x85 }, { 'å', 0x86 }, { 'ç', 0x87 }, { 'ê', 0x88 }, { 'ë', 0x89 }, { 'è', 0x8A }, { 'ï', 0x8B }, { 'î', 0x8C }, { 'ì', 0x8D }, { 'Ä', 0x8E }, { 'Å', 0x8F }, { 'É', 0x90 }, { 'æ', 0x91 }, { 'Æ', 0x92 }, { 'ô', 0x93 }, { 'ö', 0x94 }, { 'ò', 0x95 }, { 'û', 0x96 }, { 'ù', 0x97 }, { 'ÿ', 0x98 }, { 'Ö', 0x99 }, { 'Ü', 0x9A }, { '¢', 0x9B }, { '£', 0x9C }, { '¥', 0x9D }, { '₧', 0x9E }, { 'ƒ', 0x9F }, { 'á', 0xA0 }, { 'í', 0xA1 }, { 'ó', 0xA2 }, { 'ú', 0xA3 }, { 'ñ', 0xA4 }, { 'Ñ', 0xA5 }, { 'ª', 0xA6 }, { 'º', 0xA7 }, { '¿', 0xA8 }, { '⌐', 0xA9 }, { '¬', 0xAA }, { '½', 0xAB }, { '¼', 0xAC }, { '¡', 0xAD }, { '«', 0xAE }, { '»', 0xAF }, { 'α', 0xE0 }, { 'ß', 0xE1 }, { 'Γ', 0xE2 }, { 'π', 0xE3 }, { 'Σ', 0xE4 }, { 'σ', 0xE5 }, { 'µ', 0xE6 }, { 'τ', 0xE7 }, { 'Φ', 0xE8 }, { 'Θ', 0xE9 }, { 'Ω', 0xEA }, { 'δ', 0xEB }, { '∞', 0xEC }, { 'φ', 0xED }, { 'ε', 0xEE }, { '∩', 0xEF }, { '≡', 0xF0 }, { '±', 0xF1 }, { '≥', 0xF2 }, { '≤', 0xF3 }, { '⌠', 0xF4 }, { '⌡', 0xF5 }, { '÷', 0xF6 }, { '≈', 0xF7 }, { '°', 0xF8 }, { '∙', 0xF9 }, { '·', 0xFA }, { '√', 0xFB }, { 'ⁿ', 0xFC }, { '²', 0xFD }
};

// Function to lookup byte value of a character in CodePage437
byte lookup(char character) {
  for (unsigned int i = 0; i < sizeof(PC437Characters) / sizeof(LookupTable); i++) {
    if (PC437Characters[i].character == character) {
      return PC437Characters[i].value;
    }
  }

  // Return a default value if character is not found in the lookup table
  return 0x00;
}


void printEncodedString(const String& str) {
  for (int i = 0; i < str.length(); i++) {
    char character = str.charAt(i);
    if (isAscii(character)) {
      printer.write(character);
    } else {
      byte encodedCharacter = lookup(character);
      printer.write(encodedCharacter);
    }
  }
}

void printQRcode(String qrData, byte size = 2, bool isMainQR = true) {
  // Using a smaller size or adjusting based on isMainQR
  byte adjustedSize = isMainQR ? size : max(1, size - 10); // Ensure minimum size of 1

  // Adjust error correction: L (0x31), M (0x32), Q (0x33), H (0x34)
  byte eccLevel = 0x31; // Start with low and increase if errors persist

  // Commands setup
  const byte modelCommand[] = { 0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x43, adjustedSize };
  const byte eccCommand[] = { 0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x45, eccLevel };
  const byte printCommand[] = { 0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x51, 0x30 };

  printerSerial.write(modelCommand, sizeof(modelCommand));
  printerSerial.write(eccCommand, sizeof(eccCommand));

  int len = qrData.length() + 3;
  if (len > 255) {
    // Simple error handling for data that is too large
    Serial.println("Data too long for a single QR code!");
    return;
  }
  byte dataCommand[] = { 0x1D, 0x28, 0x6B, (byte)len, 0x00, 0x31, 0x50, 0x30 };
  printerSerial.write(dataCommand, sizeof(dataCommand));
  printerSerial.print(qrData);

  printerSerial.write(printCommand, sizeof(printCommand));
}

void printThing() {
  printer.wake();

  printer.setDefault();

  
  printer.justify('C');  // Center align text
  printer.feed(3);
  
  printer.boldOn();
  printer.setSize('L');
  printer.println("Thank you");
  printer.feed(1);
  printer.setSize('S');
  printer.println("This voucher can be redeemed for 5.50 GBP of Bitcoin");
  printer.feed(1);
  printer.underlineOn();
  printer.println("Scan me to get your Bitcoin");
  printer.underlineOff();
  printer.feed(1);

  qrData = "https://demo.lnbits.com/lnurldevice/atm?lightning=LNURL1DP68GURN8GHJ7UMPW3EJUURH9AKXUATJD3CZ7DR9WFG525GHL45S8";

  printQRcode(qrData);
  printer.boldOff();
  printer.feed(1);
  printer.justify('L');
  printer.println(getRandomQuote());
  printer.feed(3);

  printer.sleep();

}
