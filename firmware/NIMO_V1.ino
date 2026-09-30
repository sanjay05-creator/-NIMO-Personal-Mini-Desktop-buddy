/*
  NIMO - Personal Mini Monitor
  V1
  Controller: ESP32-C3 SuperMini

  Features:
  - OLED clock + date
  - DS3231 RTC
  - NIMO face
  - UP / DOWN / SET buttons
  - Touch wake
  - Relay control
  - Wi-Fi phone control
*/

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =========================
// PIN CONFIGURATION
// =========================

#define OLED_SDA     4
#define OLED_SCL     5

#define BTN_UP       0
#define BTN_DOWN     1
#define BTN_SET      3

#define TOUCH_PIN   10

#define RELAY_PIN    6

// =========================
// OLED
// =========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =========================
// RTC
// =========================

RTC_DS3231 rtc;

// =========================
// WIFI
// =========================

const char* AP_SSID = "NIMO";
const char* AP_PASSWORD = "nimo1234";

WebServer server(80);

// =========================
// VARIABLES
// =========================

bool lightState = false;

bool menuOpen = false;

int menuItem = 0;

unsigned long lastButtonTime = 0;
unsigned long lastTouchTime = 0;
unsigned long lastFaceTime = 0;

int faceState = 0;

// Relay module assumed ACTIVE LOW
void setLight(bool state) {
  lightState = state;

  if (lightState) {
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
}

// =========================
// NIMO FACE
// =========================

void drawNimoFace(int state) {

  // Robot head
  display.drawRoundRect(34, 12, 60, 43, 8, SSD1306_WHITE);

  // Antenna
  display.drawLine(64, 12, 64, 5, SSD1306_WHITE);
  display.fillCircle(64, 4, 2, SSD1306_WHITE);

  // Side pads
  display.fillRoundRect(28, 25, 6, 14, 2, SSD1306_WHITE);
  display.fillRoundRect(94, 25, 6, 14, 2, SSD1306_WHITE);

  if (state == 1) {
    // Blink
    display.drawLine(45, 29, 55, 29, SSD1306_WHITE);
    display.drawLine(73, 29, 83, 29, SSD1306_WHITE);
  }
  else if (state == 2) {
    // Happy
    display.fillCircle(50, 29, 5, SSD1306_WHITE);
    display.fillCircle(78, 29, 5, SSD1306_WHITE);
    display.drawLine(52, 40, 56, 43, SSD1306_WHITE);
    display.drawLine(56, 43, 64, 45, SSD1306_WHITE);
    display.drawLine(64, 45, 72, 43, SSD1306_WHITE);
    display.drawLine(72, 43, 76, 40, SSD1306_WHITE);
  }
  else if (state == 3) {
    // Thinking
    display.fillCircle(50, 29, 5, SSD1306_WHITE);
    display.fillCircle(78, 29, 5, SSD1306_WHITE);
    display.drawLine(57, 42, 71, 42, SSD1306_WHITE);
    display.fillCircle(88, 17, 2, SSD1306_WHITE);
    display.fillCircle(94, 12, 1, SSD1306_WHITE);
  }
  else if (state == 4) {
    // Surprised
    display.fillCircle(50, 29, 5, SSD1306_WHITE);
    display.fillCircle(78, 29, 5, SSD1306_WHITE);
    display.drawCircle(64, 42, 4, SSD1306_WHITE);
  }
  else {
    // Normal face
    display.fillCircle(50, 29, 5, SSD1306_WHITE);
    display.fillCircle(78, 29, 5, SSD1306_WHITE);

    // Smile
    display.drawLine(56, 41, 60, 44, SSD1306_WHITE);
    display.drawLine(60, 44, 68, 44, SSD1306_WHITE);
    display.drawLine(68, 44, 72, 41, SSD1306_WHITE);
  }
}

// =========================
// BOOT SCREEN
// =========================

void bootScreen() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(39, 8);
  display.println("NIMO");

  display.setTextSize(1);
  display.setCursor(43, 30);
  display.println("Starting...");

  display.drawRect(20, 45, 88, 8, SSD1306_WHITE);

  for (int i = 0; i < 84; i += 4) {
    display.fillRect(22, 47, i, 4, SSD1306_WHITE);
    display.display();
    delay(20);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(40, 4);
  display.println("NIMO ONLINE");

  drawNimoFace(2);

  display.display();
  delay(1200);
}

// =========================
// MAIN CLOCK SCREEN
// =========================

void drawMainScreen() {

  DateTime now = rtc.now();

  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor(3, 1);
  display.print("NIMO");

  display.setCursor(88, 1);

  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi");
  } else {
    display.print("AP");
  }

  // Time
  int hour12 = now.hour();

  String ampm = "AM";

  if (hour12 >= 12) {
    ampm = "PM";
  }

  if (hour12 == 0) {
    hour12 = 12;
  } else if (hour12 > 12) {
    hour12 -= 12;
  }

  display.setTextSize(2);
  display.setCursor(2, 13);

  if (hour12 < 10) display.print("0");
  display.print(hour12);
  display.print(":");

  if (now.minute() < 10) display.print("0");
  display.print(now.minute());

  // Seconds
  display.setTextSize(1);
  display.setCursor(80, 19);

  if (now.second() < 10) display.print("0");
  display.print(now.second());

  // AM / PM
  display.setTextSize(1);
  display.setCursor(99, 19);
  display.print(ampm);

  // Date
  display.setCursor(3, 34);

  if (now.day() < 10) display.print("0");
  display.print(now.day());
  display.print("/");

  if (now.month() < 10) display.print("0");
  display.print(now.month());
  display.print("/");
  display.print(now.year());

  // Light status
  display.setCursor(3, 55);
  display.print("LIGHT:");

  if (lightState) {
    display.print("ON");
  } else {
    display.print("OFF");
  }

  // Small NIMO face
  display.drawRoundRect(92, 34, 30, 25, 5, SSD1306_WHITE);
  display.fillCircle(100, 44, 2, SSD1306_WHITE);
  display.fillCircle(114, 44, 2, SSD1306_WHITE);
  display.drawLine(104, 51, 110, 51, SSD1306_WHITE);

  display.display();
}

