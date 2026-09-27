#pragma once
#include "BuddyCore.h"
#include "PcActivity.h"
namespace buddy {
inline int openingHeight(uint32_t elapsed,uint32_t duration) {
  if(elapsed>=duration) return 38;
  uint32_t t=elapsed*1000/duration;
  uint32_t smooth=t*t*(3000-2*t)/1000000;
  return 2+36*smooth/1000;
}
template<class Canvas> void drawFacePhase(Canvas &c,FacePhase phase,uint32_t elapsed,uint32_t now) {
  int height=phase==FacePhase::Sleeping ? 2+((now/900)%3==0) : openingHeight(elapsed,phase==FacePhase::Startup?1600:1200);
  int radius=height<10 ? height/2 : 10;
  c.fillRoundRect(36,64-height/2,34,height,radius,1);
  c.fillRoundRect(90,64-height/2,34,height,radius,1);
  if(phase==FacePhase::Sleeping) {
    int drift=(now/600)%4;
    c.small(125,32-drift,"z",3);c.small(136,22-drift,"z",3);
  } else if(phase==FacePhase::Startup) {
    int width=88*(elapsed<1800?elapsed:1800)/1800;
    c.fillRect(36,99,88,2,3);c.fillRect(36,99,width,2,1);
  }
}
template<class Canvas> void drawPcDetails(Canvas &c,const PcState &pc,uint32_t now) {
  if(!pc.connected) return;
  if(pc.activity==PcActivity::Gaming) {
    // Compact headset around, never replacing or shifting, the square eyes.
    c.drawFastHLine(41,32,78,3);c.drawLine(29,44,41,32,3);c.drawLine(119,32,131,44,3);
    c.drawFastVLine(29,44,8,3);c.drawFastVLine(131,44,8,3);
    c.fillRoundRect(27,52,6,23,2,1);c.fillRoundRect(127,52,6,23,2,1);
    c.drawFastVLine(130,76,8,1);c.drawFastHLine(119,83,12,1);c.fillRect(116,81,5,4,1);
  } else if(pc.activity==PcActivity::Coding) {
    c.drawLine(24,54,17,63,1);c.drawLine(17,63,24,72,1);
    c.drawLine(136,54,143,63,1);c.drawLine(143,63,136,72,1);
    c.drawFastHLine(69,96,12,3);
    if((now/550)%2==0)c.fillRect(84,93,7,3,1);
  }
  if(pc.introPlaying) {
    // Three small bolts flow across a track beneath the eyes.
    uint32_t elapsed=now-pc.introAt;
    for(int i=0;i<3;++i) {
      int x=8+((elapsed/18+i*48)%140);
      c.drawLine(x+4,88,x,94,i==1?1:3);c.drawFastHLine(x,94,5,i==1?1:3);c.drawLine(x+4,94,x,100,i==1?1:3);
    }
    c.center(108,"PC connected",1,1);
  }
}
}
