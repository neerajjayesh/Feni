#pragma once
#include <stdint.h>
namespace buddy {
inline uint16_t rgb565(uint32_t rgb) { return ((rgb >> 8) & 0xf800) | ((rgb >> 5) & 0x07e0) | ((rgb >> 3) & 31); }
inline uint16_t accent(uint8_t theme) {
  const uint32_t colours[] = {0x22d3ee,0xff9b42,0x4ade80,0xb388ff};
  return rgb565(colours[theme < 4 ? theme : 0]);
}
inline uint16_t shade(uint16_t c) { return (((c>>11)/5)<<11) | ((((c>>5)&63)/5)<<5) | ((c&31)/5); }
inline bool lightColour(uint16_t c) {
  return ((c >> 11)*255/31)*299 + (((c >> 5)&63)*255/63)*587 + ((c&31)*255/31)*114 > 140000;
}
inline uint16_t parseColour(const char *text) {
  if (!text || *text++ != '#') return 0;
  uint32_t rgb=0;
  for(int i=0;i<6;++i) {
    char c=*text++; int n=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;
    if(n<0) return 0; rgb=(rgb<<4)|n;
  }
  return *text ? 0 : rgb565(rgb);
}
}