// =========================
// MENU
// =========================

void drawMenu() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(3, 2);
  display.println("NIMO MENU");

  const char* items[] = {
    "Status",
    "Controls",
    "Settings",
    "Help"
  };

  for (int i = 0; i < 4; i++) {

    int y = 15 + (i * 11);

    if (i == menuItem) {
      display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(8, y);
    display.println(items[i]);

    display.setTextColor(SSD1306_WHITE);
  }

  display.display();
}

// =========================
// STATUS SCREEN
// =========================

void showStatus() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(3, 2);
  display.println("NIMO STATUS");

  display.setCursor(5, 18);
  display.println("ESP32-C3 : ONLINE");

  display.setCursor(5, 29);
  display.print("WiFi     : ");

  if (WiFi.status() == WL_CONNECTED) {
    display.println("READY");
  } else {
    display.println("AP MODE");
  }

  display.setCursor(5, 40);
  display.print("Light    : ");

  if (lightState) {
    display.println("ON");
  } else {
    display.println("OFF");
  }

  display.setCursor(5, 51);
  display.println("RTC      : OK");

  display.display();

  delay(2000);
}

// =========================
// CONTROL SCREEN
// =========================

void showControls() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(3, 2);
  display.println("CONTROLS");

  display.setCursor(8, 20);
  display.print("HOUSE LIGHT: ");

  if (lightState) {
    display.println("ON");
  } else {
    display.println("OFF");
  }

  display.setCursor(8, 36);
  display.println("SET = Toggle");

  display.setCursor(8, 50);
  display.println("UP/DOWN = Back");

  display.display();

  while (true) {

    if (digitalRead(BTN_SET) == LOW) {
      delay(200);
      setLight(!lightState);
      break;
    }

    if (digitalRead(BTN_UP) == LOW ||
        digitalRead(BTN_DOWN) == LOW) {
      delay(200);
      break;
    }
  }
}

// =========================
// HELP SCREEN
// =========================

void showHelp() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(3, 2);
  display.println("NIMO HELP");

  display.setCursor(5, 18);
  display.println("UP/DOWN : Navigate");

  display.setCursor(5, 30);
  display.println("SET     : Select");

  display.setCursor(5, 42);
  display.println("TOUCH   : Wake");

  display.setCursor(5, 54);
  display.println("Phone   : 192.168.4.1");

  display.display();

  delay(2500);
}

