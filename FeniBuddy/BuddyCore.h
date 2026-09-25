#pragma once
#include <stdint.h>

// Hardware-independent controls. All elapsed times remain valid at millis rollover.
namespace buddy {
enum class Gesture : uint8_t { None, Tap, DoubleTap, Hold };
enum class Page : uint8_t { Home, Menu, Calendar, Modes, Timers, Wled, Settings,
                            WifiInfo, WifiPassword, ForgetWifi, CustomTimer, Wifi, Colours };
enum class Mode : uint8_t { Buddy, Clock, Auto };
constexpr uint8_t MenuCount = 5;
constexpr uint16_t MaxTimerMinutes = 180;

struct Touch {
  static constexpr uint32_t Debounce = 30, DoubleGap = 350, HoldTime = 900;
  bool raw = false, down = false, held = false, pending = false, second = false;
  bool ready = false, idleStarted = false;
  uint32_t changed = 0, pressed = 0, released = 0, idleAt = 0;

  Gesture poll(bool active, uint32_t now) {
    // TTP223 calibrates at power-up. Require a released sensor before accepting input.
    if (!ready) {
      if (active) idleStarted = false;
      else if (!idleStarted) { idleStarted = true; idleAt = now; }
      else if (now - idleAt >= 600) { ready = true; raw = down = false; }
      return Gesture::None;
    }
    if (active != raw) { raw = active; changed = now; }
    Gesture event = Gesture::None;
    if (raw != down && now - changed >= Debounce) {
      down = raw;
      if (down) {
        pressed = now; held = false;
        second = pending && now - released <= DoubleGap;
        if (pending && !second) { pending = false; event = Gesture::Tap; }
      } else if (!held) {
        if (second) { pending = second = false; event = Gesture::DoubleTap; }
        else { pending = true; released = now; }
      }
    }
    if (down && !held && now - pressed >= HoldTime) {
      held = true; pending = second = false; event = Gesture::Hold;
    }
    if (pending && !down && !raw && now - released > DoubleGap) {
      pending = false; event = Gesture::Tap;
    }
    return event;
  }
};

struct Ui {
  Page page = Page::Home;
  Mode mode = Mode::Auto;
  uint8_t menu = 0, choice = 0, eventCount = 0, presetCount = 0;
  bool online = false, peek = false, timerRunning = false, timerDone = false;
  bool reminder = false, modeChanged = false;
  bool forgetRequested = false, customInvalid = false;
  uint8_t theme = 0;
  bool themeChanged = false, meetingActive = false, meetingDismissed = false;
  static constexpr uint32_t IdleTimeout = 60000;
  bool idleBuddy = false;
  uint32_t activityAt = 0, idleEnteredAt = 0;
  uint16_t customMinutes = 25;
  uint8_t customField = 0;
  int8_t requestedPreset = -1;
  uint32_t peekAt = 0, timerAt = 0, timerLength = 0, doneAt = 0, passwordAt = 0;

