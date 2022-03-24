#include <TOTP.h>
#include <Adafruit_SSD1306.h>

struct VERSION {
  int major;
  int minor;
  int patch;
};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t hmacKey[] = {0x66, 0x61, 0x72, 0x74, 0x73};
TOTP totp = TOTP(hmacKey, 5,30);

time_t now;
struct tm timeinfo;
char strftime_buf[64];
long timeWhenTOTPGenerated;
String totpCode;

static VERSION build_version = {
  0, 0, 1
};

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  initDisplay();
  delay(2500);
}

void loop() {

  drawProgressLine();

  String newCode = String(totp.getCode(time(&now)));
  if(totpCode != newCode) {
    totpCode = String(newCode);
    timeWhenTOTPGenerated = time(&now);
    updateDisplay(newCode);
  }

}

void initDisplay() {

  char versionString[9];
  sprintf(versionString, "%2d.%2d.%2d", build_version.major, build_version.minor, build_version.patch);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(6,14);
  display.print("ESP32-Authenticator");
  display.setCursor(38,28);
  display.print("version");
  display.setCursor(34,42);
  display.print(versionString);
  display.display();

}

void updateDisplay(String totp) {
  display.clearDisplay();
  display.setCursor(10,14);
  display.print("Some Issuer");
  display.setTextSize(3);
  display.setCursor(10,30);
  display.print(totp);
  display.setTextSize(1);
  display.display();

}

void drawProgressLine() {
  const float currentTime = time(&now);
  const float currentProgress = ((currentTime - timeWhenTOTPGenerated) / 30);
  
  display.fillRect(0, 0, 128, 6, BLACK);
  display.drawLine(0, 2, 128 - (128 * currentProgress), 2, WHITE);    //  line is drawn backwards
  display.display();
}
