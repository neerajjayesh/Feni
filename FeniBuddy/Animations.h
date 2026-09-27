#pragma once
#include <LittleFS.h>
#include <flash_hal.h>
#include "AnimationFormat.h"
bool animationStorage=false,animationUploadBusy=false;
buddy::AnimationHeader clipHeaders[buddy::AnimationSlots];
bool clipPresent[buddy::AnimationSlots]={};
uint32_t clipSizes[buddy::AnimationSlots]={};
String clipPath(int slot) {return "/feni/"+String(slot)+".fna";}
struct ClipPlayer {
  File file;int slot=-1;uint16_t frame=0;uint32_t frameAt=0;bool shown=false;
  void stop() {file.close();slot=-1;frame=0;shown=false;}
  bool draw(int wanted,uint32_t now) {
    if(wanted<0||wanted>=buddy::AnimationSlots||!clipPresent[wanted]||animationUploadBusy){stop();return false;}
    if(slot!=wanted){stop();file=LittleFS.open(clipPath(wanted),"r");if(!file)return false;file.seek(48,SeekSet);slot=wanted;}
    auto &h=clipHeaders[wanted];canvas.animationPalette(h.colours);
    if(shown && uint32_t(now-frameAt)<1000/h.fps)return true;
    if(frame>=h.frames){file.seek(48,SeekSet);frame=0;}
    if(!buddy::readAnimationFrame(file,[](uint32_t at,uint16_t count,uint8_t colour){canvas.animationRun(at,count,colour);})) {stop();return false;}
    ++frame;frameAt=shown ? frameAt+1000/h.fps : now;shown=true;canvas.display();return true;
  }
} clipPlayer;
void releaseAnimationFrame() {clipPlayer.stop();canvas.releaseFrame();}
void restoreAnimationFrame() {canvas.ensureFrame();}
int previewClip=-1;uint32_t previewUntil=0;
void refreshClip(int slot) {
  clipPresent[slot]=false;clipSizes[slot]=0;
  File file=LittleFS.open(clipPath(slot),"r");uint8_t header[48];
  if(file && file.read(header,48)==48 && buddy::readAnimationHeader(header,clipHeaders[slot])) {clipPresent[slot]=true;clipSizes[slot]=file.size();}
  if(slot==0)ui.startupDuration=clipPresent[0]?clipHeaders[0].duration():buddy::Ui::StartupDuration;
  if(slot==3)ui.wakeDuration=clipPresent[3]?clipHeaders[3].duration():buddy::Ui::WakeDuration;
  if(slot==1)pc.introDuration=clipPresent[1]?clipHeaders[1].duration():buddy::PcState::IntroDuration;
}
void beginAnimationStorage() {
  LittleFS.setConfig(LittleFSConfig(false));animationStorage=LittleFS.begin();
  // Initialize only a fully erased partition. Never format an unknown filesystem.
  if(!animationStorage) {
    bool blank=true;alignas(4) uint32_t block[64];
    uint32_t start=FS_start-0x40200000,end=FS_end-0x40200000;
    if(end<=start||end>ESP.getFlashChipRealSize())blank=false;
    for(uint32_t at=start;blank && at<end;at+=sizeof(block)) {
      if(!ESP.flashRead(at,block,sizeof(block))){blank=false;break;}
      for(auto word:block)if(word!=0xffffffff){blank=false;break;}
      yield();
    }
    if(blank && LittleFS.format())animationStorage=LittleFS.begin();
  }
  if(animationStorage)for(int i=0;i<buddy::AnimationSlots;++i)refreshClip(i);
}
int desiredClip(uint32_t now) {
  if(ui.facePhase==buddy::FacePhase::Startup)return 0;
  if(ui.facePhase==buddy::FacePhase::Waking)return 3;
  if(ui.sleeping())return 2;
  if(previewClip>=0 && int32_t(previewUntil-now)>0)return previewClip;
  previewClip=-1;
  if(pc.connected && pc.introPlaying)return 1;
  if(pc.connected && pc.activity==buddy::PcActivity::Gaming)return 4;
  if(pc.connected && pc.activity==buddy::PcActivity::Coding)return 5;
  if(pc.connected && pc.activity==buddy::PcActivity::Custom)return pc.customSlot;
  return 6;
}
bool validAnimationFile(const String &path,int slot) {
  File file=LittleFS.open(path,"r");uint8_t bytes[48];buddy::AnimationHeader h;
  if(!file||file.size()>buddy::AnimationMaxBytes||file.read(bytes,48)!=48||!buddy::readAnimationHeader(bytes,h))return false;
  if((slot==0||slot==1||slot==3)&&h.duration()>10000)return false;
  for(int i=0;i<h.frames;++i){if(!buddy::readAnimationFrame(file,[](uint32_t,uint16_t,uint8_t){}))return false;yield();}
  return file.position()==file.size();
}
