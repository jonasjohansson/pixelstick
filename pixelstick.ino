#include <Adafruit_NeoPixel.h>
#include <ArduinoSort.h>

#include <SPI.h>
#include <SD.h>

#define DELAY 80
#define BRIGHTNESS 200
#define LED_PIN 2
#define NUM_PIXELS 144
#define BTN_UP 19
#define BTN_DOWN 18
#define BTN_LEFT 23
#define BTN_RIGHT 22

Adafruit_NeoPixel *pixels;

File root;
File dataFile;

int filePos = 0;
int numFiles = 0;

bool runAnimation = false;

String fileNames[10];
String currentFilename = "";

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pixels = new Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
  pixels->begin();
  pixels->setBrightness(BRIGHTNESS);
  setupSDcard();
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(17, LOW);
}

void loop()
{

  // loop through until current file position
  for (int i = 0; i < (filePos + 1); i++)
  {

    // create a spacing between the lit LED for visibility
    int j = i * 5;

    // color every other pixel green and blue
    if (i % 2 == 0)
    {
      pixels->setPixelColor(j, pixels->Color(0, 255, 0));
    }
    else
    {
      pixels->setPixelColor(j, pixels->Color(0, 0, 255));
    }
    pixels->show();
  }

  bool buttonUp = !digitalRead(BTN_UP);
  bool buttonDown = !digitalRead(BTN_DOWN);
  //bool buttonLeft = !digitalRead(BTN_LEFT);
  bool buttonRight = !digitalRead(BTN_RIGHT);

  // is any of the up or down button pushed?
  if (buttonUp || buttonDown)
  {

    // get the direction
    int dir = (buttonUp) ? 1 : -1;

    // add to current file position
    filePos = filePos + dir;

    // if less than 0 set to end of list
    if (filePos < 0)
    {
      filePos = numFiles - 1;
    }
    // otherwise set to the beginning
    else if (filePos == numFiles)
    {
      filePos = 0;
    }

    // print out the current file position
    Serial.print(filePos);
    Serial.print("\t");
    Serial.println(fileNames[filePos]);

    // clear all pixels
    pixels->clear();

    // wait a bit…
    delay(250);
  }

  if (buttonRight)
  {
    Serial.print("Play!");
    pixels->clear();
    pixels->setPixelColor(0, pixels->Color(255, 0, 0));
    pixels->show();
    sendFile(fileNames[filePos]);
  }
}

void sendFile(String filename)
{

  char temp[14];

  filename.toCharArray(temp, 14);
  dataFile = SD.open(temp);

  if (dataFile)
  {
    int i = 0;
    int r, g, b;

    // wait a bit after clicking go so there's time to find the right pose and place
    delay(8000);

    while (dataFile.available())
    {

      bool buttonRight = !digitalRead(BTN_RIGHT);

      if (buttonRight)
      {
        Serial.println("Stop!");
        pixels->clear();
        pixels->show();
        dataFile.close();
        delay(200);
        break;
      }

      if (i == (pixels->numPixels()))
      {
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
  }
  else
  {
    Serial.println("Error reading");
    setupSDcard();
    return;
  }
}

void setupSDcard()
{
  pinMode(BUILTIN_SDCARD, OUTPUT);
  while (!SD.begin(BUILTIN_SDCARD))
  {
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

void getFileNamesFromSD(File dir)
{
  int fileCount = 0;
  String CurrentFilename = "";
  while (1)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      numFiles = fileCount;
      sortArray(fileNames, numFiles);
      Serial.print(fileCount);
      Serial.println(" files found!");
      Serial.println("Names: ");
      for (int i = 0; i < numFiles; i++)
      {
        Serial.println(fileNames[i]);
      }
      entry.close();
      break;
    }
    else
    {
      CurrentFilename = entry.name();
      if (CurrentFilename.endsWith(".TXT") || CurrentFilename.startsWith("_"))
      {
        fileNames[fileCount] = entry.name();
        fileCount++;
      }
    }
    entry.close();
  }
}