  void noteActivity(uint32_t now) { activityAt=now;idleBuddy=false; }
  // A newly starting meeting gets one viewing window even if Feni was already idle.
  void wakeForMeeting(uint32_t now) { if(page==Page::Home && mode!=Mode::Clock) noteActivity(now); }
  void update(uint32_t now) {
    if(!idleBuddy && uint32_t(now-activityAt)>=IdleTimeout) {
      page=Page::Home;choice=0;peek=false;idleBuddy=true;idleEnteredAt=now;
      reminder=false;timerDone=false;customInvalid=false;
    }
    if (peek && now - peekAt >= 10000) peek = false;
    if (timerRunning && now - timerAt >= timerLength) {
      timerRunning = false; timerDone = true; doneAt = now;
    }
    if (timerDone && now - doneAt >= 15000) timerDone = false;
    if (page == Page::WifiPassword && now - passwordAt >= 15000) { page = Page::Wifi; choice = 1; }
  }
  bool startTimer(uint16_t minutes, uint32_t now) {
    if (timerRunning || minutes < 1 || minutes > MaxTimerMinutes) return false;
    timerAt = now; timerLength = uint32_t(minutes) * 60000UL;
    timerRunning = true; timerDone = false; peek = false; return true;
  }
  void cancelTimer() { timerRunning = timerDone = false; }
  uint32_t secondsLeft(uint32_t now) const {
    uint32_t elapsed = now - timerAt;
    return timerRunning && elapsed < timerLength ? (timerLength - elapsed + 999) / 1000 : 0;
  }
  bool showClock() const { return !idleBuddy && page == Page::Home && (peek || (online && mode == Mode::Clock)); }
  bool showMeeting() const { return !idleBuddy && page == Page::Home && meetingActive && !meetingDismissed && !showClock(); }
  void back() {
    if (timerDone) { timerDone = false; return; }
    if (reminder) { reminder = false; return; }
    if (page == Page::Home) { if (peek) peek = false; else if (showMeeting()) meetingDismissed = true; return; }
    if (page == Page::WifiInfo) { page = Page::Wifi; choice = 0; return; }
    if (page == Page::WifiPassword) { page = Page::Wifi; choice = 1; return; }
    if (page == Page::ForgetWifi) { page = Page::Wifi; choice = 2; return; }
    if (page == Page::Wifi) { page = Page::Settings; choice = 0; return; }
    if (page == Page::Colours) { page = Page::Settings; choice = 1; return; }
    if (page == Page::CustomTimer) { page = Page::Timers; choice = 3; customInvalid = false; return; }
    if (page == Page::Menu) page = Page::Home;
    else page = Page::Menu;
    choice = 0;
  }
  void handle(Gesture gesture, uint32_t now) {
    if (gesture == Gesture::None) return;
    noteActivity(now);
    if (gesture == Gesture::DoubleTap) { back(); return; }
    if (timerDone) { timerDone = false; return; }
    if (reminder) { reminder = false; return; }
    if (gesture == Gesture::Tap) {
      switch (page) {
        case Page::Home: if (!online || mode == Mode::Auto) { peek = true; peekAt = now; } break;
        case Page::Menu: menu = (menu + 1) % MenuCount; break;
        case Page::Modes: choice = (choice + 1) % 3; break;
        case Page::Timers: if (!timerRunning) choice = (choice + 1) % 4; break;
        case Page::Calendar: if (eventCount) choice = (choice + 1) % eventCount; break;
        case Page::Wled: if (presetCount) choice = (choice + 1) % presetCount; break;
        case Page::Settings: choice = (choice + 1) % 2; break;
        case Page::Wifi: choice = (choice + 1) % 3; break;
        case Page::Colours: choice = (choice + 1) % 4; break;
        case Page::ForgetWifi: choice = (choice + 1) % 2; break;
        case Page::CustomTimer: {
          customInvalid = false;
          if (customField == 3) { customField = 0; break; }
          const uint16_t place = customField == 0 ? 100 : customField == 1 ? 10 : 1;
          uint16_t digit = (customMinutes / place) % 10;
          customMinutes = customMinutes - digit * place + ((digit + 1) % (customField == 0 ? 2 : 10)) * place;
          break;
        }
        case Page::WifiInfo: case Page::WifiPassword: break;
      }
    } else if (gesture == Gesture::Hold) {
      switch (page) {
        case Page::Home: page = Page::Menu; peek = false; break;
        case Page::Menu:
          page = static_cast<Page>(static_cast<uint8_t>(Page::Calendar) + menu);
          choice = page == Page::Modes ? static_cast<uint8_t>(mode) : 0;
          break;
        case Page::Modes: mode = static_cast<Mode>(choice); modeChanged = true; page = Page::Home; break;
        case Page::Timers:
          if (timerRunning) cancelTimer();
          else if (choice == 3) { page = Page::CustomTimer; customField = 0; customInvalid = false; }
          else startTimer((choice + 1) * 10, now);
          break;
        case Page::Wled: if (presetCount) requestedPreset = choice; break;
        case Page::Calendar: break;
        case Page::Settings:
          page = choice == 0 ? Page::Wifi : Page::Colours;
          choice = page == Page::Colours ? theme : 0; break;
        case Page::Colours: theme = choice; themeChanged = true; break;
        case Page::Wifi:
          if (choice == 0) page = Page::WifiInfo;
          else if (choice == 1) { page = Page::WifiPassword; passwordAt = now; }
          else { page = Page::ForgetWifi; choice = 0; }
          break;
        case Page::ForgetWifi:
          if (choice == 1) forgetRequested = true;
          page = Page::Wifi; choice = 2; break;
        case Page::CustomTimer:
          if (customField < 3) ++customField;
          else if (startTimer(customMinutes, now)) { page = Page::Timers; choice = 3; }
          else { customInvalid = true; customField = 0; }
          break;
        case Page::WifiInfo: case Page::WifiPassword: break;
      }
    }
  }
};
}
