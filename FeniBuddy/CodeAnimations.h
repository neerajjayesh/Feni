#pragma once
#include "FaceMotion.h"
#include "CustomAnimations.h"
constexpr uint8_t CodeSlots=15;
int activeCodeSlot=-1,previewCodeSlot=-1;
uint32_t codeStartedAt=0,previewCodeUntil=0;
void beginCodeAnimations(){ui.startupDuration=userAnimationDuration(0);ui.wakeDuration=userAnimationDuration(3);pc.introDuration=userAnimationDuration(1);}
int desiredCodeSlot(uint32_t now){
  if(ui.facePhase==buddy::FacePhase::Startup)return 0;
  if(ui.facePhase==buddy::FacePhase::Waking)return 3;
  if(ui.sleeping())return 2;
  if(previewCodeSlot>=0 && int32_t(previewCodeUntil-now)>0)return previewCodeSlot;
  previewCodeSlot=-1;
  if(pc.connected && pc.introPlaying)return 1;
  if(pc.connected && pc.activity==buddy::PcActivity::Gaming)return 4;
  if(pc.connected && pc.activity==buddy::PcActivity::Coding)return 5;
  if(pc.connected && pc.activity==buddy::PcActivity::Custom)return pc.customSlot;
  return 6;
}
bool drawCodeAnimation(uint32_t now){
  int slot=desiredCodeSlot(now);
  if(slot!=activeCodeSlot){activeCodeSlot=slot;codeStartedAt=now;}
  bool preview=previewCodeSlot==slot && int32_t(previewCodeUntil-now)>0;
  if(!hasUserAnimation(slot)&&!preview)return false;
  canvas.clearDisplay();uint32_t elapsed=now-codeStartedAt;
  if(preview && userAnimationDuration(slot))elapsed%=userAnimationDuration(slot);
  if(drawUserAnimation(canvas,slot,elapsed,now)){canvas.display();return true;}
  if(!preview)return false;
  // Preview existing coded behaviours without changing the saved mode or activity.
  if(slot==0||slot==2||slot==3)buddy::drawFacePhase(canvas,slot==0?buddy::FacePhase::Startup:slot==2?buddy::FacePhase::Sleeping:buddy::FacePhase::Waking,elapsed,now);
  else {
    int height=(elapsed%4000<150)?4:38;
    int centerY=(slot==4||slot==7)?19:64;
    canvas.fillRoundRect(36,centerY-height/2,34,height,6,1);canvas.fillRoundRect(90,centerY-height/2,34,height,6,1);
    buddy::PcState demo;demo.connected=true;demo.activity=slot==4?buddy::PcActivity::Gaming:slot==5?buddy::PcActivity::Coding:slot==7?buddy::PcActivity::Custom:buddy::PcActivity::Neutral;
    demo.introPlaying=slot==1;demo.introAt=codeStartedAt;buddy::drawPcDetails(canvas,demo,now,elapsed);
  }
  canvas.display();return true;
}
