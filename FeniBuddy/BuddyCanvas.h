#pragma once
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include "Theme.h"
// 2-bit framebuffer: 5 KB, with row accents for event cards. TLS retains its IRAM heap.
class BuddyCanvas : public Adafruit_GFX {
 public:
  explicit BuddyCanvas(Adafruit_ST7735 &panel) : Adafruit_GFX(160,128), panel_(panel) {}
  uint32_t frames=0;
  uint16_t ink=0xffff;
  static constexpr size_t BmpBytes=61494;
  void palette(uint16_t accent, bool face=false) {
    ink=face ? accent : 0xffff;
    for(int y=0;y<128;++y) { accents_[y]=accent; shades_[y]=buddy::shade(accent); }
  }
  void rowColour(int y,int height,uint16_t colour) {
    for(int i=max(0,y);i<min(128,y+height);++i) { accents_[i]=colour; shades_[i]=buddy::shade(colour); }
  }
  void drawPixel(int16_t x,int16_t y,uint16_t colour) override {
    if(x<0 || y<0 || x>=160 || y>=128) return;
    uint8_t shift=(3-(x&3))*2, &pixel=pixels_[y*40+x/4];
    pixel=(pixel&~(3<<shift))|((colour&3)<<shift);
  }
  uint8_t pixel(int x,int y) const { return (pixels_[y*40+x/4]>>((3-(x&3))*2))&3; }
  uint16_t colourAt(int x,int y) const {
    switch(pixel(x,y)) { case 1:return ink;case 2:return accents_[y];case 3:return shades_[y];default:return 0; }
  }
  void fillScreen(uint16_t colour) override { memset(pixels_,(colour&3)*0x55,sizeof(pixels_)); }
  void clearDisplay() { fillScreen(0); }
  void font(uint8_t size=1) {
    setTextSize(1);setTextWrap(false);
    setFont(size>=4?&FreeSans24pt7b:size==3?&FreeSans18pt7b:size==2?&FreeSans12pt7b:&FreeSans9pt7b);
  }
  int width(const String &value,uint8_t size=1) {
    font(size);int16_t x,y;uint16_t w,h;getTextBounds(value,0,0,&x,&y,&w,&h);return w;
  }
  String fit(String value,int limit,uint8_t size=1) {
    if(width(value,size)<=limit) return value;
    while(value.length() && width(value+"...",size)>limit) value.remove(value.length()-1);
    return value+"...";
  }
  void text(int x,int y,const String &value,uint8_t size=1,uint8_t colour=1) {
    font(size);setTextColor(colour);int16_t bx,by;uint16_t w,h;
    getTextBounds(value,0,0,&bx,&by,&w,&h);setCursor(x-bx,y-by);print(value);
  }
  void center(int y,const String &value,uint8_t size=1,uint8_t colour=1) { text(max(0,(160-width(value,size))/2),y,value,size,colour); }
  void small(int x,int y,const String &value,uint8_t colour=1) {
    setFont(nullptr);setTextSize(1);setTextColor(colour);setTextWrap(false);setCursor(x,y);print(value);
  }
  void display() {
    panel_.startWrite();panel_.setAddrWindow(0,0,160,128);
    for(int y=0;y<128;++y) {
      for(int x=0;x<160;++x) line_[x]=__builtin_bswap16(colourAt(x,y));
      SPI.writeBytes(reinterpret_cast<const uint8_t *>(line_),sizeof(line_));
    }
    panel_.endWrite();++frames;yield();
  }
  void writeBmp(WiFiClient &client) {
    uint8_t header[54]={};
    auto put=[&](int i,uint32_t v){for(int n=0;n<4;++n)header[i+n]=(v>>(8*n))&255;};
    header[0]='B';header[1]='M';put(2,BmpBytes);put(10,54);put(14,40);put(18,160);put(22,128);
    header[26]=1;header[28]=24;put(34,61440);client.write(header,sizeof(header));
    uint8_t row[480];
    for(int y=127;y>=0;--y) {
      for(int x=0;x<160;++x) {uint16_t c=colourAt(x,y);row[x*3]=(c&31)*255/31;row[x*3+1]=((c>>5)&63)*255/63;row[x*3+2]=(c>>11)*255/31;}
      client.write(row,sizeof(row));yield();
    }
  }
 private:
  Adafruit_ST7735 &panel_;
  uint8_t pixels_[5120]={};
  uint16_t accents_[128]={},shades_[128]={};
  alignas(4) uint16_t line_[160];
};
