#pragma once
#include <stdint.h>
#include <string.h>
namespace buddy {
// Select the soonest ending active timed event. End is exclusive; all-day events never count down.
template<typename T> int activeMeeting(const T *events, uint8_t count, uint32_t now, const char *keepId=nullptr, uint32_t keepStart=0) {
  int selected=-1;
  for(uint8_t i=0;i<count;++i) {
    const T &e=events[i];
    if(e.allDay || now<e.start || now>=e.end) continue;
    if(keepId && (strcmp(keepId,e.id) || keepStart!=e.start)) continue;
    if(selected<0 || e.end<events[selected].end ||
       (e.end==events[selected].end && strcmp(e.id,events[selected].id)<0)) selected=i;
  }
  return selected;
}
}
