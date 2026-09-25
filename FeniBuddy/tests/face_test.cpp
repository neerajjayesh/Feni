#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <algorithm>
using byte=uint8_t;
using std::min;using std::max;
unsigned long simulatedNow=0;
unsigned long millis() {return simulatedNow;}
long random(long maximum) {return maximum>0 ? maximum/2 : 0;}
long random(long minimum,long maximum) {return minimum+random(maximum-minimum);}
struct Display {
  void clearDisplay() {}
  void display() {}
  void fillRoundRect(int,int,int,int,int,int) {}
  void fillTriangle(int,int,int,int,int,int,int) {}
};
#include "../src/roboeyes/FluxGarage_RoboEyes.h"
#include "../Face.h"
int main() {
  Display display;RoboEyes<Display> eyes(display);
  eyes.begin(160,128,25);configureCenteredFace(eyes);eyes.setAutoblinker(true,3,3);eyes.open();
  bool blinkSeen=false,openSeen=false;
  for(int frame=0;frame<1000;++frame) {
    simulatedNow+=40;if(frame%100==0)eyes.setMood((frame/100)%2 ? HAPPY : DEFAULT);
    eyes.update();
    assert(!eyes.idle && !eyes.curious && !eyes.hFlicker && !eyes.vFlicker);
    assert(eyes.eyeLx==36 && eyes.eyeRx==90 && eyes.eyeLyNext==45 && eyes.eyeRyNext==45);
    assert((eyes.eyeLx+eyes.eyeRx+eyes.eyeRwidthCurrent)/2==80);
    if(eyes.eyeLheightCurrent<=4)blinkSeen=true;
    if(eyes.eyeLheightCurrent>=37) {openSeen=true;assert(eyes.eyeLy>=45 && eyes.eyeLy<=46);}
  }
  assert(blinkSeen && openSeen);
  puts("PASS: fixed central gaze across 1000 RoboEyes frames, with blinks and expressions");
}
