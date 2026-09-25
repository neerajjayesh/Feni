#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Schedule.h>
#include <umm_malloc/umm_heap_select.h>
#include <time.h>
#include <sys/time.h>
#include <stddef.h>
#include "Hardware.h"
#include "BuddyCore.h"
#include "BuddyCanvas.h"
#include "Settings.h"

Adafruit_ST7735 tft(TFT_CS,TFT_DC,TFT_RESET);
BuddyCanvas canvas(tft);
buddy::Touch touch;
buddy::Ui ui;
ESP8266WebServer server(80);
DNSServer dns;
bool apActive=false, lanActive=false, integrationBusy=false;
bool frameContainsPassword=false;
uint32_t lastInputAt=0, reminderAt=0;
uint32_t forgetRequestAt=0;
String networkMessage="Join Feni-Setup";
String wledMessage="Set WLED IP in browser";
String settingsMessage="Hold to open";

#ifdef DEFAULT
#undef DEFAULT
#endif
#include "src/roboeyes/FluxGarage_RoboEyes.h"
#include "Face.h"
RoboEyes<BuddyCanvas> eyes(canvas);

uint32_t epochNow() { time_t value=time(nullptr); return value>1700000000 ? uint32_t(value) : 0; }
void sampleControls() {
  uint32_t now=millis();
  bool active=(TOUCH_ENABLED && digitalRead(TOUCH_PIN)==(TOUCH_ACTIVE_HIGH ? HIGH : LOW)) || digitalRead(FLASH_BUTTON)==LOW;
  buddy::Gesture gesture=touch.poll(active,now);
  if(gesture!=buddy::Gesture::None || active) lastInputAt=now;
  ui.handle(gesture,now); ui.update(now);
}

#include "Calendar.h"
#include "Display.h"
#include "Page.h"
#include "Services.h"

void setup() {
  Serial.begin(115200);
  if(TOUCH_ENABLED) pinMode(TOUCH_PIN,INPUT);
  pinMode(FLASH_BUTTON,INPUT_PULLUP);
  randomSeed(ESP.getCycleCount() ^ micros());
  tft.initR(TFT_TAB);tft.setRotation(TFT_ROTATION);tft.setSPISpeed(16000000);
  eyes.begin(160,128,25);configureCenteredFace(eyes);
  eyes.setAutoblinker(ON,3,3);eyes.open();
  loadConfig();ui.theme=loadTheme();ui.mode=static_cast<buddy::Mode>(settings.mode);ui.presetCount=settings.presetCount;
  if(settings.deployment[0]) calendarMessage="Waiting for first sync";
  if(settings.wledIp[0]) wledMessage="Hold a preset to apply";
  configTime(settings.timezone,"pool.ntp.org","time.google.com");
  startNetwork();renderUi(millis());
  // This callback only samples controls and changes RAM state. No I/O, delay or flash writes.
  // It runs at network yields as well, retaining gestures during HTTPS requests.
  if(!schedule_recurrent_function_us([](){ sampleControls();return true; },10000))
    Serial.println(F("Warning: background input sampler unavailable"));
  Serial.println(F("FeniBuddy 3.2.0 ready. STATUS for diagnostics."));
}

void loop() {
  sampleControls(); handleNetwork();
  server.handleClient(); if(apActive) dns.processNextRequest(); if(lanActive) MDNS.update();
  handleSerial(); sampleControls();
  if(ui.forgetRequested && millis()-forgetRequestAt>=500) {
    ui.forgetRequested=false;
    if(forgetSavedWifi()) {ui.page=buddy::Page::Home;ui.peek=false;}
  }
  if(ui.modeChanged) {
    ui.modeChanged=false;settings.mode=static_cast<uint8_t>(ui.mode);
    settingsDirty=true;settingsDirtyAt=millis();
  }
  if(settingsDirty && millis()-settingsDirtyAt>=1500) {
    if(saveConfig(settings)) settingsDirty=false;
    else {settingsDirtyAt=millis();Serial.println(F("Settings save failed; retrying"));}
  }
  static bool themeDirty=false;static uint32_t themeDirtyAt=0;
  if(ui.themeChanged){ui.themeChanged=false;themeDirty=true;themeDirtyAt=millis();}
  if(themeDirty && millis()-themeDirtyAt>=1500){if(saveTheme(ui.theme))themeDirty=false;else themeDirtyAt=millis();}
#ifdef FENI_TEST_SCENES
  pollCalendarFixture();
#endif
  updateMeeting();handleCalendarReminders(); renderUi(millis());
  if(ui.requestedPreset>=0) {
    int selected=ui.requestedPreset;ui.requestedPreset=-1;applyWledPreset(selected);
  }
  pollWledVerification();
  // Minute timers must not starve Calendar sync; the input sampler keeps time during HTTPS.
  if((ui.page==buddy::Page::Home || (ui.page==buddy::Page::Timers && ui.timerRunning)) && !ui.timerDone && !ui.reminder &&
     !ui.peek && !wledVerifyPending && millis()-lastInputAt>5000 && !touch.down && !touch.pending) pollCalendar();
  yield();
}
