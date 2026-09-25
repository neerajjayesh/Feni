#include <assert.h>
#include <stdio.h>
#include "../BuddyCore.h"
using namespace buddy;
unsigned checks=0;
#define CHECK(value) do {++checks;assert(value);} while(0)

void arm(Touch &touch,uint32_t at=0) {
  CHECK(touch.poll(false,at)==Gesture::None);
  CHECK(touch.poll(false,at+600)==Gesture::None);
  CHECK(touch.ready);
}
void controls() {
  Touch sensor;CHECK(sensor.poll(true,0)==Gesture::None);CHECK(sensor.poll(true,5000)==Gesture::None);CHECK(!sensor.ready);
  arm(sensor,5100);
  CHECK(sensor.poll(true,6000)==Gesture::None);CHECK(sensor.poll(true,6030)==Gesture::None);
  CHECK(sensor.poll(false,6100)==Gesture::None);CHECK(sensor.poll(false,6130)==Gesture::None);
  CHECK(sensor.poll(false,6480)==Gesture::None);CHECK(sensor.poll(false,6481)==Gesture::Tap);
  CHECK(sensor.poll(false,7000)==Gesture::None);

  Touch dual;arm(dual);
  dual.poll(true,1000);dual.poll(true,1030);dual.poll(false,1100);dual.poll(false,1130);
  dual.poll(true,1250);CHECK(dual.poll(true,1280)==Gesture::None);
  dual.poll(false,1340);CHECK(dual.poll(false,1370)==Gesture::DoubleTap);
  CHECK(dual.poll(false,2000)==Gesture::None); // no accidental first tap

  Touch held;arm(held);
  held.poll(true,1000);held.poll(true,1030);
  CHECK(held.poll(true,1929)==Gesture::None);CHECK(held.poll(true,1930)==Gesture::Hold);
  CHECK(held.poll(true,8000)==Gesture::None);held.poll(false,8100);CHECK(held.poll(false,8130)==Gesture::None);
  CHECK(held.poll(false,9000)==Gesture::None); // hold release never becomes a tap

  Touch secondHold;arm(secondHold);
  secondHold.poll(true,1000);secondHold.poll(true,1030);secondHold.poll(false,1100);secondHold.poll(false,1130);
  secondHold.poll(true,1250);secondHold.poll(true,1280);
  CHECK(secondHold.poll(true,2180)==Gesture::Hold);secondHold.poll(false,2300);secondHold.poll(false,2330);
  CHECK(secondHold.poll(false,3000)==Gesture::None);

  Touch bounce;arm(bounce);
  bounce.poll(true,1000);bounce.poll(false,1010);CHECK(bounce.poll(false,1500)==Gesture::None);

  Touch wrap;arm(wrap,0xfffff000UL);
  wrap.poll(true,0xfffffff0UL);wrap.poll(true,20);CHECK(wrap.poll(true,920)==Gesture::Hold);

  Touch slow;arm(slow);
  slow.poll(true,1000);slow.poll(true,1030);slow.poll(false,1100);slow.poll(false,1130);
  slow.poll(true,1600);CHECK(slow.poll(true,1630)==Gesture::Tap);
  slow.poll(false,1700);slow.poll(false,1730);CHECK(slow.poll(false,2081)==Gesture::Tap);
}

void menus() {
  Ui ui;CHECK(!ui.online);CHECK(!ui.showClock());
  ui.handle(Gesture::Tap,100);CHECK(ui.showClock());ui.update(10099);CHECK(ui.peek);ui.update(10100);CHECK(!ui.peek);
  ui.handle(Gesture::Hold,20000);CHECK(ui.page==Page::Menu);CHECK(ui.menu==0);
  ui.handle(Gesture::Hold,21000);CHECK(ui.page==Page::Calendar);
  ui.handle(Gesture::DoubleTap,22000);CHECK(ui.page==Page::Menu);
  ui.handle(Gesture::Tap,23000);CHECK(ui.menu==1);
  ui.handle(Gesture::Hold,24000);CHECK(ui.page==Page::Modes);CHECK(ui.choice==2);
  ui.handle(Gesture::Tap,25000);CHECK(ui.choice==0);ui.handle(Gesture::Hold,26000);
  CHECK(ui.mode==Mode::Buddy);CHECK(ui.modeChanged);CHECK(ui.page==Page::Home);
  ui.online=true;ui.handle(Gesture::Tap,27000);CHECK(!ui.showClock());
  ui.mode=Mode::Clock;CHECK(ui.showClock());ui.handle(Gesture::Tap,28000);CHECK(ui.showClock());
  ui.online=false;CHECK(!ui.showClock());ui.handle(Gesture::Tap,29000);CHECK(ui.showClock());
  ui.handle(Gesture::DoubleTap,29500);CHECK(!ui.showClock());
  ui.online=true;ui.mode=Mode::Auto;ui.handle(Gesture::Tap,30000);CHECK(ui.showClock());
  ui.handle(Gesture::Hold,30100);CHECK(ui.page==Page::Menu);CHECK(!ui.peek);
  ui.handle(Gesture::DoubleTap,30200);CHECK(ui.page==Page::Home);
  // Double back preserves each submenu's highlighted parent and does not select.
  for(unsigned index=0;index<MenuCount;++index) {
    ui.page=Page::Menu;ui.menu=index;ui.handle(Gesture::Hold,40000);
    CHECK(int(ui.page)==int(Page::Calendar)+index);ui.handle(Gesture::DoubleTap,41000);
    CHECK(ui.page==Page::Menu);CHECK(ui.menu==index);
  }
}