// =========================
// SETTINGS SCREEN
// =========================

void showSettings() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(3, 2);
  display.println("SETTINGS");

  display.setCursor(5, 19);
  display.println("NIMO FACE : ON");

  display.setCursor(5, 31);
  display.println("TOUCH     : ON");

  display.setCursor(5, 43);
  display.println("WiFi CTRL : ON");

  display.setCursor(5, 55);
  display.println("V1 DEVICE");

  display.display();

  delay(2200);
}

// =========================
// WEB PAGE
// =========================

String webPage() {

  String state;

  if (lightState) {
    state = "ON";
  } else {
    state = "OFF";
  }

  String page = "";

  page += "<!DOCTYPE html>";
  page += "<html>";
  page += "<head>";
  page += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  page += "<title>NIMO</title>";

  page += "<style>";
  page += "body{background:#080808;color:white;font-family:Arial;text-align:center;padding:30px}";
  page += "h1{font-size:42px}";
  page += ".box{padding:20px;border:1px solid #555;border-radius:15px}";
  page += "button{font-size:20px;padding:15px 25px;margin:10px;border-radius:10px}";
  page += "</style>";

  page += "</head>";
  page += "<body>";

  page += "<h1>NIMO</h1>";

  page += "<div class='box'>";
  page += "<h2>HOUSE LIGHT</h2>";
  page += "<h3>Status: " + state + "</h3>";

  page += "<a href='/on'><button>TURN ON</button></a>";
  page += "<a href='/off'><button>TURN OFF</button></a>";

  page += "<p>NIMO ONLINE</p>";

  page += "</div>";

  page += "</body>";
  page += "</html>";

  return page;
}

// =========================
// WEB HANDLERS
// =========================

void handleRoot() {

  server.send(200, "text/html", webPage());
}

void handleOn() {

  setLight(true);

  server.send(200, "text/html", webPage());
}

void handleOff() {

  setLight(false);

  server.send(200, "text/html", webPage());
}

// =========================
// BUTTON HANDLING
// =========================

void handleButtons() {

  if (millis() - lastButtonTime < 180) {
    return;
  }

  if (digitalRead(BTN_SET) == LOW) {

    lastButtonTime = millis();

    if (!menuOpen) {
      menuOpen = true;
      menuItem = 0;
      return;
    }

    if (menuItem == 0) {
      showStatus();
    }

    else if (menuItem == 1) {
      showControls();
    }

    else if (menuItem == 2) {
      showSettings();
    }

    else if (menuItem == 3) {
      showHelp();
    }

    menuOpen = false;
  }

  if (digitalRead(BTN_UP) == LOW) {

    lastButtonTime = millis();

    if (menuOpen) {
      menuItem--;

      if (menuItem < 0) {
        menuItem = 3;
      }
    }
  }

  if (digitalRead(BTN_DOWN) == LOW) {

    lastButtonTime = millis();

    if (menuOpen) {
      menuItem++;

      if (menuItem > 3) {
        menuItem = 0;
      }
    }
  }
}

// =========================
// TOUCH
// =========================

void handleTouch() {

  if (digitalRead(TOUCH_PIN) == HIGH) {

    if (millis() - lastTouchTime > 800) {

      lastTouchTime = millis();

      faceState = 2;
    }
  }
}

// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(115200);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SET, INPUT_PULLUP);

  pinMode(TOUCH_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);

  // Start with relay OFF
  setLight(false);

  // I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED ERROR");

    while (true) {
      delay(1000);
    }
  }

  // RTC
  if (!rtc.begin()) {

    Serial.println("RTC ERROR");
  }

  // WiFi Access Point
  WiFi.mode(WIFI_AP);

  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );

  Serial.println("NIMO WiFi started");
  Serial.println(WiFi.softAPIP());

  // Web server
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.begin();

  bootScreen();
}

// =========================
// LOOP
// =========================

void loop() {

  server.handleClient();

  handleButtons();

  handleTouch();

  // Face animation
  if (millis() - lastFaceTime > 5000) {

    lastFaceTime = millis();

    faceState++;

    if (faceState > 4) {
      faceState = 0;
    }
  }

  if (menuOpen) {

    drawMenu();

  } else {

    drawMainScreen();
  }

  delay(50);
}
