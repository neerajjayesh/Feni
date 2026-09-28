#include <assert.h>
#include <stdio.h>
#include "../FeniAI.h"
struct Canvas {
  unsigned pixels[4]={};
  void fillRect(int x,int y,int w,int h,int colour) {
    assert(x>=0&&y>=0&&x+w<=160&&y+h<=128&&w>0&&h>0);
    assert(colour>=0&&colour<=3);pixels[colour]+=w*h;
  }
};
int main(){
  for(int state=0;state<4;state++)for(unsigned t=0;t<12000;t+=40){
    Canvas c;FeniAI::draw(c,state,t);assert(c.pixels[1]>0);
    assert(c.pixels[0]+c.pixels[1]+c.pixels[2]+c.pixels[3]==(state==0?120*96:state==3?80*64:160*128));
    if(state==1||state==2)assert(c.pixels[2]>0);
    if(state==0)assert(c.pixels[2]==0&&c.pixels[3]==0); // No typing lines/cursor.
  }
  Canvas thinking;FeniAI::draw(thinking,1,13000);
  Canvas attention;FeniAI::draw(attention,2,13001);
  assert(FeniAI::logoX==1&&FeniAI::state==FeniAI::ST_ATTENTION);
  assert(attention.pixels[1]>thinking.pixels[1]);
  Canvas rollover;FeniAI::draw(rollover,0,0xfffffff0);FeniAI::draw(rollover,0,1000);
  puts("PASS: supplied AI scenes, all frame bounds, separate logo/accent palette, transitions and rollover");
}
