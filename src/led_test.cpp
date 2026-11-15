#include <Arduino.h>
#include <Wire.h>

// XVF3800 listens on I2C address 0x2C (XIAO SDA = GPIO5, SCL = GPIO6)
#define XMOS_ADDR 0x2C

// LED commands live in resource 20 (GPO servicer)
#define RESID_GPO      20
#define CMD_EFFECT     12
#define CMD_BRIGHTNESS 13
#define CMD_SPEED      15
#define CMD_COLOR      16

// Effect values
#define EFFECT_OFF       0
#define EFFECT_BREATHING 1
#define EFFECT_RAINBOW   2
#define EFFECT_SOLID     3

// Sends [resid][cmd][len][data...] and prints an error if the XVF3800 doesn't respond
bool xmosWrite(uint8_t cmd, const uint8_t *data, uint8_t len) {
  Wire.beginTransmission(XMOS_ADDR);
  Wire.write(RESID_GPO);
  Wire.write(cmd);
  Wire.write(len);
  Wire.write(data, len);
  uint8_t err = Wire.endTransmission();
  if (err != 0) {
    Serial.printf("  I2C error %u on cmd %u\n", err, cmd);
    return false;
  }
  return true;
}

void setEffect(uint8_t effect) { xmosWrite(CMD_EFFECT, &effect, 1); }
void setBrightness(uint8_t b)  { xmosWrite(CMD_BRIGHTNESS, &b, 1); }
void setSpeed(uint8_t s)       { xmosWrite(CMD_SPEED, &s, 1); }

// Color as 0xRRGGBB, sent low byte first like Seeed's example
void setColor(uint32_t rgb) {
  uint8_t data[4] = {
    (uint8_t)(rgb & 0xFF),
    (uint8_t)((rgb >> 8) & 0xFF),
    (uint8_t)((rgb >> 16) & 0xFF),
    0x00
  };
  xmosWrite(CMD_COLOR, data, 4);
}

struct Step {
  const char *name;
  uint8_t effect;
  uint32_t color;
};

Step steps[] = {
  {"solid red",        EFFECT_SOLID,     0xFF0000},
  {"solid green",      EFFECT_SOLID,     0x00FF00},
  {"solid blue",       EFFECT_SOLID,     0x0000FF},
  {"breathing orange", EFFECT_BREATHING, 0xFF8800},
  {"rainbow",          EFFECT_RAINBOW,   0x000000},
  {"off",              EFFECT_OFF,       0x000000},
};
const int NUM_STEPS = sizeof(steps) / sizeof(steps[0]);
int current = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(1000);  // give the XVF3800 time to boot

  setBrightness(255);
  setSpeed(1);
  Serial.println("LED test starting, changes every 3 seconds");
}

void loop() {
  Step &s = steps[current];
  Serial.printf("Setting: %s\n", s.name);
  setColor(s.color);
  setEffect(s.effect);

  current = (current + 1) % NUM_STEPS;
  delay(3000);
}
