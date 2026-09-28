#pragma once
#include "BuddyCore.h"
#include "PcActivity.h"
#include "FeniAnimations-pop-game.h"
#include "FeniPcConnected.h"
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
template<class Canvas> void drawPcDetails(Canvas &c,const PcState &pc,uint32_t now,uint32_t elapsed=0) {
  if(!pc.connected) return;
  if(pc.activity==PcActivity::Gaming) {
    if(!pc.introPlaying)FeniAnim::drawGamer(c,elapsed%8000,8000,1,0);
  } else if(pc.activity==PcActivity::Custom && pc.customSlot==7) {
    if(!pc.introPlaying)FeniAnim::drawPopcorn(c,elapsed%9000,9000,1,0);
  } else if(pc.activity==PcActivity::Coding && !pc.introPlaying) {
    c.drawLine(24,54,17,63,1);c.drawLine(17,63,24,72,1);
    c.drawLine(136,54,143,63,1);c.drawLine(143,63,136,72,1);
    c.drawFastHLine(69,96,12,3);
    if((now/550)%2==0)c.fillRect(84,93,7,3,1);
  }
  if(pc.introPlaying) {
    FeniAnim::EyeGeom eg;eg.lx=53;eg.rx=107;eg.cy=64;eg.w=34;eg.h=38;
    FeniAnim::drawPcConnected(c,elapsed,eg,1,0);
  }
}
}
