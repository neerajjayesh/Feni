#pragma once
// Optional USB-only fixtures, compiled only with -DFENI_TEST_SCENES.
// No Google events or EEPROM settings are changed; the real cache is restored.
struct CalendarFixtureBackup {
  Event cache[8];uint8_t count;uint32_t fetched;String message;
};
std::unique_ptr<CalendarFixtureBackup> fixtureBackup;
uint32_t fixtureUntil=0;
void stopCalendarFixture() {
  if(!fixtureBackup)return;
  memcpy(events,fixtureBackup->cache,sizeof(events));eventCount=fixtureBackup->count;ui.eventCount=eventCount;
  calendarFetched=fixtureBackup->fetched;calendarMessage=fixtureBackup->message;fixtureBackup.reset();
  calendarAttempted=false;ui.reminder=false;ui.choice=0;meetingId[0]=0;meetingStart=0;
}
void startCalendarFixture() {
  stopCalendarFixture();fixtureBackup.reset(new CalendarFixtureBackup);
  memcpy(fixtureBackup->cache,events,sizeof(events));fixtureBackup->count=eventCount;
  fixtureBackup->fetched=calendarFetched;fixtureBackup->message=calendarMessage;
  uint32_t now=epochNow();fixtureUntil=now+45;
  memset(events,0,sizeof(events));
  strcpy(events[0].id,"fixture-day");strcpy(events[0].title,"All day fixture");events[0].start=now-3600;events[0].end=now+3600;events[0].allDay=true;
  strcpy(events[1].id,"fixture-meeting");strcpy(events[1].title,"Design review");events[1].start=now-300;events[1].end=fixtureUntil;events[1].colour=buddy::parseColour("#8e24aa");
  eventCount=ui.eventCount=2;calendarFetched=now;calendarAttempted=true;calendarAttemptAt=millis();
  ui.page=buddy::Page::Home;ui.peek=ui.reminder=ui.meetingDismissed=false;ui.timerDone=false;
}
void pollCalendarFixture() {if(fixtureBackup && epochNow()>=fixtureUntil+2)stopCalendarFixture();}
