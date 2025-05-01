#include <Audio.h>
#include "SPIFFS.h"

// Audio objects
Audio audio;

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Failed to initialize SPIFFS!");
    while (1);
  }
  Serial.println("SPIFFS initialized.");

  // Start the audio object
  audio.setPinout(25, 26, 27); // BCLK, LRC, DIN
  audio.setVolume(20);         // Volume (0-21)

  // Begin MP3 playback from SPIFFS
  if (audio.connecttoFS(SPIFFS, "/helloworld.mp3")) {
    Serial.println("MP3 playback started.");
  } else {
    Serial.println("Failed to start MP3 playback.");
  }
}

void loop() {
  audio.loop(); // Maintain audio playback
}
