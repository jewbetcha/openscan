// Bench test: verify the SSD1306 OLED is wired and addressable.
// Expect: "openscan oled ok" on the display, then a counter incrementing.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr uint8_t PIN_SDA = 21;
constexpr uint8_t PIN_SCL = 22;
constexpr uint8_t I2C_ADDR = 0x3C;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) {
    Serial.println("oled init failed");
    while (true) delay(1000);
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2);
  oled.setCursor(0, 0);
  oled.println("oled ok");
  oled.display();
  delay(1000);
}

void loop() {
  static uint32_t i = 0;
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(3);
  oled.println(i++);
  oled.display();
  delay(250);
}
