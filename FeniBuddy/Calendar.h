#pragma once
#include "GoogleRoots.h"
#include "Meeting.h"
struct Event { char id[25], title[73]; uint32_t start,end; bool allDay; uint16_t colour; };
Event events[8] = {};
uint8_t eventCount=0;
uint32_t calendarFetched=0, calendarAttemptAt=0;
bool calendarAttempted=false;
String calendarMessage="Calendar not linked";
Event reminderEvent={};
struct Seen { char id[25]; uint32_t start; uint8_t stages; };
Seen seen[16]={};
uint8_t seenCursor=0;
int meetingIndex=-1;
char meetingId[25]={};
uint32_t meetingStart=0;

#include "BoundedBody.h"

String cleanText(const String &value,size_t limit) {
  String result;
  for(size_t i=0;i<value.length() && result.length()<limit;++i)
    if(uint8_t(value[i])>=32 && uint8_t(value[i])<=126) result+=value[i];
  return result;
}
bool safeKey(const String &value,size_t minimum,size_t maximum) {
  if(value.length()<minimum || value.length()>maximum) return false;
  for(size_t i=0;i<value.length();++i)
    if(!isalnum(uint8_t(value[i])) && value[i]!='_' && value[i]!='-') return false;
  return true;
}
bool calendarFresh() { return calendarFetched && epochNow() && epochNow()>=calendarFetched && epochNow()-calendarFetched<=900; }
void updateMeeting() {
  // A cached active meeting may finish offline. Do not start another from stale data.
  int next=buddy::activeMeeting(events,eventCount,epochNow(),calendarFresh()?nullptr:meetingId,meetingStart);
  if(next<0) { meetingId[0]=0;meetingStart=0;ui.meetingDismissed=false; }
  else if(strcmp(meetingId,events[next].id) || meetingStart!=events[next].start) {
    strlcpy(meetingId,events[next].id,sizeof(meetingId));meetingStart=events[next].start;ui.meetingDismissed=false;
    ui.wakeForMeeting(millis());
  }
  meetingIndex=next;ui.meetingActive=next>=0;
}

bool parseCalendar(const char *body) {
  DynamicJsonDocument doc(4096);
  if(deserializeJson(doc,body) || doc["ok"]!=true || !doc["events"].is<JsonArray>()) {
    calendarMessage="Check deployment / key";return false;
  }
  uint32_t fetched=doc["fetchedAt"] | 0UL, now=epochNow();
  JsonArray rows=doc["events"].as<JsonArray>();
  if(!now || fetched>now+300 || now>fetched+900 || rows.size()>8) {calendarMessage="Invalid calendar time";return false;}
  Event candidate[8]={}; uint8_t count=0;
  for(JsonObject row:rows) {
    const char *id=row["id"] | "", *title=row["title"] | "";
    if(!safeKey(id,1,24) || !strlen(title) || strlen(title)>72 || !row["start"].is<uint32_t>() || !row["end"].is<uint32_t>()) {
      calendarMessage="Invalid calendar event";return false;
    }
    Event &event=candidate[count++];
    strlcpy(event.id,id,sizeof(event.id));cleanText(title,72).toCharArray(event.title,sizeof(event.title));
    event.start=row["start"];event.end=row["end"];event.allDay=row["allDay"] | false;
    event.colour=buddy::parseColour(row["color"] | "");
    if(event.start<1700000000 || event.end<=event.start) {calendarMessage="Invalid event dates";return false;}
  }
  for(int i=0;i<count;++i) for(int j=i+1;j<count;++j) if(candidate[j].start<candidate[i].start) {
    Event temp=candidate[i];candidate[i]=candidate[j];candidate[j]=temp;
  }
  memcpy(events,candidate,sizeof(events));eventCount=count;ui.eventCount=count;
  if(ui.page==buddy::Page::Calendar && ui.choice>=count) ui.choice=0;
  calendarFetched=fetched;calendarMessage="Calendar synced";
  if(ui.reminder) {
    bool exists=false;
    for(int i=0;i<count;++i) if(!strcmp(events[i].id,reminderEvent.id) && events[i].start==reminderEvent.start) {
      reminderEvent=events[i];exists=true;
    }
    if(!exists) ui.reminder=false;
  }
  return true;
}

