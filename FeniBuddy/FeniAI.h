#pragma once
#include <stdint.h>
#include <string.h>
#include <math.h>
// Adapted from the supplied FeniCodeStatus.ino; retain its scene geometry.
// Palette 1 is the fixed orange logo, 2 the current accent, 3 its dim shade.
namespace FeniAI {
constexpr int CELL=4,GW=40,GH=32;
uint8_t cur[GH][GW];
enum : uint8_t { P_BG,P_LOGO,P_ACCENT,P_ALERT,P_CODE_A,P_CODE_B,P_CODE_C,P_DIM,P_RIP };
static inline void px(int x, int y, uint8_t c) {
  if ((unsigned)x < GW && (unsigned)y < GH) cur[y][x] = c;
}
static inline void rectC(int x, int y, int w, int h, uint8_t c) {
  for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) px(x + i, y + j, c);
}

// ------------------------------------------------------------------ helpers
uint32_t hash32(uint32_t x) {
  x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16;
  return x;
}
uint32_t rngState = 2463534242UL;
uint32_t rnd() { rngState ^= rngState << 13; rngState ^= rngState >> 17; rngState ^= rngState << 5; return rngState; }

// ------------------------------------------------------------------ logo
// Native grid 16 x 10 units (matches your image):
//   body  x2..13, y0..7   arms x0..1 / x14..15, y4..5
//   eyes  1x2 holes at x4 and x11, y2..3      legs x3,5,10,12  y8..9
void drawLogo(int ox, int oy, float s, int armL, int armR, uint8_t legLift,
              int eyeTop, int eyeH) {
  auto R = [&](int x, int y, int w, int h, uint8_t c) {
    int left=int(x*s),top=int(y*s);
    rectC(ox+left,oy+top,int(w*s+0.5f),int(h*s+0.5f),c);
  };
  R(2, 0, 12, 8, P_LOGO);                 // body
  R(0, 4 + armL, 2, 2, P_LOGO);           // left arm
  R(14, 4 + armR, 2, 2, P_LOGO);          // right arm
  static const uint8_t legX[4] = {3, 5, 10, 12};
  for (int i = 0; i < 4; i++) R(legX[i], 8, 1, ((legLift >> i) & 1) ? 1 : 2, P_LOGO);
  if (eyeH == -1) {
    for(int ex=3;ex<=10;ex+=7) {
      R(ex,2,1,1,P_BG);R(ex+2,2,1,1,P_BG);R(ex+1,3,1,1,P_BG);
      R(ex,4,1,1,P_BG);R(ex+2,4,1,1,P_BG);
    }
  } else if (eyeH > 0) { R(4, eyeTop, 1, eyeH, P_BG); R(11, eyeTop, 1, eyeH, P_BG); }
}

int blinkPhase(uint32_t t) {              // 0 open, 1 half, 2 closed
  uint32_t m = t % 3600;
  if (m < 90) return 1;
  if (m < 190) return 2;
  if (m < 280) return 1;
  return 0;
}

// ------------------------------------------------------------------ code lines
struct CodeLine { uint8_t indent, n, len[4], col[4]; };
CodeLine cl[3];
uint8_t typed = 0;
uint32_t lastType = 0, holdUntil = 0;

void makeLine(CodeLine &L) {
  L.indent = (rnd() % 4) * 2;
  L.n = 2 + rnd() % 3;                    // 2..4 segments
  for (int i = 0; i < L.n; i++) {
    L.len[i] = 2 + rnd() % 4;             // 2..5 cells
    L.col[i] = P_CODE_A + rnd() % 3;
  }
}
uint8_t lineTotal(const CodeLine &L) {
  uint8_t s = 0;
  for (int i = 0; i < L.n; i++) s += L.len[i] + (i ? 1 : 0);
  return s;
}
void drawLine(const CodeLine &L, int row, int limit) {
  int x = 4 + L.indent, drawn = 0;
  for (int i = 0; i < L.n; i++) {
    for (int k = 0; k < L.len[i]; k++) {
      if (drawn >= limit) return;
      px(x, row, L.col[i]); px(x, row + 1, L.col[i]);
      x++; drawn++;
    }
    x++; drawn++;                         // gap cell
  }
}
void typingStep(uint32_t now) {
  uint8_t total = lineTotal(cl[2]);
  if (typed < total) {
    if (now - lastType >= 65) { typed++; lastType = now; if (typed >= total) holdUntil = now + 600; }
  } else if (int32_t(now - holdUntil) >= 0) {
    cl[0] = cl[1]; cl[1] = cl[2]; makeLine(cl[2]); typed = 0;
  }
}

// ------------------------------------------------------------------ state
enum FeniState : uint8_t { ST_CODING, ST_THINKING, ST_ATTENTION, ST_LIMIT };
FeniState state = ST_CODING;
int logoX = 4;
uint32_t lastMove = 0;

void feniSetState(FeniState s) {
  if (s == state) return;
  bool snap = (s == ST_THINKING || state == ST_THINKING);
  state = s;
  if (snap && s != ST_THINKING) logoX = (s == ST_ATTENTION) ? 1 : 4;
}

