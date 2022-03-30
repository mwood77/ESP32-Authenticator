#include <WiFi.h>
#include <TOTP.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1

struct tm timeinfo;
struct VERSION {
  int major;
  int minor;
  int patch;
};

char ssid[] = "";
char password[] = "";

String connecting = String("SSID:");
String message = String("ESP32-Authenticator");
String version = String("version");
String totpCode = String("");

long timeWhenTOTPGenerated;

uint8_t hmacKey[] = {0x52, 0x41, 0x50, 0x4f, 0x59, 0x47, 0x52, 0x54, 0x44, 0x50, 0x58, 0x47, 0x53, 0x32, 0x59, 0x48};

/**  
  offset examples
  GMT +1 = 3600 (3600 seconds in an hour)
  GMT +8 = 28800
  GMT -1 = -3600
  GMT 0 = 0
*/
const long  gmtOffset_sec = 3600;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", gmtOffset_sec);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

TOTP totp = TOTP(hmacKey, 16);

static VERSION build_version = {
  0, 0, 1
};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  bootDisplay(message, version, true);
  
  delay(2500);
  initWifi();
  
}

void loop() {

  // @todo - UDP not working...
  String newCode = String(totp.getCode(timeClient.getEpochTime()));

  if(totpCode != newCode) {
    totpCode = String(newCode);
    timeWhenTOTPGenerated = timeClient.getEpochTime();
    updateDisplay(totpCode);
  } else {
    drawProgressLine();
  }

}

void initWifi() {
  boolean connToggle = false;
  display.clearDisplay();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    bootDisplay(connecting, ssid, false);
    if (connToggle) {
      display.fillRect(26, 42, 128, 20, BLACK);
      display.setCursor(26, 42);
      display.print("connecting.");
      connToggle = !connToggle;
    } else {
      display.fillRect(26, 42, 128, 20, BLACK);
      display.setCursor(26, 42);
      display.print("connecting..."); 
      connToggle = !connToggle;
    }
    delay(1000);
    display.display();
  }
  Serial.println("connection made");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
}

void bootDisplay(String m1, String m2, boolean displayVersion) {

  char versionString[9];
  sprintf(versionString, "%2d.%2d.%2d", build_version.major, build_version.minor, build_version.patch);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(6,14);
  display.print(m1);
  display.setCursor(38,28);
  display.print(m2);

  if (displayVersion) {
    display.setCursor(34,42);
    display.print(versionString);
  }

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
  const float currentTime = timeClient.getEpochTime();
  const float currentProgress = ((currentTime - timeWhenTOTPGenerated) / 30);
  
  display.fillRect(0, 0, 128, 6, BLACK);
  display.drawLine(0, 2, 128 - (128 * currentProgress), 2, WHITE);    //  line is drawn backwards
  display.display();
}