void timersAndOverlays() {
  Ui ui;ui.page=Page::Timers;ui.handle(Gesture::Tap,0);CHECK(ui.choice==1);
  ui.handle(Gesture::Tap,100);CHECK(ui.choice==2);ui.handle(Gesture::Tap,200);CHECK(ui.choice==3);
  ui.handle(Gesture::Tap,300);CHECK(ui.choice==0);
  ui.handle(Gesture::Hold,1000);CHECK(ui.timerRunning);CHECK(ui.secondsLeft(1000)==600);
  CHECK(ui.secondsLeft(1001)==600);CHECK(ui.secondsLeft(2000)==599);
  ui.handle(Gesture::DoubleTap,2100);CHECK(ui.page==Page::Menu);CHECK(ui.timerRunning);
  ui.update(600999);CHECK(!ui.timerDone);ui.update(601000);CHECK(ui.timerDone);CHECK(!ui.timerRunning);
  ui.handle(Gesture::DoubleTap,602000);CHECK(!ui.timerDone);CHECK(ui.page==Page::Menu);
  ui.reminder=true;ui.handle(Gesture::DoubleTap,603000);CHECK(!ui.reminder);CHECK(ui.page==Page::Menu);
  ui.reminder=true;ui.timerDone=true;ui.handle(Gesture::DoubleTap,604000);CHECK(!ui.timerDone);CHECK(ui.reminder);
  ui.handle(Gesture::DoubleTap,605000);CHECK(!ui.reminder);
  for(unsigned choice=0;choice<3;++choice) {
    Ui timer;timer.page=Page::Timers;timer.choice=choice;timer.handle(Gesture::Hold,0xffffff00UL);
    uint32_t length=(choice+1)*600000UL;CHECK(timer.timerLength==length);
    timer.update(uint32_t(0xffffff00UL+length-1));CHECK(timer.timerRunning);
    timer.update(uint32_t(0xffffff00UL+length));CHECK(timer.timerDone);
    timer.update(uint32_t(0xffffff00UL+length+15000));CHECK(!timer.timerDone);
  }
  ui.page=Page::Timers;ui.handle(Gesture::Hold,610000);CHECK(ui.timerRunning);ui.handle(Gesture::Hold,611000);CHECK(!ui.timerRunning);
  ui.page=Page::Wled;ui.presetCount=3;ui.choice=0;
  ui.handle(Gesture::Tap,612000);CHECK(ui.choice==1);CHECK(ui.requestedPreset==-1);
  ui.handle(Gesture::Hold,613000);CHECK(ui.requestedPreset==1);
  ui.requestedPreset=-1;ui.handle(Gesture::DoubleTap,614000);CHECK(ui.requestedPreset==-1);CHECK(ui.page==Page::Menu);
  Ui empty;empty.page=Page::Wled;empty.handle(Gesture::Hold,1);CHECK(empty.requestedPreset==-1);
}

