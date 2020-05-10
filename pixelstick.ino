#include <Adafruit_NeoPixel.h>
#include <ArduinoSort.h>

#include <SPI.h>
#include <SD.h>

#define DELAY 80
#define BRIGHTNESS 224
#define LED_PIN 4
#define NUM_PIXELS 144

Adafruit_NeoPixel *pixels;

File root;
File dataFile;

int filePos = 0;
int numFiles = 0;

bool runAnimation = false;

String fileNames[10];
String currentFilename = "";

void setup() {
  Serial.begin(115200);
  //pinMode(0, INPUT_PULLUP);
  //pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pixels = new Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
  pixels->begin();
  pixels->setBrightness(BRIGHTNESS);
  setupSDcard();
  delay(100);
}

void loop() {

  for (int i = 0; i < (filePos + 1); i++) {
    int j = i * 5;
    if (i % 2 == 0) {
      pixels->setPixelColor(j, pixels->Color(0, 255, 0));
    } else {
      pixels->setPixelColor(j, pixels->Color(0, 0, 255));
    }
    pixels->show();
  }

  bool buttonLeft = !digitalRead(3);
  bool buttonRight = !digitalRead(2);

  if (buttonLeft) {
    if (filePos < numFiles) {
      filePos++;
    } else {
      filePos = 0;
    }
    Serial.print(filePos);
    Serial.print("\t");
    Serial.println(fileNames[filePos]);
    delay(200);
  }

  if (buttonRight) {
    Serial.print("Play!");
    sendFile(fileNames[filePos]);
  }
}

void sendFile(String filename) {

  char temp[14];

  filename.toCharArray(temp, 14);
  dataFile = SD.open(temp);

  if (dataFile) {
    int i = 0;
    int r, g, b;

    delay(5000);

    while (dataFile.available()) {

      bool buttonRight = !digitalRead(2);

      if (buttonRight) {
        Serial.print("Stop!");
        pixels->clear();
        pixels->show();
        dataFile.close();
        delay(200);
        break;
      }

      if (i == (pixels->numPixels())) {
        i = 0;
        pixels->show();
        delay(DELAY);
      }

      r = dataFile.parseInt();
      g = dataFile.parseInt();
      b = dataFile.parseInt();

      pixels->setPixelColor(i, r, g, b);

      i++;
    }
    
  } else {
    Serial.println("Error reading");
    setupSDcard();
    return;
  }
}

void setupSDcard() {
  pinMode(BUILTIN_SDCARD, OUTPUT);
  while (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD init failed!");
    delay(500);
  }
  Serial.println("SD init done");
  delay(1000);
  root = SD.open("/");
  Serial.println("Scanning files…");
  delay(500);
  getFileNamesFromSD(root);
}

void getFileNamesFromSD(File dir) {
  int fileCount = 0;
  String CurrentFilename = "";
  while (1) {
    File entry =  dir.openNextFile();
    if (!entry) {
      numFiles = fileCount;
      sortArray(fileNames, numFiles);
      Serial.print(fileCount);
      Serial.println(" files found!");
      Serial.println("Names: ");
      for (int i = 0; i < numFiles; i++) {
        Serial.println(fileNames[i]);
      }
      entry.close();
      break;
    } else {
      CurrentFilename = entry.name();
      if (CurrentFilename.endsWith(".TXT") || CurrentFilename.startsWith("_")) {
        fileNames[fileCount] = entry.name();
        fileCount++;
      }
    }
    entry.close();
  }
}
