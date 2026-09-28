#pragma once
#include <stdint.h>
namespace buddy {
enum class PcActivity : uint8_t { Neutral, Gaming, Coding, Custom };
struct PcState {
  static constexpr uint32_t Timeout=45000, IntroDuration=4200;
  bool connected=false, introPending=false, introPlaying=false;
  bool foreground=false;
  PcActivity activity=PcActivity::Neutral;
  uint32_t seenAt=0, connectedAt=0, introAt=0;
  uint32_t introDuration=IntroDuration;
  uint8_t customSlot=7;
  bool keepsAwake(uint32_t now) const {return connected && foreground && activity!=PcActivity::Neutral && uint32_t(now-seenAt)<Timeout;}
  void receive(PcActivity next,uint32_t now,bool inForeground=true) {
    if(!connected || uint32_t(now-seenAt)>=Timeout) {connectedAt=now;introPending=true;introPlaying=false;}
    connected=true;seenAt=now;activity=next;foreground=inForeground;
  }
  void update(uint32_t now,bool mayAnimate) {
    if(connected && uint32_t(now-seenAt)>=Timeout) {connected=false;activity=PcActivity::Neutral;introPending=introPlaying=false;}
    if(introPending && uint32_t(now-connectedAt)>30000) introPending=false;
    if(introPending && mayAnimate) {introPending=false;introPlaying=true;introAt=now;}
    if(introPlaying && uint32_t(now-introAt)>=introDuration) introPlaying=false;
  }
};
}
