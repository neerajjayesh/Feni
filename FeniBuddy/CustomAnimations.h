#pragma once
// Feni Studio replaces this header in an isolated build with your saved code.
const char *codeRevision="builtin";
bool hasUserAnimation(uint8_t slot) {return false;}
uint32_t userAnimationDuration(uint8_t slot) {
  switch(slot){case 0:return 1800;case 1:return 4200;case 3:return 1400;default:return 0;}
}
bool drawUserAnimation(BuddyCanvas &canvas,uint8_t slot,uint32_t elapsed,uint32_t now) {return false;}
