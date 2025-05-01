#include <TFT_eSPI.h>
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "Orbitron10pt7b.h"
#include "Orbitron20pt7b.h"
#include "Aerospace10pt7b.h"
#include "Aerospace15pt7b.h"
#include "Aerospace20pt7b.h"

TFT_eSPI tft = TFT_eSPI();

#define GFXFF 1
#define ORBI10 &Orbitron_VariableFont_wght10pt7b
#define ORBI20 &Orbitron_VariableFont_wght20pt7b
#define AERO10 &Aerospace10pt7b
#define AERO15 &Aerospace15pt7b
#define AERO20 &Aerospace20pt7b

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
  {"Capital\n     FM UK", "http://vis.media-ice.musicradio.com/CapitalMP3"},
  {"Swiss\n     SRF 3", "http://stream.srg-ssr.ch/m/drs3/mp3_128"},
  {"Swiss\n     Couleur\n     3", "http://stream.srg-ssr.ch/m/couleur3/mp3_128"},
  {"Radio\n     Swiss\n     Pop", "http://stream.srg-ssr.ch/m/rsp/mp3_128"},
  {"Virgin\n     Radio\n     Rock", "http://icy.unitedradio.it/Virgin.mp3"},
  {"Classic\n     FM", "http://ice-sov.musicradio.com/ClassicFMMP3"},
  {"Joint\n     Radio\n     Reggae", "http://star.jointil.net/proxy/jrn_reggae?mp=/stream"}
};

const int numStations = sizeof(stations) / sizeof(stations[0]);
Audio audio;

class NetworkRadioController {
private:
  int stationIndex = 0;
  int volume = 5;  // Set initial volume to 5
  int lastClkState = HIGH;
  int lastSwState = HIGH;
  bool volumeMode = false;

public:
  void init() {
    pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
    pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
    pinMode(ROTARY_SW_PIN, INPUT_PULLUP);
    Serial.println("NetworkRadioController initialized");
  }

  void processRotation() {
    int clkState = digitalRead(ROTARY_CLK_PIN);
    int dtState = digitalRead(ROTARY_DT_PIN);

    if (clkState != lastClkState) {
      if (clkState == LOW) {
        if (dtState == HIGH) {
          if (volumeMode) {
            volume = min(volume + 1, 21); // Max volume is 21
            audio.setVolume(volume);
            updateVolumeDisplay();
          } else {
            stationIndex = (stationIndex + 1) % numStations;
            changeStation();
          }
        } else {
          if (volumeMode) {
            volume = max(volume - 1, 0); // Min volume is 0
            audio.setVolume(volume);
            updateVolumeDisplay();
          } else {
            stationIndex = (stationIndex - 1 + numStations) % numStations;
            changeStation();
          }
        }
        delay(50);
      }
    }
    lastClkState = clkState;
  }

  void processClick() {
    int swState = digitalRead(ROTARY_SW_PIN);
    if (swState == LOW && lastSwState == HIGH) {
      volumeMode = !volumeMode; // Toggle volume mode
      if (volumeMode) {
        updateVolumeDisplay();
      } else {
        changeStation();
      }
      delay(200);
    }
    lastSwState = swState;
  }

  void changeStation() {
    Serial.print("Selected Station: ");
    Serial.println(stations[stationIndex].name);
    audio.connecttohost(stations[stationIndex].url);
    updateDisplay(stations[stationIndex].name);
  }

  void updateDisplay(const char* stationName) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(40, 60);
    tft.setTextColor(TFT_YELLOW);
    tft.setFreeFont(ORBI20);
    tft.print("Playing:");
    tft.setTextColor(TFT_BLUE);
    tft.setFreeFont(AERO15);
    tft.setCursor(40, 100);
    tft.print(stationName);
  }

  void updateVolumeDisplay() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(40, 60);
    tft.setTextColor(TFT_YELLOW);
    tft.setFreeFont(ORBI20);
    tft.print("Volume:");
    tft.setTextColor(TFT_GREEN);
    tft.setFreeFont(AERO20);
    tft.setCursor(100, 180);
    tft.print(volume);
  }
};

NetworkRadioController radioController;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Attempting to connect to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Wi-Fi Connected!");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(5);  // Set initial volume to 5
  Serial.println("Audio setup complete");

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  radioController.init();
  radioController.changeStation();
}

void loop() {
  radioController.processRotation();
  radioController.processClick();
  audio.loop();
}
