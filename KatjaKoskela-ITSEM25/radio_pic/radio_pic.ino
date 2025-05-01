#include <Audio.h>
#include <TFT_eSPI.h>
#include "Arduino.h"
#include "WiFi.h"

#include "FS.h"
#include "SPIFFS.h"
#include "ezTime.h"
#include "Orbitron10pt7b.h"
#include "Orbitron20pt7b.h"
#include "Aerospace10pt7b.h"
#include "Aerospace15pt7b.h"
#include "Aerospace20pt7b.h"

TFT_eSPI tft = TFT_eSPI();
Timezone myTZ;

#define GFXFF 1
#define ORBI20 &Orbitron_VariableFont_wght20pt7b
#define ORBI10 &Orbitron_VariableFont_wght10pt7b
#define AERO15 &Aerospace15pt7b

#define I2S_DOUT 27
#define I2S_BCLK 25
#define I2S_LRC  26
#define ROTARY_CLK_PIN 17
#define ROTARY_DT_PIN  18
#define ROTARY_SW_PIN  19

String ssid = "xxx";
String password = "xxx";

struct Station {
  const char* name;
  const char* url;
};

Station stations[] = {
  {"Capital FM UK", "http://vis.media-ice.musicradio.com/CapitalMP3"},
  {"Swiss SRF 3", "http://stream.srg-ssr.ch/m/drs3/mp3_128"},
  {"Virgin Radio Rock", "http://icy.unitedradio.it/Virgin.mp3"},
};

const int numStations = sizeof(stations) / sizeof(stations[0]);
Audio audio;
String lastTime = ""; // Stores last updated time

class NetworkRadioController {
private:
  int stationIndex = 0;
  int volume = 5;
  int lastClkState = HIGH;
  int lastSwState = HIGH;
  bool volumeMode = false; // True = Adjust volume, False = Change stations

public:
  void init() {
    pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
    pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
    pinMode(ROTARY_SW_PIN, INPUT_PULLUP);
    Serial.println("Controller ready");
  }

  void processRotation() {
    int clkState = digitalRead(ROTARY_CLK_PIN);
    int dtState = digitalRead(ROTARY_DT_PIN);
    if (clkState != lastClkState && clkState == LOW) {
      if (dtState == HIGH) {
        volumeMode ? adjustVolume(1) : nextStation();
      } else {
        volumeMode ? adjustVolume(-1) : prevStation();
      }
      delay(50);
    }
    lastClkState = clkState;
  }

  void processClick() {
    int swState = digitalRead(ROTARY_SW_PIN);
    if (swState == LOW && lastSwState == HIGH) {
      volumeMode = !volumeMode; // Toggle mode
      updateModeDisplay();
      delay(200);
    }
    lastSwState = swState;
  }

  void nextStation() {
    stationIndex = (stationIndex + 1) % numStations;
    playStation();
  }

  void prevStation() {
    stationIndex = (stationIndex - 1 + numStations) % numStations;
    playStation();
  }

  void adjustVolume(int change) {
    volume = constrain(volume + change, 0, 21);
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
    updateModeDisplay();
  }

  void playStation() {
    Serial.print("Playing: ");
    Serial.println(stations[stationIndex].name);
    audio.connecttohost(stations[stationIndex].url);
    updateStationDisplay();
  }

  void updateStationDisplay() {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(ORBI10);
    // Directly print over the previous station text, no need to clear space
    tft.setCursor(50, 100);
    tft.print(stations[stationIndex].name);
  }

  void updateModeDisplay() {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(AERO15);
    // Directly print over the previous volume or mode text
    tft.setCursor(60, 100);
    tft.print(volumeMode ? ": " + String(volume) : "");
  }
};

NetworkRadioController radioController;

void displayBackground() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS mount failed!");
    return;
  }
  File file = SPIFFS.open("/patrik.bin", "r");
  if (!file) {
    Serial.println("Image load failed!");
    return;
  }
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 240; x++) {
      uint16_t color = file.read() | (file.read() << 8);
      tft.drawPixel(x, y, color);
    }
  }
  file.close();
}

void updateClock() {
  String timeStr = myTZ.dateTime("H:i"); // HH:MM format
  if (timeStr != lastTime) { // Only update if time changed
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(AERO15);
    // Directly print the time, overwriting the old time
    tft.setCursor(60, 200);
    tft.print(timeStr);
    lastTime = timeStr;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi Connected!");

  myTZ.setLocation("Europe/Helsinki");
  waitForSync(); // Ensures time is synced correctly before displaying

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(5);

  tft.init();
  tft.setRotation(0);
  displayBackground();
  
  radioController.init();
  radioController.playStation();
}

void loop() {
  radioController.processRotation();
  radioController.processClick();
  audio.loop();
  updateClock();
  delay(100); // Reduced delay for smoother interaction
}
