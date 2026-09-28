#include <assert.h>
#include <stdio.h>
#include "../FaceMotion.h"
struct Canvas {
  int rounded=0,circles=0,lines=0;
  void fillRoundRect(int,int,int w,int h,int,int colour){assert(w>0&&h>0&&colour>=0&&colour<=3);++rounded;}
  void drawRoundRect(int,int,int,int,int,int){++rounded;}
  void fillCircle(int,int,int r,int colour){assert(r>=0&&colour<=3);++circles;}
  void drawCircle(int,int,int,int){++circles;}
  void drawFastHLine(int,int,int,int){++lines;}
  void drawFastVLine(int,int,int,int){++lines;}
  void drawLine(int,int,int,int,int){++lines;}
  void fillRect(int,int,int,int,int){++lines;}
  void center(int,const char*,int,int){++lines;}
};
int main(){
  Canvas absent;FeniAnim::drawGamer(absent,0,8000);FeniAnim::drawPopcorn(absent,9000,9000);assert(absent.rounded==0&&absent.circles==0);
  buddy::PcState pc;pc.connected=true;pc.activity=buddy::PcActivity::Gaming;
  Canvas game;buddy::drawPcDetails(game,pc,1000,1000);assert(game.rounded>=7&&game.circles>=10);
  pc.activity=buddy::PcActivity::Custom;pc.customSlot=7;Canvas popcorn;buddy::drawPcDetails(popcorn,pc,1000,1000);assert(popcorn.circles>=18&&popcorn.lines>30);
  pc.customSlot=8;Canvas other;buddy::drawPcDetails(other,pc,1000,1000);assert(other.rounded==0&&other.circles==0);
  pc.customSlot=7;pc.introPlaying=true;Canvas intro;buddy::drawPcDetails(intro,pc,1000,1000);assert(intro.circles==0&&intro.lines>0);
  for(unsigned t=0;t<9000;t+=40){Canvas c;FeniAnim::drawGamer(c,t%8000,8000);FeniAnim::drawPopcorn(c,t,9000);}
  puts("PASS: controller/popcorn geometry, cycle envelopes, entertainment routing, unrelated custom slot and PC intro priority");
}
