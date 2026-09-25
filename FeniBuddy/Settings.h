#pragma once
constexpr size_t EEPROM_BYTES = 4096;
constexpr int CONFIG_OFFSET = 512; // Preserve previous firmware's Wi-Fi and pet slots.
constexpr uint32_t CONFIG_MAGIC = 0x46424D31;
struct Preset { uint8_t id; char name[21]; };
struct Config {
  uint32_t magic;
  uint8_t version, mode, presetCount;
  char ssid[33], password[65], timezone[49], wledIp[16];
  char deployment[193], calendarKey[65];
  Preset presets[6];
  uint32_t checksum;
};
Config settings = {};
void clearWifiCredentials(Config &value) {
  memset(value.ssid, 0, sizeof(value.ssid));
  memset(value.password, 0, sizeof(value.password));
}
bool settingsDirty = false;
uint32_t settingsDirtyAt = 0;
static_assert(CONFIG_OFFSET + sizeof(Config) <= EEPROM_BYTES,"Settings fit EEPROM");
uint32_t configChecksum(const Config &c) {
  uint32_t h=2166136261UL; const uint8_t *p=reinterpret_cast<const uint8_t *>(&c);
  for(size_t i=0;i<offsetof(Config,checksum);++i) h=(h^p[i])*16777619UL;
  return h;
}
bool saveConfig(Config &c) {
  c.magic=CONFIG_MAGIC; c.version=1; c.checksum=configChecksum(c);
  EEPROM.begin(EEPROM_BYTES);
  EEPROM.put(CONFIG_OFFSET,c); bool ok=EEPROM.commit(); EEPROM.end(); return ok;
}
void loadConfig() {
  EEPROM.begin(EEPROM_BYTES); EEPROM.get(CONFIG_OFFSET,settings); EEPROM.end();
  if(settings.magic!=CONFIG_MAGIC || settings.version!=1 || settings.checksum!=configChecksum(settings) ||
     settings.mode>2 || settings.presetCount>6) {
    memset(&settings,0,sizeof(settings)); settings.mode=2; strcpy(settings.timezone,"IST-5:30");
  }
  settings.ssid[32]=settings.password[64]=settings.timezone[48]=settings.wledIp[15]=0;
  settings.deployment[192]=settings.calendarKey[64]=0;
  for(auto &p:settings.presets) p.name[20]=0;
}
