#include <assert.h>
#include <stdio.h>
#include "../BuddyCore.h"
#include "../PcActivity.h"
#include "../FaceMotion.h"
using namespace buddy;
struct Canvas {
  int eyes=0;
  void fillRoundRect(int x,int y,int w,int h,int,int) {
    assert(x==36||x==90);assert(w==34 && h>=2 && h<=38);assert(y+h/2==64);++eyes;
  }
  void small(int,int,const char*,int) {}
  void fillRect(int,int,int,int,int) {}
};
int main() {
  for(int mode=0;mode<3;++mode) {
    Ui ui;ui.online=true;ui.mode=static_cast<Mode>(mode);ui.noteActivity(1000);
    ui.update(300999);assert(!ui.sleeping());ui.update(301000);assert(ui.sleeping());
    ui.noteActivity(301010);assert(ui.sleeping()); // Raw input must not bypass the wake gesture.
    ui.handle(Gesture::Tap,301400);assert(ui.facePhase==FacePhase::Waking && !ui.peek && !ui.showClock());
    ui.startFaceFrame(301400);
    ui.update(302799);assert(ui.animating());ui.update(302800);assert(!ui.animating() && !ui.sleeping() && !ui.showClock());
    ui.handle(Gesture::DoubleTap,303000);assert(ui.peek && ui.showClock());
    ui.handle(Gesture::DoubleTap,303500);assert(!ui.peek);
    ui.handle(Gesture::Hold,304000);assert(ui.page==Page::Menu);
    ui.handle(Gesture::DoubleTap,305000);assert(ui.page==Page::Home);
  }
  Ui autoUi;autoUi.online=true;autoUi.handle(Gesture::Tap,1);assert(!autoUi.peek);
  autoUi.beginStartup(100);autoUi.startFaceFrame(100);assert(autoUi.animating());autoUi.update(1899);assert(autoUi.animating());autoUi.update(1900);assert(!autoUi.animating());
  Ui blocked;blocked.update(300000);blocked.handle(Gesture::Tap,301000);blocked.update(320000);
  assert(blocked.facePhase==FacePhase::Waking);blocked.startFaceFrame(320000);blocked.update(321399);assert(blocked.animating());blocked.update(321400);assert(!blocked.animating());
  Ui wrap;wrap.noteActivity(0xfffffff0);wrap.update(uint32_t(0xfffffff0+300000UL));assert(wrap.sleeping());
  wrap.handle(Gesture::Tap,300500);assert(wrap.facePhase==FacePhase::Waking);
  Ui timer;timer.startTimer(10,0);timer.update(300000);assert(timer.sleeping() && timer.timerRunning);
  timer.update(600000);assert(timer.timerDone && !timer.timerRunning);
  Ui meeting;meeting.update(300000);assert(meeting.sleeping());meeting.meetingActive=true;meeting.wakeForMeeting(301000);assert(meeting.showMeeting());
  PcState pc;pc.receive(PcActivity::Gaming,100);pc.update(100,false);assert(pc.introPending && !pc.introPlaying);
  pc.update(200,true);assert(pc.introPlaying);pc.receive(PcActivity::Coding,300);assert(pc.introAt==200 && !pc.introPending);
  pc.update(4399,true);assert(pc.introPlaying);
  pc.update(4400,true);assert(!pc.introPlaying && pc.connected && pc.activity==PcActivity::Coding);
  pc.update(45299,true);assert(pc.connected);pc.update(45300,true);assert(!pc.connected && pc.activity==PcActivity::Neutral);
  pc.receive(PcActivity::Gaming,46000);assert(pc.introPending);
  pc.receive(PcActivity::Neutral,92000);assert(pc.connectedAt==92000); // Expired without an update.
  pc.receive(PcActivity::Coding,0xfffffff0);pc.update(uint32_t(0xfffffff0+45000UL),true);assert(!pc.connected);
  Ui idle;PcState active;
  for(uint32_t now=0;now<=600000;now+=5000) {active.receive(PcActivity::Coding,now);active.update(now,true);idle.update(now,active.keepsAwake(now));}
  assert(active.connected && !idle.sleeping() && idle.idleBuddy && idle.activityAt==0);
  // Every non-neutral foreground category wakes a sleeping buddy in every mode.
  for(int mode=0;mode<3;mode++)for(int kind=1;kind<=3;kind++) {
    Ui u;u.mode=static_cast<Mode>(mode);u.update(300000);assert(u.sleeping());
    PcState p;p.receive(static_cast<PcActivity>(kind),300010);u.update(300010,p.keepsAwake(300010));
    assert(u.facePhase==FacePhase::Waking && !u.showClock());u.startFaceFrame(300010);
    u.update(301410,p.keepsAwake(301410));assert(!u.sleeping()&&!u.animating());
    p.receive(PcActivity::Neutral,302000);u.update(302000,p.keepsAwake(302000));
    u.update(601999);assert(!u.sleeping());u.update(602000);assert(u.sleeping());
  }
  Ui background;PcState bg;
  for(uint32_t now=0;now<=300000;now+=5000){bg.receive(PcActivity::Gaming,now,false);background.update(now,bg.keepsAwake(now));}
  assert(background.sleeping());bg.receive(PcActivity::Neutral,301000);assert(!bg.keepsAwake(301000));
  active.receive(PcActivity::Custom,700000);idle.update(700000,active.keepsAwake(700000));
  assert(active.keepsAwake(744999)&&!active.keepsAwake(745000));idle.update(745000,active.keepsAwake(745000));
  idle.update(1044999);assert(!idle.sleeping());idle.update(1045000);assert(idle.sleeping());
  active.receive(PcActivity::Gaming,0xfffffff0);assert(active.keepsAwake(20000)&&!active.keepsAwake(45000));
  int previous=0;for(unsigned t=0;t<=1800;t+=20) {int height=openingHeight(t,1600);assert(height>=previous && height<=38);previous=height;}
  Canvas canvas;for(unsigned t=0;t<=1800;t+=20)drawFacePhase(canvas,FacePhase::Startup,t,t);
  drawFacePhase(canvas,FacePhase::Sleeping,0,500000);assert(canvas.eyes==184);
  puts("PASS: sleep/wake in all modes, clock gestures, startup, PC expiry/reconnect, passive heartbeats and centered animation geometry");
}
