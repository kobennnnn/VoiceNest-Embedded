#include <Arduino.h>
#include "AudioTools.h"

// 16 kHz, 2 channels, 32 bit, matches the XVF3800 I2S firmware
AudioInfo info(16000, 2, 32);

I2SStream i2s;
I2SConfig config;

// 16000 samples/s x 2 channels x 4 bytes = 128000 bytes = about 1 second of audio
uint8_t buffer[128000];

void setup() {
  Serial.begin(115200);
  while (!Serial);  // waits until you open the Serial Monitor
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  config = i2s.defaultConfig(TX_MODE);
  config.copyFrom(info);
  config.pin_bck = 8;
  config.pin_ws = 7;
  config.pin_data = 44;     // XIAO to XVF3800 (speaker out)
  config.pin_data_rx = 43;  // XVF3800 to XIAO (mic in)
  config.is_master = true;
  i2s.begin(config);
  Serial.println("Mic test ready. Talk, then listen.");
}

void loop() {
  // Record about 1 second
  i2s.end();
  config.rx_tx_mode = RX_MODE;
  i2s.begin(config);
  Serial.println("Recording...");
  size_t bytesRead = i2s.readBytes(buffer, sizeof(buffer));

  // Find the loudest sample so you can see mic data as a number
  int32_t *samples = (int32_t *)buffer;
  size_t count = bytesRead / 4;
  int32_t peak = 0;
  for (size_t i = 0; i < count; i++) {
    int32_t v = samples[i];
    if (v < 0) v = -v;
    if (v > peak) peak = v;
  }
  Serial.printf("Got %u bytes, peak level: %ld\n", (unsigned)bytesRead, (long)peak);

  // Play it back
  i2s.end();
  config.rx_tx_mode = TX_MODE;
  i2s.begin(config);
  Serial.println("Playing back...");
  i2s.write(buffer, bytesRead);
}
