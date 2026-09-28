#pragma once
// FeniPcConnected.h
// "PC connected" flash animation for RoboEyes on a 160x128 ST7735.
// Needs FeniAnimations.h (shares the helpers). Non-blocking, same overlay
// hook as the gamer / popcorn animations.
//
// Timeline (4.2 s total):
//   0.00 - 1.30  eyes get a bolt cut-out (pop-in) + crackling spark between eyes
//   1.30 - 1.58  full-screen strobe flash
//   1.58 - 3.90  big bolt + "PC connected" typed out underneath
//   3.90 - 4.20  exit flash, then normal eyes come back

#include "FeniAnimations-pop-game.h"
#include <string.h>

namespace FeniAnim {

constexpr uint32_t PC_CONNECTED_MS = 4200;

// Where the eyes sit on screen. Defaults = RoboEyes default 36x36 eyes, 10px gap,
// centred on 160x128. Change if you use setWidth/setHeight/setSpacebetween/setPosition.
struct EyeGeom {
  int lx = 57;   // left eye centre x
  int rx = 103;  // right eye centre x
  int cy = 64;   // eye centre y
  int w  = 36;   // eye width
  int h  = 36;   // eye height
};

inline uint32_t h32(uint32_t x) {
  x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16;
  return x;
}

// Lightning bolt (same silhouette as your reference), scanline-filled into a
// w x h box at (x, y). Works for any size.
template<class Canvas> inline void fillBolt(Canvas &g, int x, int y, int w, int h, uint16_t col) {
  static const uint8_t px[7] = {32, 83, 61, 99, 24, 47, 1};
  static const uint8_t py[7] = {0, 0, 33, 33, 100, 55, 55};
  if (w < 2 || h < 2) return;
  for (int yy = 0; yy < h; yy++) {
    float v = yy * 100.0f / h;
    float xs[7];
    int n = 0;
    for (int i = 0; i < 7; i++) {
      int j = (i + 1) % 7;
      float y0 = py[i], y1 = py[j];
      if ((y0 <= v && y1 > v) || (y1 <= v && y0 > v))
        xs[n++] = px[i] + (v - y0) * (px[j] - (float)px[i]) / (y1 - y0);
    }
    for (int a = 1; a < n; a++) {                 // tiny insertion sort
      float k = xs[a]; int b = a - 1;
      while (b >= 0 && xs[b] > k) { xs[b + 1] = xs[b]; b--; }
      xs[b + 1] = k;
    }
    for (int k = 0; k + 1 < n; k += 2) {
      int xa = x + (int)(xs[k] * w / 100.0f);
      int xb = x + (int)(xs[k + 1] * w / 100.0f);
      g.drawFastHLine(xa, y + yy, xb - xa + 1, col);
    }
  }
}

template<class Canvas> inline void drawPcConnected(Canvas &g, uint32_t t,
                            const EyeGeom &eg = EyeGeom(),
                            uint16_t FG = 1, uint16_t BG = 0) {
  const int screenWidth = g.width();

  // ---------------------------------------------------------- 1) bolts on eyes
  if (t < 1300) {
    float s = easeOut(t / 300.0f);
    float pop = (t < 300) ? s * (1.0f + 0.25f * sinf(s * 3.14159f)) : 1.0f;  // overshoot
    int hb = (int)(eg.h * 0.62f * pop);
    int wb = (int)(hb * 0.7f);
    if (hb >= 4) {
      fillBolt(g, eg.lx - wb / 2, eg.cy - hb / 2, wb, hb, BG);
      fillBolt(g, eg.rx - wb / 2, eg.cy - hb / 2, wb, hb, BG);
    }

    // crackling spark in the gap between the eyes
    int gx0 = eg.lx + eg.w / 2, gx1 = eg.rx - eg.w / 2;
    if (t > 250 && (t / 55) % 4 != 0) {
      uint32_t seed = h32(t / 55);
      const int segs = 5;
      int px0 = gx0, py0 = eg.cy;
      for (int i = 1; i <= segs; i++) {
        int nx = gx0 + (gx1 - gx0) * i / segs;
        int ny = eg.cy + (i == segs ? 0 : (int)((h32(seed + i) >> 8) % 9) - 4);
        g.drawLine(px0, py0, nx, ny, FG);
        g.drawLine(px0, py0 + 1, nx, ny + 1, FG);
        px0 = nx; py0 = ny;
      }
    }

    // flying specks around the eyes, more of them right before the flash
    int dots = (t > 900) ? 9 : 4;
    for (int i = 0; i < dots; i++) {
      uint32_t s2 = h32((t / 80) * 31 + i);
      int ex = (i & 1) ? eg.rx : eg.lx;
      int ox = (int)(s2 & 0x3F) - 32;
      int oy = (int)((s2 >> 8) & 0x3F) - 32;
      g.fillRect(ex + ox, eg.cy + oy, 2, 2, FG);
    }
    return;
  }

  // ---------------------------------------------------------- 2) entry flash
  if (t < 1580) {
    uint32_t k = (t - 1300) / 70;                 // FG, BG, FG, BG
    g.fillScreen((k & 1) ? BG : FG);
    return;
  }

  // ---------------------------------------------------------- 4) exit flash
  if (t >= 3900) {
    uint32_t k = (t - 3900) / 70;
    if (k == 0)      g.fillScreen(FG);
    else if (k == 1) g.fillScreen(BG);
    return;                                        // k >= 2: normal eyes visible
  }

  // ---------------------------------------------------------- 3) big bolt + text
  uint32_t tb = t - 1580;
  g.fillScreen(BG);

  float s = 0.6f + 0.4f * easeOut(tb / 280.0f);
  int bh = (int)(84 * s);
  int bw = (int)(bh * 0.7f);
  int bx = screenWidth / 2 - bw / 2 + (((tb / 180) % 5 == 0) ? 1 : 0);   // subtle electric jitter
  int by = 4 + (84 - bh) / 2;
  fillBolt(g, bx, by, bw, bh, FG);

  // little energy sparkles (plus signs) blinking around the bolt
  static const int8_t spx[4] = {-52, 52, -48, 50};
  static const int8_t spy[4] = {-30, -20, 22, 30};
  for (int i = 0; i < 4; i++) {
    if (((tb / 150) + i) % 3 == 0) {
      int cx = screenWidth / 2 + spx[i], cy = 46 + spy[i];
      g.drawFastHLine(cx - 3, cy, 7, FG);
      g.drawFastVLine(cx, cy - 3, 7, FG);
    }
  }

  // "PC connected", typed out; bold via 1px double-strike
  static const char msg[] = "PC connected";
  int chars = (tb > 250) ? (int)((tb - 250) / 40) : 0;
  if (chars > 12) chars = 12;
  if (chars > 0) {
    char buf[13];
    memcpy(buf, msg, chars);
    buf[chars] = 0;
    g.setFont(nullptr); // The supplied lettering uses the classic GFX bitmap font.
    g.setTextSize(2);
    g.setTextWrap(false);
    g.setTextColor(FG);
    int x0 = (screenWidth - 12 * 12) / 2;
    g.setCursor(x0, 104);     g.print(buf);
    g.setCursor(x0 + 1, 104); g.print(buf);
  }
}

}  // namespace FeniAnim
