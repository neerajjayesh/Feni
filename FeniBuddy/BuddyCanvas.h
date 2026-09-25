#pragma once
// RoboEyes-compatible monochrome framebuffer: 2.5 KB, leaving room for TLS.
class BuddyCanvas : public GFXcanvas1 {
 public:
  explicit BuddyCanvas(Adafruit_ST7735 &panel) : GFXcanvas1(160,128), panel_(panel) {}
  uint16_t ink = 0x67ff;
  uint32_t frames = 0;
  void clearDisplay() { fillScreen(0); }
  void text(int x, int y, const String &value, uint8_t size = 1) {
    setTextWrap(false); setTextColor(1); setTextSize(size); setCursor(x,y); print(value);
  }
  void center(int y, const String &value, uint8_t size = 1) {
    text(max(0,(160-int(value.length())*6*size)/2),y,value,size);
  }
  void display() {
    if (!getBuffer()) return;
    const uint16_t on = __builtin_bswap16(ink);
    panel_.startWrite(); panel_.setAddrWindow(0,0,160,128);
    for (int y=0;y<128;++y) {
      for (int x=0;x<160;++x) line_[x] = getBuffer()[y*20+x/8] & (0x80>>(x%8)) ? on : 0;
      SPI.writeBytes(reinterpret_cast<const uint8_t *>(line_),sizeof(line_));
    }
    panel_.endWrite(); ++frames; yield();
  }
  void writeBmp(WiFiClient &client) {
    uint8_t header[62] = {};
    auto put = [&](int i,uint32_t v) { for(int n=0;n<4;++n) header[i+n]=(v>>(8*n))&255; };
    header[0]='B';header[1]='M';put(2,2622);put(10,62);put(14,40);put(18,160);put(22,128);
    header[26]=1;header[28]=1;put(34,2560);put(46,2);
    header[58]=(ink&31)*255/31;header[59]=((ink>>5)&63)*255/63;header[60]=(ink>>11)*255/31;
    client.write(header,sizeof(header));
    for(int y=127;y>=0;--y) { client.write(getBuffer()+y*20,20); yield(); }
  }
 private:
  Adafruit_ST7735 &panel_;
  alignas(4) uint16_t line_[160];
};
