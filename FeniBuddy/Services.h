#pragma once
#include "WledProtocol.h"
uint32_t wifiAttemptAt=0, disconnectedAt=0, connectRequestedAt=0, portalCloseAt=0;
bool connectionPending=false, offlineTracking=false, portalClosing=false, networkConnected=false;
WiFiEventHandler wifiDisconnectedHandler;
uint8_t wifiDisconnectReason=0;
String csrfToken;
bool wledVerifyPending=false;
uint32_t wledVerifyAt=0;
uint8_t wledVerifyTries=0,wledVerifyId=0;
char wledVerifyIp[16]={},wledVerifyName[21]={};

String asJson(JsonDocument &doc) {String output;serializeJson(doc,output);return output;}
String statusJson() {
  DynamicJsonDocument doc(4096);
  doc["name"]="Feni";doc["firmware"]="feni-buddy-3.1.1";
  doc["wifi"]=ui.online;doc["setup"]=apActive;doc["ip"]=WiFi.localIP().toString();doc["networkMessage"]=networkMessage;
  doc["ssid"]=settings.ssid;doc["mode"]=int(ui.mode);doc["page"]=int(ui.page);doc["choice"]=ui.choice;doc["menu"]=ui.menu;
  doc["clock"]=epochNow();doc["timezone"]=settings.timezone;doc["peek"]=ui.peek;
  doc["timerRunning"]=ui.timerRunning;doc["timerSeconds"]=ui.secondsLeft(millis());doc["timerDone"]=ui.timerDone;
  doc["timerDurationSeconds"]=ui.timerLength/1000;doc["customMinutes"]=ui.customMinutes;doc["customField"]=ui.customField;
  doc["menuCount"]=buddy::MenuCount;doc["settingsMessage"]=settingsMessage;
  doc["faceFixed"]=!eyes.idle && !eyes.hFlicker && !eyes.vFlicker;
  doc["eyeLeftX"]=eyes.eyeLx;doc["eyeRightX"]=eyes.eyeRx;doc["eyeTargetY"]=eyes.eyeLyNext;
  doc["heap"]=ESP.getFreeHeap();doc["maxBlock"]=ESP.getMaxFreeBlockSize();doc["frames"]=canvas.frames;
  doc["wifiStatus"]=int(WiFi.status());doc["wifiDisconnectReason"]=wifiDisconnectReason;
  doc["wifiPhyMode"]=int(WiFi.getPhyMode());doc["wifiSleepMode"]=int(WiFi.getSleepMode());
#ifdef MMU_IRAM_HEAP
  uint32_t iramHeap,iramMaxBlock;
  { HeapSelectIram iram;iramHeap=ESP.getFreeHeap();iramMaxBlock=ESP.getMaxFreeBlockSize(); }
  doc["iramHeap"]=iramHeap;doc["iramMaxBlock"]=iramMaxBlock;
#endif
  doc["touchReady"]=touch.ready;doc["touchActive"]=touch.down;doc["uptimeMs"]=millis();
  doc["input"]=TOUCH_ENABLED ? "flash+ttp223" : "flash";doc["touchEnabled"]=TOUCH_ENABLED;
  doc["calendarConfigured"]=settings.deployment[0] && settings.calendarKey[0];doc["calendarFresh"]=calendarFresh();
  doc["calendarMessage"]=calendarMessage;doc["calendarFetched"]=calendarFetched;doc["reminder"]=ui.reminder;
  JsonArray rows=doc.createNestedArray("events");
  for(int i=0;i<eventCount;++i) {JsonObject row=rows.createNestedObject();row["title"]=events[i].title;row["start"]=events[i].start;row["allDay"]=events[i].allDay;}
  doc["wledIp"]=settings.wledIp;doc["wledMessage"]=wledMessage;
  JsonArray presets=doc.createNestedArray("presets");
  for(int i=0;i<settings.presetCount;++i) {JsonObject p=presets.createNestedObject();p["id"]=settings.presets[i].id;p["name"]=settings.presets[i].name;}
  return asJson(doc);
}
bool authorizeWrite() {
  if(server.header("X-Feni-Token")!=csrfToken) {server.send(403,"text/plain","Reload the Feni page and try again.");return false;}
  lastInputAt=millis();
  return true;
}
bool numberArg(const String &name,uint32_t &result) {
  String value=server.arg(name);uint64_t number=0;
  if(!value.length() || value.length()>10) return false;
  for(size_t i=0;i<value.length();++i) {if(value[i]<'0' || value[i]>'9') return false;number=number*10+value[i]-'0';}
  if(number>0xffffffffULL) return false;result=number;return true;
}
bool commitSettings(Config &candidate) {
  if(!saveConfig(candidate)) {server.send(500,"text/plain","Could not save settings. Please retry.");return false;}
  settings=candidate;settingsDirty=false;return true;
}
void startPortal() {
  if(apActive) return;
  WiFi.mode(WIFI_AP_STA);
  apActive=WiFi.softAP(SETUP_SSID,SETUP_PASSWORD,1,false,4);
  if(apActive) dns.start(53,"*",WiFi.softAPIP());
}
void stopPortal() {
  dns.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);apActive=false;portalClosing=false;
}
bool forgetSavedWifi() {
  Config candidate=settings;clearWifiCredentials(candidate);
  if(!saveConfig(candidate)) {settingsMessage="Could not forget; retry";return false;}
  settings=candidate;settingsDirty=false;
  connectionPending=false;portalClosing=false;wledVerifyPending=false;
  WiFi.setAutoReconnect(false);WiFi.disconnect(false);
  ui.online=false;networkConnected=false;
  if(lanActive) {MDNS.close();lanActive=false;}
  startPortal();networkMessage="Wi-Fi forgotten. Join Feni-Setup";
  settingsMessage="Wi-Fi forgotten";return true;
}
void receiveWifi() {
  if(!authorizeWrite()) return;
  String ssid=server.arg("ssid"),password=server.arg("password");
  bool hex=password.length()==64;
  for(size_t i=0;hex && i<password.length();++i) if(!isxdigit(uint8_t(password[i]))) hex=false;
  if(ssid.length()<1 || ssid.length()>32 || password.length()>64 ||
      (password.length()>0 && password.length()<8) || (password.length()==64 && !hex)) {
    server.send(400,"text/plain","Use a Wi-Fi name up to 32 bytes and a valid password (blank for an open network).");return;
  }
  Config candidate=settings;ssid.toCharArray(candidate.ssid,sizeof(candidate.ssid));password.toCharArray(candidate.password,sizeof(candidate.password));
  if(!commitSettings(candidate)) return;
  wledVerifyPending=false;
  server.send(200,"text/plain","Saved. Connecting now. After success, rejoin your home Wi-Fi and open http://feni.local.");
  connectionPending=true;connectRequestedAt=millis();networkMessage="Connecting to "+ssid;
}
void receiveSettings() {
  if(!authorizeWrite()) return;
  Config candidate=settings;
  if(server.hasArg("mode")) {uint32_t value;if(!numberArg("mode",value)||value>2){server.send(400,"text/plain","Invalid mode");return;}candidate.mode=value;}
  if(server.hasArg("timezone")) {
    String tz=server.arg("timezone");bool ok=tz.length()>0 && tz.length()<sizeof(candidate.timezone);
    for(size_t i=0;ok && i<tz.length();++i) if(!isalnum(uint8_t(tz[i])) && String("+-:,.<>/").indexOf(tz[i])<0) ok=false;
    if(!ok) {server.send(400,"text/plain","Enter a POSIX timezone such as IST-5:30 or UTC0.");return;}
    tz.toCharArray(candidate.timezone,sizeof(candidate.timezone));
  }
  if(server.hasArg("wledIp")) {
    String ip=server.arg("wledIp");ip.trim();IPAddress parsed;
    if(ip.length() && (!parsed.fromString(ip) || ip!=parsed.toString() || parsed[0]==0 || parsed[0]>=224 || parsed[3]==255)) {
      server.send(400,"text/plain","Enter the WLED IPv4 address, without http:// or a port.");return;
    }
    ip.toCharArray(candidate.wledIp,sizeof(candidate.wledIp));candidate.presetCount=0;
    for(int i=0;i<6;++i) {
      String field="p"+String(i),id=server.arg(field+"id"),name=cleanText(server.arg(field+"name"),20);if(!id.length()) continue;
      uint32_t number;if(!numberArg(field+"id",number)||number<1||number>250) {server.send(400,"text/plain","Preset IDs must be 1 to 250.");return;}
      for(int j=0;j<candidate.presetCount;++j) if(candidate.presets[j].id==number) {server.send(400,"text/plain","Use each preset ID once.");return;}
      Preset &p=candidate.presets[candidate.presetCount++];p={};p.id=number;
      if(!name.length()) name="Preset "+String(number);name.toCharArray(p.name,sizeof(p.name));
    }
  }
  if(server.arg("clearCalendar")=="1") {candidate.deployment[0]=candidate.calendarKey[0]=0;}
  else if(server.hasArg("calendarUrl") && server.arg("calendarUrl").length()) {
    String url=server.arg("calendarUrl");url.trim();String prefix="https://script.google.com/macros/s/";
    if(!url.startsWith(prefix)||!url.endsWith("/exec")) {server.send(400,"text/plain","Use the Apps Script deployment URL ending in /exec.");return;}
    String deployment=url.substring(prefix.length(),url.length()-5),key=server.arg("calendarKey");
    if(!safeKey(deployment,20,192)||!safeKey(key,32,64)) {server.send(400,"text/plain","Supply the deployment URL and 32–64 character secret key.");return;}
    deployment.toCharArray(candidate.deployment,sizeof(candidate.deployment));key.toCharArray(candidate.calendarKey,sizeof(candidate.calendarKey));
  }
  bool calendarChanged=strcmp(candidate.deployment,settings.deployment)||strcmp(candidate.calendarKey,settings.calendarKey);
  if(!commitSettings(candidate)) return;
  wledVerifyPending=false;
  ui.mode=static_cast<buddy::Mode>(settings.mode);ui.presetCount=settings.presetCount;
  if(ui.page==buddy::Page::Wled && ui.choice>=ui.presetCount) ui.choice=0;
  if(ui.page==buddy::Page::Modes) ui.choice=int(ui.mode);
  configTime(settings.timezone,"pool.ntp.org","time.google.com");
  if(calendarChanged) {
    eventCount=ui.eventCount=0;calendarFetched=0;ui.reminder=false;calendarAttempted=false;memset(seen,0,sizeof(seen));
    calendarMessage=settings.deployment[0] ? "Waiting for first sync" : "Calendar not linked";
  }
  wledMessage=settings.wledIp[0] ? "Hold a preset to apply" : "Set WLED IP in browser";
  server.send(200,"text/plain","Settings saved.");
}
bool wledOnOurNetwork(IPAddress &address) {
  if(!ui.online || !address.fromString(settings.wledIp)) return false;
  IPAddress local=WiFi.localIP(), mask=WiFi.subnetMask();
  bool hostZero=true,hostOne=true,same=true;
  for(int i=0;i<4;++i) {
    if((address[i]&mask[i])!=(local[i]&mask[i])) return false;
    uint8_t host=address[i]&~mask[i];if(host) hostZero=false;if(host!=uint8_t(~mask[i])) hostOne=false;
    if(address[i]!=local[i]) same=false;
  }
  return !hostZero && !hostOne && !same;
}
void applyWledPreset(int selected) {
  wledVerifyPending=false;
  if(selected<0 || selected>=settings.presetCount) return;
  IPAddress address;
  if(!wledOnOurNetwork(address)) {wledMessage="WLED needs same network";return;}
  wledMessage="Sending preset...";renderUi(millis());
  WiFiClient client;client.setTimeout(1500);HTTPClient http;http.setTimeout(1500);http.setReuse(false);
  if(!http.begin(client,"http://"+address.toString()+"/json/state")) {wledMessage="WLED connection failed";return;}
  http.addHeader("Content-Type","application/json");
  String payload="{\"ps\":"+String(settings.presets[selected].id)+"}";
  int code=http.POST(payload);
  if(code!=200) {wledMessage=code<0 ? "WLED unreachable" : "WLED HTTP "+String(code);http.end();return;}
  std::unique_ptr<BoundedBody> body(new(std::nothrow) BoundedBody);
  if(!body || http.getSize()>3072 || http.writeToStream(body.get())<0 || body->overflow) {wledMessage="WLED response error";http.end();return;}
  http.end();
  WledReply reply=readWledReply(body->data);
  if(!reply.parsed || reply.error || (!reply.accepted && reply.preset<0)) {wledMessage="WLED rejected request";return;}
  // WLED loads the preset on a later loop, after replying to this POST.
  wledVerifyPending=true;wledVerifyAt=millis();wledVerifyTries=0;wledVerifyId=settings.presets[selected].id;
  strlcpy(wledVerifyIp,settings.wledIp,sizeof(wledVerifyIp));strlcpy(wledVerifyName,settings.presets[selected].name,sizeof(wledVerifyName));
  wledMessage="Checking preset...";
}
void pollWledVerification() {
  if(!wledVerifyPending || millis()-wledVerifyAt<400) return;
  wledVerifyAt=millis();++wledVerifyTries;
  if(!ui.online) {wledMessage="Wi-Fi disconnected";wledVerifyPending=false;return;}
  WiFiClient client;client.setTimeout(1500);HTTPClient http;http.setTimeout(1500);http.setReuse(false);
  if(!http.begin(client,"http://"+String(wledVerifyIp)+"/json/state")) {wledMessage="WLED connection failed";wledVerifyPending=false;return;}
  int code=http.GET();
  std::unique_ptr<BoundedBody> body(new(std::nothrow) BoundedBody);
  if(code!=200 || !body || http.getSize()>3072 || http.writeToStream(body.get())<0 || body->overflow) {
    http.end();wledMessage="WLED result unavailable";wledVerifyPending=false;return;
  }
  http.end();WledReply reply=readWledReply(body->data);
  if(!reply.parsed || reply.error) {wledMessage="WLED preset error";wledVerifyPending=false;return;}
  if(reply.preset==wledVerifyId) {wledMessage="Applied: "+String(wledVerifyName);wledVerifyPending=false;return;}
  if(wledVerifyTries>=4) {wledMessage="Preset not confirmed";wledVerifyPending=false;}
}
void sendPage() {server.sendHeader("Cache-Control","no-store");server.send_P(200,"text/html; charset=utf-8",BUDDY_PAGE);}
void startNetwork() {
  char token[33];snprintf(token,sizeof(token),"%08x%08x%08x%08x",os_random(),os_random(),os_random(),os_random());csrfToken=token;
  WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.hostname("feni");WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  wifiDisconnectedHandler=WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event){wifiDisconnectReason=event.reason;});
  if(settings.ssid[0]) {WiFi.begin(settings.ssid,settings.password);wifiAttemptAt=millis();networkMessage="Connecting to saved Wi-Fi";}
  else startPortal();
  server.collectHeaders("X-Feni-Token");
  server.on("/",HTTP_GET,sendPage);
  server.on("/session",HTTP_GET,[](){server.sendHeader("Cache-Control","no-store");server.send(200,"text/plain",csrfToken);});
  server.on("/status",HTTP_GET,[](){server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",statusJson());});
  server.on("/wifi",HTTP_POST,receiveWifi);server.on("/settings",HTTP_POST,receiveSettings);
  server.on("/wifi/forget",HTTP_POST,[](){
    if(!authorizeWrite())return;
    server.send(200,"text/plain","Forgetting Wi-Fi. Join Feni-Setup, then open http://192.168.4.1.");
    forgetRequestAt=millis();ui.forgetRequested=true;
  });
  server.on("/timer",HTTP_POST,[](){
    if(!authorizeWrite())return;
    if(server.arg("action")=="cancel") {ui.cancelTimer();server.send(200,"text/plain","Timer cancelled.");return;}
    uint32_t minutes;
    if(!numberArg("minutes",minutes)||minutes<1||minutes>buddy::MaxTimerMinutes) {server.send(400,"text/plain","Choose 1 to 180 minutes.");return;}
    if(!ui.startTimer(minutes,millis())) {server.send(409,"text/plain","Cancel the running timer first.");return;}
    ui.customMinutes=minutes;ui.page=buddy::Page::Timers;ui.choice=3;
    server.send(200,"text/plain","Timer started.");
  });
  server.on("/clock",HTTP_POST,[](){
    if(!authorizeWrite()) return;uint32_t epoch;
    if(!numberArg("epoch",epoch)||epoch<1700000000||epoch>4102444800UL) {server.send(400,"text/plain","Invalid time");return;}
    timeval tv={time_t(epoch),0};settimeofday(&tv,nullptr);server.send(200,"text/plain","Clock synchronized.");
  });
  server.on("/control",HTTP_POST,[](){
    if(!authorizeWrite()) return;String action=server.arg("action");
    buddy::Gesture gesture=action=="tap" ? buddy::Gesture::Tap : action=="hold" ? buddy::Gesture::Hold : action=="back" ? buddy::Gesture::DoubleTap : buddy::Gesture::None;
    if(gesture==buddy::Gesture::None){server.send(400,"text/plain","Unknown control");return;}
    lastInputAt=millis();ui.handle(gesture,lastInputAt);server.send(200,"text/plain","OK");
  });
  server.on("/calendar/sync",HTTP_POST,[](){if(!authorizeWrite())return;calendarAttempted=false;server.send(200,"text/plain","Sync queued. Return Feni to its home screen.");});
  server.on("/wled/apply",HTTP_POST,[](){
    if(!authorizeWrite())return;uint32_t selected;
    if(!numberArg("index",selected)||selected>=settings.presetCount){server.send(400,"text/plain","Invalid preset");return;}
    ui.requestedPreset=selected;server.send(200,"text/plain","Queued. Check WLED status for confirmation.");
  });
  server.on("/frame.bmp",HTTP_GET,[](){
    server.sendHeader("Cache-Control","no-store");
    if(frameContainsPassword || ui.page==buddy::Page::WifiPassword) {server.send(403,"text/plain","Password is visible only on Feni's screen.");return;}
    server.setContentLength(2622);server.send(200,"image/bmp","");WiFiClient client=server.client();canvas.writeBmp(client);
  });
  server.on("/generate_204",HTTP_GET,sendPage);server.on("/hotspot-detect.html",HTTP_GET,sendPage);
  server.on("/connecttest.txt",HTTP_GET,sendPage);server.on("/ncsi.txt",HTTP_GET,sendPage);
  server.onNotFound([](){
    if(apActive && server.method()==HTTP_GET) {server.sendHeader("Location","http://192.168.4.1/",true);server.send(302,"text/plain","");}
    else server.send(404,"text/plain","Not found");
  });server.begin();
}
void handleNetwork() {
  uint32_t now=millis();
  if(connectionPending && now-connectRequestedAt>=500) {
    connectionPending=false;startPortal();portalClosing=false;
    WiFi.disconnect(false);WiFi.setAutoReconnect(true);WiFi.begin(settings.ssid,settings.password);wifiAttemptAt=now;
  }
  bool connected=WiFi.status()==WL_CONNECTED;
  ui.online=connected;
  if(connected) {
    offlineTracking=false;
    if(!networkConnected) {
      networkConnected=true;
      lanActive=MDNS.begin("feni");if(lanActive)MDNS.addService("http","tcp",80);
      networkMessage="Connected: "+WiFi.localIP().toString();Serial.println(networkMessage);
      if(apActive && !portalClosing) {portalClosing=true;portalCloseAt=now;}
      calendarAttempted=false;
    }
    if(apActive && !portalClosing) {portalClosing=true;portalCloseAt=now;}
    if(portalClosing && now-portalCloseAt>=5000) stopPortal();
  } else {
    networkConnected=false;
    portalClosing=false;
    if(lanActive) {MDNS.close();lanActive=false;}
    if(!offlineTracking) {offlineTracking=true;disconnectedAt=now;}
    if(now-disconnectedAt>=20000 || !settings.ssid[0]) startPortal();
    if(settings.ssid[0] && now-wifiAttemptAt>=30000) {
      networkMessage=apActive ? "Could not connect; check Wi-Fi details" : "Reconnecting Wi-Fi";
      WiFi.begin(settings.ssid,settings.password);wifiAttemptAt=now;
    }
  }
}
void handleSerial() {
  static char line[32];static uint8_t used=0;static bool overflow=false;
  for(int budget=0;budget<64 && Serial.available();++budget) {
    char c=Serial.read();if(c=='\r')continue;
    if(c=='\n') {
      line[used]=0;
      if(!overflow && !strcmp(line,"STATUS")) Serial.println(statusJson());
      else if(!overflow && !strcmp(line,"WIFI")) {
        // Scan only reports matches for the configured network; never credentials.
        int count=WiFi.scanNetworks(false,true),matches=0;
        for(int i=0;i<count;++i) if(WiFi.SSID(i)==settings.ssid) {
          ++matches;Serial.printf("Saved network: channel=%d rssi=%d encryption=%d\n",WiFi.channel(i),WiFi.RSSI(i),WiFi.encryptionType(i));
        }
        WiFi.scanDelete();Serial.printf("Wi-Fi status=%d reason=%d matches=%d\n",WiFi.status(),wifiDisconnectReason,matches);
      }
      else if(!overflow && (!strcmp(line,"RECONNECT") || !strcmp(line,"WIFI_G") || !strcmp(line,"WIFI_N"))) {
        WiFi.disconnect(false);WiFi.setAutoReconnect(true);
        if(!strcmp(line,"WIFI_G")) WiFi.setPhyMode(WIFI_PHY_MODE_11G);
        if(!strcmp(line,"WIFI_N")) WiFi.setPhyMode(WIFI_PHY_MODE_11N);
        if(settings.ssid[0]) {WiFi.begin(settings.ssid,settings.password);wifiAttemptAt=millis();}
        Serial.println(F("OK"));
      }
      else if(!overflow && (!strcmp(line,"TAP") || !strcmp(line,"HOLD") || !strcmp(line,"BACK"))) {
        lastInputAt=millis();ui.handle(!strcmp(line,"TAP") ? buddy::Gesture::Tap : !strcmp(line,"HOLD") ? buddy::Gesture::Hold : buddy::Gesture::DoubleTap,lastInputAt);
        Serial.println(F("OK"));
      } else if(used) Serial.println(F("Commands: STATUS, TAP, HOLD, BACK, WIFI, RECONNECT"));
      used=0;overflow=false;
    } else if(used<sizeof(line)-1)line[used++]=c;else overflow=true;
  }
}