void customTimer() {
  Ui ui;ui.page=Page::Timers;ui.choice=3;
  ui.handle(Gesture::Hold,1000);CHECK(ui.page==Page::CustomTimer);CHECK(ui.customMinutes==25);
  ui.handle(Gesture::Tap,2000);CHECK(ui.customMinutes==125);
  ui.handle(Gesture::Tap,3000);CHECK(ui.customMinutes==25);
  ui.handle(Gesture::Hold,4000);CHECK(ui.customField==1);
  ui.handle(Gesture::Tap,5000);CHECK(ui.customMinutes==35);
  ui.handle(Gesture::Hold,6000);ui.handle(Gesture::Tap,7000);CHECK(ui.customMinutes==36);
  ui.handle(Gesture::Hold,8000);CHECK(ui.customField==3);CHECK(!ui.timerRunning);
  ui.handle(Gesture::Hold,9000);CHECK(ui.timerRunning);CHECK(ui.timerLength==2160000);CHECK(ui.page==Page::Timers);
  CHECK(!ui.startTimer(10,10000));CHECK(ui.timerLength==2160000);
  ui.cancelTimer();CHECK(!ui.timerRunning);
  ui.handle(Gesture::Hold,11000);CHECK(ui.page==Page::CustomTimer);
  ui.handle(Gesture::DoubleTap,12000);CHECK(ui.page==Page::Timers && ui.choice==3);
  ui.handle(Gesture::Hold,13000);ui.customMinutes=0;ui.customField=3;ui.handle(Gesture::Hold,14000);
  CHECK(!ui.timerRunning);CHECK(ui.customInvalid);CHECK(ui.customField==0);
  ui.customMinutes=181;ui.customField=3;ui.handle(Gesture::Hold,15000);CHECK(!ui.timerRunning);CHECK(ui.customInvalid);
  ui.customMinutes=180;ui.customField=3;ui.handle(Gesture::Hold,16000);CHECK(ui.timerLength==10800000);
  ui.cancelTimer();CHECK(!ui.startTimer(0,17000));CHECK(!ui.startTimer(181,17000));CHECK(ui.startTimer(1,17000));
  ui.update(76999);CHECK(ui.timerRunning);ui.update(77000);CHECK(ui.timerDone);
  Ui wrapped;CHECK(wrapped.startTimer(180,0xfffff000UL));
  wrapped.update(uint32_t(0xfffff000UL+10800000));CHECK(wrapped.timerDone);
}

void wifiSettings() {
  Ui ui;ui.page=Page::Menu;ui.menu=4;ui.online=true;
  ui.handle(Gesture::Hold,1000);CHECK(ui.page==Page::Settings);
  ui.handle(Gesture::Hold,2000);CHECK(ui.page==Page::WifiInfo);
  ui.handle(Gesture::DoubleTap,3000);CHECK(ui.page==Page::Settings && ui.choice==0);
  ui.handle(Gesture::Tap,4000);ui.handle(Gesture::Hold,5000);CHECK(ui.page==Page::WifiPassword);
  ui.update(19999);CHECK(ui.page==Page::WifiPassword);ui.update(20000);CHECK(ui.page==Page::Settings && ui.choice==1);
  ui.handle(Gesture::Hold,21000);ui.handle(Gesture::DoubleTap,22000);CHECK(ui.page==Page::Settings && ui.choice==1);
  ui.handle(Gesture::Tap,23000);ui.handle(Gesture::Hold,24000);CHECK(ui.page==Page::ForgetWifi && ui.choice==0);
  ui.handle(Gesture::Hold,25000);CHECK(ui.page==Page::Settings);CHECK(!ui.forgetRequested);CHECK(ui.online);
  ui.handle(Gesture::Hold,26000);ui.handle(Gesture::Tap,27000);ui.handle(Gesture::DoubleTap,28000);
  CHECK(ui.page==Page::Settings && ui.choice==2);CHECK(!ui.forgetRequested);
  ui.handle(Gesture::Hold,29000);ui.handle(Gesture::Tap,30000);ui.handle(Gesture::Hold,31000);
  CHECK(ui.forgetRequested);CHECK(ui.online); // Network/flash work is deferred to the main loop.
  ui.forgetRequested=false;ui.handle(Gesture::DoubleTap,32000);CHECK(ui.page==Page::Menu && ui.menu==4);
  ui.handle(Gesture::Tap,33000);CHECK(ui.menu==0);
  ui.page=Page::Settings;ui.choice=1;ui.handle(Gesture::Hold,0xfffffff0UL);
  ui.update(uint32_t(0xfffffff0UL+14999));CHECK(ui.page==Page::WifiPassword);
  ui.update(uint32_t(0xfffffff0UL+15000));CHECK(ui.page==Page::Settings);
}
int main() {controls();menus();timersAndOverlays();customTimer();wifiSettings();printf("PASS: %u control, navigation, timer, and settings checks\n",checks);}
