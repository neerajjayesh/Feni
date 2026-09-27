#include <stdio.h>
#include <assert.h>
#include <vector>
#include <fstream>
#include "../AnimationFormat.h"
struct Reader {std::vector<uint8_t> data;size_t at=0;int read(uint8_t *p,size_t n){if(at+n>data.size())return 0;memcpy(p,data.data()+at,n);at+=n;return n;}};
int main(int argc,char **argv) {
  uint8_t bytes[48]={};memcpy(bytes,"FNA1",4);bytes[4]=160;bytes[6]=128;bytes[8]=12;bytes[10]=3;
  buddy::AnimationHeader h;assert(buddy::readAnimationHeader(bytes,h));assert(h.duration()==250);
  bytes[8]=21;assert(!buddy::readAnimationHeader(bytes,h));bytes[8]=12;bytes[12]=1;assert(!buddy::readAnimationHeader(bytes,h));
  Reader r;r.data={3,0,0,0,0,80,15};int count=0;
  assert(buddy::readAnimationFrame(r,[&](uint32_t at,uint16_t n,uint8_t c){assert(at==0&&c==15);count+=n;}));assert(count==20480);
  r.at=0;r.data[4]=1;assert(!buddy::readAnimationFrame(r,[](uint32_t,uint16_t,uint8_t){}));
  r.at=0;r.data[4]=0;r.data[6]=16;assert(!buddy::readAnimationFrame(r,[](uint32_t,uint16_t,uint8_t){}));
  r.at=0;r.data[6]=1;r.data.pop_back();assert(!buddy::readAnimationFrame(r,[](uint32_t,uint16_t,uint8_t){}));
  if(argc>1){std::ifstream f(argv[1],std::ios::binary);Reader file;file.data.assign(std::istreambuf_iterator<char>(f),{});assert(file.read(bytes,48)==48&&buddy::readAnimationHeader(bytes,h));for(int i=0;i<h.frames;i++)assert(buddy::readAnimationFrame(file,[](uint32_t,uint16_t,uint8_t){}));assert(file.at==file.data.size());}
  puts("PASS: FNA1 header, exact frame length, run bounds, palette bounds, truncation and optional C# packer compatibility");
}
