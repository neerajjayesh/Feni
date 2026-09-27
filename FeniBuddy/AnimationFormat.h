#pragma once
#include <stdint.h>
#include <string.h>
namespace buddy {
constexpr uint32_t AnimationMaxBytes=524288, AnimationPixels=160*128;
constexpr uint8_t AnimationSlots=15;
inline uint16_t le16(const uint8_t *p) {return p[0]|uint16_t(p[1])<<8;}
inline uint32_t le32(const uint8_t *p) {return le16(p)|uint32_t(le16(p+2))<<16;}
struct AnimationHeader {
  uint16_t fps=0,frames=0,colours[16]={};
  uint32_t duration() const {return fps ? uint32_t(frames)*1000/fps : 0;}
};
inline bool readAnimationHeader(const uint8_t *p,AnimationHeader &h) {
  if(memcmp(p,"FNA1",4)||le16(p+4)!=160||le16(p+6)!=128||le32(p+12)!=0) return false;
  h.fps=le16(p+8);h.frames=le16(p+10);
  if(h.fps<1||h.fps>20||h.frames<1||h.frames>300||h.frames>h.fps*30) return false;
  for(int i=0;i<16;++i)h.colours[i]=le16(p+16+i*2);
  return true;
}
template<class Reader,class Pixel> bool readAnimationFrame(Reader &reader,Pixel pixel) {
  uint8_t size[4];if(reader.read(size,4)!=4)return false;
  uint32_t length=le32(size);if(!length||length%3||length>AnimationPixels*3)return false;
  uint32_t written=0;
  // Bounded stack buffer avoids one filesystem access per pixel/run.
  uint8_t bytes[384];
  while(length) {
    uint32_t n=length<sizeof(bytes)?length:sizeof(bytes);
    if(reader.read(bytes,n)!=int(n))return false;
    for(uint32_t i=0;i<n;i+=3) {
      uint16_t count=le16(bytes+i);uint8_t colour=bytes[i+2];
      if(!count||colour>15||written+count>AnimationPixels)return false;
      pixel(written,count,colour);written+=count;
    }
    length-=n;
  }
  return written==AnimationPixels;
}
}