bool fetchCalendarBody(BoundedBody &body) {
  BearSSL::X509List anchors(GOOGLE_ROOTS);
  BearSSL::WiFiClientSecure client;
  client.setTrustAnchors(&anchors);client.setTimeout(5000);
  HTTPClient http;http.setTimeout(5000);http.setReuse(false);http.useHTTP10(true);
  String url="https://script.google.com/macros/s/"+String(settings.deployment)+"/exec?key="+String(settings.calendarKey);
  const char *headers[]={"Location"};
  for(int attempt=0;attempt<3;++attempt) {
    if(!http.begin(client,url)) {calendarMessage="Calendar connection failed";return false;}
    http.collectHeaders(headers,1);
    int code=http.GET();
    if(code==302 || code==303 || code==301 || code==307 || code==308) {
      String location=http.header("Location");http.end();
      // Never forward the key to a login page, another server, or plain HTTP.
      if(!location.startsWith("https://script.googleusercontent.com/")) {calendarMessage="Deployment needs access";return false;}
      url=location;continue;
    }
    if(code!=200) {
      int sslError=client.getLastSSLError();http.end();
      calendarMessage=code<0 ? "Calendar network "+String(code)+" / TLS "+String(sslError) : "Calendar HTTP "+String(code);return false;
    }
    if(http.getSize()>3072) {http.end();calendarMessage="Calendar response too large";return false;}
    int received=http.writeToStream(&body);http.end();
    if(received<0 || body.overflow || !body.used) {calendarMessage="Calendar body "+String(received)+" / "+String(body.used);return false;}
    return true;
  }
  calendarMessage="Too many calendar redirects";return false;
}
void pollCalendar() {
  if(!ui.online || !settings.deployment[0] || !settings.calendarKey[0]) return;
  if(!epochNow()) {calendarMessage="Waiting for clock sync";return;}
  if(calendarAttempted && millis()-calendarAttemptAt<120000) return;
  calendarAttempted=true;calendarAttemptAt=millis();
  // BearSSL automatically puts its full-size TLS buffers in the secondary IRAM
  // heap. Keep enough DRAM for the TLS stack, certificate validation and JSON.
  bool memoryReady=ESP.getFreeHeap()>=32000 && ESP.getMaxFreeBlockSize()>=18000;
#ifdef MMU_IRAM_HEAP
  bool dramReady=ESP.getFreeHeap()>=20000 && ESP.getMaxFreeBlockSize()>=8000;
  { HeapSelectIram iram; memoryReady=dramReady && ESP.getMaxFreeBlockSize()>=18000; }
#endif
  if(!memoryReady) {calendarMessage="Low memory; retry later";return;}
  integrationBusy=true;calendarMessage="Syncing calendar...";
  // Allocate on heap rather than the ESP8266's small stack; release TLS before parsing.
  std::unique_ptr<BoundedBody> body(new(std::nothrow) BoundedBody);
  if(!body) calendarMessage="Low memory; retry later";
  else if(fetchCalendarBody(*body)) parseCalendar(body->data);
  integrationBusy=false;
}
void handleCalendarReminders() {
  uint32_t now=epochNow();
  if(ui.reminder && millis()-reminderAt>=15000) ui.reminder=false;
  if(!calendarFresh() || ui.reminder || ui.timerDone) return;
  for(int i=0;i<eventCount;++i) {
    Event &event=events[i];if(event.allDay || event.end<=now) continue;
    uint8_t stage=0;
    if(event.start>now && event.start-now<=600) stage=1;
    else if(now>=event.start && now-event.start<=120) stage=2;
    if(!stage) continue;
    int slot=-1;
    for(int j=0;j<16;++j) if(!strcmp(seen[j].id,event.id) && seen[j].start==event.start) {slot=j;break;}
    if(slot<0) {slot=seenCursor++%16;seen[slot]={};strlcpy(seen[slot].id,event.id,25);seen[slot].start=event.start;}
    if(!(seen[slot].stages&stage)) {
      seen[slot].stages|=stage;reminderEvent=event;ui.reminder=true;reminderAt=millis();return;
    }
  }
}