// ------------------------------------------------------------------ scenes
void sceneCode(uint32_t now, bool attention) {
  int target = attention ? 1 : 4;
  if (now - lastMove >= 45) {
    lastMove = now;
    if (logoX < target) logoX++; else if (logoX > target) logoX--;
  }

  int armL, armR, eyeTop, eyeH;
  uint8_t legLift = ((now / 420) & 1) ? 0b0101 : 0b1010;
  if (attention) {                        // waving + wide alert eyes
    bool w = (now / 110) & 1;
    armL = w ? -2 : -1;  armR = w ? -1 : -2;
    eyeTop = 1;  eyeH = 3;
  } else {                                // typing + glancing down at the code
    uint32_t h = hash32(now / 110);
    armL = h & 1;  armR = (h >> 1) & 1;
    eyeTop = 3;  eyeH = 2;
    int b = blinkPhase(now);
    if (b == 1) { eyeTop = 4; eyeH = 1; } else if (b == 2) eyeH = 0;
  }
  // Keep the smaller normal creature centered.
  drawLogo(attention?logoX:8, attention?2:7, attention?2.0f:1.5f, armL, armR, legLift, eyeTop, eyeH);

  // The normal AI scene is creature-only; keep code details for attention.
  if (attention) {
    drawLine(cl[0], 24, 255);
    drawLine(cl[1], 27, 255);
    drawLine(cl[2], 30, typed);
    if ((now / 350) & 1) rectC(4 + cl[2].indent + typed, 30, 2, 2, P_ACCENT);
  }

  if (attention) {                        // hopping "!"
    int ey = 8 - (((now / 180) & 1) ? 0 : 1);
    rectC(35, ey, 3, 5, P_ALERT);
    rectC(35, ey + 6, 3, 2, P_ALERT);
  }
}

void sceneThinking(uint32_t now) {
  int bob = (now / 800) & 1;
  uint8_t legLift = ((now / 500) & 1) ? 0b0011 : 0b1100;
  int eyeTop = 1, eyeH = 2;               // looking up at the bubble
  int b = blinkPhase(now);
  if (b == 1) { eyeTop = 2; eyeH = 1; } else if (b == 2) eyeH = 0;
  drawLogo(2, 11 + bob, 1, 0, 0, legLift, eyeTop, eyeH);

  // thought bubble (outline, corners trimmed) + tail
  for (int x = 22; x <= 36; x++) { px(x, 9, P_DIM); px(x, 22, P_DIM); }
  for (int y = 10; y <= 21; y++) { px(21, y, P_DIM); px(37, y, P_DIM); }
  px(20, 17, P_DIM); px(19, 18, P_DIM);

  // 3 bouncing dots in a wave
  for (int i = 0; i < 3; i++) {
    float w = sinf(now * 0.007f - i * 0.8f);
    int up = (w > 0) ? (int)(w * 3.0f + 0.5f) : 0;
    rectC(24 + i * 4, 14 - up, 3, 3, P_ACCENT);
  }
}


// 3x5 pixel "R.I.P." at cell scale sc, top-left (x, y)
void drawRip(int x, int y, int sc, uint8_t col) {
  static const uint8_t gR[5] = {0b110, 0b101, 0b110, 0b101, 0b101};
  static const uint8_t gI[5] = {0b111, 0b010, 0b010, 0b010, 0b111};
  static const uint8_t gP[5] = {0b110, 0b101, 0b110, 0b100, 0b100};
  auto glyph = [&](const uint8_t *g) {
    for (int r = 0; r < 5; r++)
      for (int c = 0; c < 3; c++)
        if (g[r] & (4 >> c)) rectC(x + c * sc, y + r * sc, sc, sc, col);
    x += 4 * sc;
  };
  auto dot = [&]() { rectC(x, y + 4 * sc, sc, sc, col); x += 2 * sc; };
  glyph(gR); dot(); glyph(gI); dot(); glyph(gP); dot();
}

void sceneLimit(uint32_t now) {
  // R.I.P. text: slow pulse between red and dim
  bool bright = sinf(now * 0.003f) > -0.3f;
  drawRip(3, 0, 2, bright ? P_RIP : P_DIM);

  // slumped logo: arms drooping, legs short; tiny twitches now and then
  int armL = ((now % 4300) < 140) ? 1 : 2;
  int armR = 2;
  uint8_t legLift = ((now % 2900) < 130) ? 0b0000 : 0b1111;
  drawLogo(4, 12, 2, armL, armR, legLift, 0, -1);      // eyeH = -1 -> X eyes

  // ground line
  for (int x = 3; x <= 36; x++) px(x, 30, P_DIM);

  // faint specks drifting up on both sides
  static const uint8_t sx[3] = {1, 38, 2};
  for (int i = 0; i < 3; i++) {
    int ph = (now / 260 + i * 5) % 14;
    if (ph < 11) px(sx[i], 26 - ph, P_DIM);
  }
}


template<class Canvas> void draw(Canvas &canvas,uint8_t status,uint32_t now) {
  static bool initialized=false;
  if(!initialized){for(int i=0;i<3;i++)makeLine(cl[i]);initialized=true;}
  feniSetState(status==1?ST_THINKING:status==2?ST_ATTENTION:status==3?ST_LIMIT:ST_CODING);
  memset(cur,0,sizeof(cur));
  if(state==ST_LIMIT)sceneLimit(now);else if(state==ST_THINKING)sceneThinking(now);else sceneCode(now,state==ST_ATTENTION);
  static const uint8_t colours[]={0,1,2,2,2,3,2,3,2};
  // The normal prompting scene uses a centered 120x96 viewport.
  const int cell=state==ST_LIMIT?2:state==ST_CODING?3:CELL;
  const int left=(160-GW*cell)/2,top=(128-GH*cell)/2;
  for(int y=0;y<GH;y++)for(int x=0;x<GW;) {
    uint8_t colour=colours[cur[y][x]];int end=x+1;
    while(end<GW && colours[cur[y][end]]==colour)end++;
    canvas.fillRect(left+x*cell,top+y*cell,(end-x)*cell,cell,colour);x=end;
  }
}
} // namespace FeniAI
