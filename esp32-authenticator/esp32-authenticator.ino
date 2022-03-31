#include <WiFi.h>
#include <TOTP.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1

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
String syncMessage1 = String("syncing with time");
String syncMessage2 = String("server");

unsigned long timeWhenTOTPGenerated;

// europe.pool.ntp.org
const char * udpAddress = "85.254.217.5";
const int udpPort = 3333;

//XK4GBLITWBVMCBOM
// to hex -> 0xba, 0xb8, 0x60, 0xad, 0x13, 0xb0, 0x6a, 0xc1, 0x05, 0xcc
uint8_t hmacKey[] =  {0xba, 0xb8, 0x60, 0xad, 0x13, 0xb0, 0x6a, 0xc1, 0x05, 0xcc};

WiFiUDP udp;
NTPClient timeClient(udp, udpAddress);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

TOTP totp = TOTP(hmacKey, 10);

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

  timeClient.update();
  unsigned long epoch = timeClient.getEpochTime();
  String newCode = String(totp.getCode(epoch));

  if(totpCode != newCode) {
    totpCode = String(newCode);
    timeWhenTOTPGenerated = epoch;
    updateDisplay(totpCode);
  } else {
    drawProgressLine(timeWhenTOTPGenerated);
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
  display.clearDisplay();
  bootDisplay(syncMessage1, syncMessage2, false);
  display.display();
  udp.begin(WiFi.localIP(),udpPort);
  delay(2000);
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

void drawProgressLine(unsigned long epoch) {
  unsigned long currentTime = timeClient.getEpochTime();  
  float secondsElapsed = (currentTime - epoch) % 30;
  float percentage = (secondsElapsed / 30);
  
  display.fillRect(0, 0, 128, 6, BLACK);
  display.drawLine(0, 2, 128 - (128 * percentage), 2, WHITE);    //  line is drawn backwards
  display.display();
}
