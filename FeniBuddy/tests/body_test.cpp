#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <algorithm>
// Match the ESP8266 Stream/Print contract used by HTTPClient's optimized copier.
struct Stream {
  virtual ~Stream() {}
  virtual size_t write(uint8_t)=0;
  virtual size_t write(const uint8_t*,size_t)=0;
  virtual int availableForWrite() {return 0;}
  virtual bool outputCanTimeout() {return true;}
  virtual int available()=0;
  virtual int read()=0;
  virtual int peek()=0;
  virtual void flush()=0;
};
#include "../BoundedBody.h"
size_t copyResponse(Stream &out,const std::string &input) {
  size_t sent=0;
  for(unsigned turns=0;sent<input.size() && turns<100;++turns) {
    int capacity=out.availableForWrite();
    if(!capacity && !out.outputCanTimeout()) break;
    size_t count=std::min(size_t(capacity),std::min(size_t(512),input.size()-sent));
    sent+=out.write(reinterpret_cast<const uint8_t*>(input.data()+sent),count);
  }
  return sent;
}
int main() {
  const std::string json="{\"ok\":true,\"events\":[]}";
  BoundedBody small;assert(copyResponse(small,json)==json.size());
  assert(!small.overflow && !strcmp(small.data,json.c_str()));
  BoundedBody limit;std::string maximum(3072,'x');
  assert(copyResponse(limit,maximum)==maximum.size());assert(!limit.overflow && limit.data[3072]==0);
  BoundedBody large;assert(copyResponse(large,std::string(4000,'x'))<4000);
  assert(large.overflow && large.availableForWrite()==0 && !large.outputCanTimeout());
  assert(large.used<=3072 && large.data[large.used]==0);
  BoundedBody chunks;assert(chunks.write(uint8_t('a'))==1);
  assert(copyResponse(chunks,"bc")==2 && !strcmp(chunks.data,"abc"));
  puts("PASS: HTTP writable-capacity contract, complete JSON, exact limit, overflow and chunked writes");
}
