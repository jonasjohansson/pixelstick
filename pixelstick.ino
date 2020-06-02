#include <Adafruit_NeoPixel.h>
#include <ArduinoSort.h>

#include <SPI.h>
#include <SD.h>

#define DELAY 80
#define BRIGHTNESS 255
#define LED_PIN 2
#define NUM_PIXELS 144
#define BTN_GND 17
#define BTN_UP 19
#define BTN_DOWN 18
#define BTN_LEFT 23
#define BTN_RIGHT 22

Adafruit_NeoPixel *pixels;

File root;
File dataFile;

int filePos = 0;
int numFiles = 0;

bool useButtons = true;
bool runAnimation = false;

String fileNames[10];
String currentFilename = "";

void setup()
{
  Serial.begin(115200);
  if (useButtons)
  {
    pinMode(BTN_GND, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    digitalWrite(BTN_GND, LOW);
  }
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pixels = new Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
  pixels->begin();
  pixels->setBrightness(BRIGHTNESS);
  setupSDcard();
  delay(1000);
}

void loop()
{
  if (useButtons)
  {
    bool buttonUp = !digitalRead(BTN_UP);
    bool buttonDown = !digitalRead(BTN_DOWN);
    bool buttonLeft = !digitalRead(BTN_LEFT);
    bool buttonRight = !digitalRead(BTN_RIGHT);
    if (buttonUp || buttonDown)
    {
      nextFile(buttonUp);
    }
    else if (buttonLeft)
    {
      test();
    }
    else if (buttonRight)
    {
      playFile(fileNames[filePos]);
    }
  }
  else
  {
    delay(5000);
    playFile(fileNames[filePos]);
  }
}

void playFile(String filename)
{

  pixels->clear();
  pixels->setPixelColor(0, pixels->Color(255, 0, 0));
  pixels->show();

  char temp[14];

  filename.toCharArray(temp, 14);
  Serial.println(filename);
  dataFile = SD.open(temp);

  if (dataFile)
  {
    int i = 0;
    int r, g, b;

    // wait a bit after clicking go so there's time to find the right pose and place
    delay(5000);

    while (dataFile.available())
    {

      bool buttonRight = !digitalRead(BTN_RIGHT);

      if (buttonRight)
      {
        closeFile();
        break;
      }

      if (i == pixels->numPixels())
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
    closeFile();
    if (!useButtons)
      nextFile(true);
  }
  else
  {
    Serial.println("Error reading");
    setupSDcard();
    return;
  }
}

void closeFile()
{
  Serial.println("Close file");
  dataFile.close();
  pixels->clear();
  pixels->show();
  delay(200);
}

void nextFile(int dir)
{

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

  showCurrentPosition();
}

void showCurrentPosition()
{
  for (int i = 0; i < (filePos + 1); i++)
  {
    int j = i * 5;
    if (i % 2 == 0)
      pixels->setPixelColor(j, pixels->Color(0, 255, 0));
    else
      pixels->setPixelColor(j, pixels->Color(0, 0, 255));
  }
  pixels->show();
}

void test()
{
  for (int i = 0; i < pixels->numPixels(); i++)
    pixels->setPixelColor(i, pixels->Color(128, 128, 128));
  pixels->show();
  delay(5000);
  pixels->clear();
  pixels->show();
}

void setupSDcard()
{
  pinMode(2, OUTPUT);
  while (!SD.begin(2))
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
        Serial.println("Found: " + String(entry.name()));
        fileNames[fileCount] = entry.name();
        fileCount++;
      }
    }
    entry.close();
  }
}
