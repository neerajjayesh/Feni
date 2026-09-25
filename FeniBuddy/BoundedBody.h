#pragma once
// HTTPClient removes chunk framing before writing here. Never accept unbounded responses.
class BoundedBody : public Stream {
 public:
  char data[3073]={}; size_t used=0; bool overflow=false;
  size_t write(uint8_t value) override { return write(&value,1); }
  size_t write(const uint8_t *value,size_t length) override {
    if(length>sizeof(data)-1-used) {overflow=true;return 0;}
    memcpy(data+used,value,length);used+=length;data[used]=0;return length;
  }
  // The core's optimized stream copier asks for writable capacity before write().
  // Allow one overflow probe, then stop immediately if a response exceeds the cap.
  int availableForWrite() override {return overflow ? 0 : sizeof(data)-used;}
  bool outputCanTimeout() override {return false;}
  int available() override {return 0;}
  int read() override {return -1;}
  int peek() override {return -1;}
  void flush() override {}
};

