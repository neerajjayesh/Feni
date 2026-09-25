#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
struct FakeEEPROM {
  uint8_t flash[4096]={}, staging[4096]={};bool fail=false;
  void begin(size_t) {memcpy(staging,flash,sizeof(flash));}
  void end() {}
  template<typename T> void put(int offset,const T &value) {memcpy(staging+offset,&value,sizeof(value));}
  template<typename T> void get(int offset,T &value) {memcpy(&value,staging+offset,sizeof(value));}
  bool commit() {if(fail)return false;memcpy(flash,staging,sizeof(flash));return true;}
} EEPROM;
#include "../Settings.h"
int main() {
  loadConfig();assert(settings.mode==2 && !strcmp(settings.timezone,"IST-5:30"));
  strcpy(settings.ssid,"Test-network");strcpy(settings.password,"test-password");
  strcpy(settings.calendarKey,"test-calendar-key");strcpy(settings.wledIp,"192.168.1.60");
  settings.mode=1;settings.presetCount=1;settings.presets[0].id=7;strcpy(settings.presets[0].name,"Test preset");
  assert(saveConfig(settings));Config before=settings;
  assert(loadTheme()==0);
  for(uint8_t t=0;t<4;++t){assert(saveTheme(t));assert(loadTheme()==t);loadConfig();assert(!memcmp(&settings,&before,sizeof(Config)));}
  assert(!saveTheme(4));EEPROM.fail=true;assert(!saveTheme(0));assert(loadTheme()==3);EEPROM.fail=false;
  Config candidate=settings;clearWifiCredentials(candidate);
  for(char value:candidate.ssid)assert(value==0);for(char value:candidate.password)assert(value==0);
  assert(candidate.mode==before.mode && !strcmp(candidate.calendarKey,before.calendarKey));
  assert(!memcmp(candidate.presets,before.presets,sizeof(candidate.presets)) && !strcmp(candidate.wledIp,before.wledIp));
  EEPROM.fail=true;assert(!saveConfig(candidate));loadConfig();assert(!strcmp(settings.ssid,"Test-network"));
  EEPROM.fail=false;assert(saveConfig(candidate));loadConfig();assert(!settings.ssid[0] && !settings.password[0]);
  assert(settings.mode==1 && settings.presets[0].id==7 && !strcmp(settings.calendarKey,"test-calendar-key"));
  puts("PASS: Wi-Fi credential removal, preserved preferences, persistence and failed-commit recovery");
}
