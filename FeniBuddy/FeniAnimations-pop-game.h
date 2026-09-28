#pragma once
// FeniAnimations.h
// Gamer + Popcorn ("watching") overlay animations for RoboEyes.
// Target: 1.8" ST7735, landscape 160x128, drawn into a 1-bit Adafruit_GFX
// canvas (e.g. GFXcanvas1) AFTER the eyes are rendered and BEFORE the canvas
// is pushed to the TFT.
//
// FG = "on" pixel (eye colour), BG = "off" pixel. Shapes are filled with BG
// before outlining so they cleanly cover whatever is behind them.
// Both animations are non-blocking: call every frame with elapsed time.

#include <stdint.h>
// Adapted to a canvas template; drawing and timings are from the supplied file.
#include <math.h>

namespace FeniAnim {

// ------------------------------------------------------------ helpers
inline float clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }
inline float easeOut(float x) { x = 1.0f - clamp01(x); return 1.0f - x * x * x; }
inline float easeInOut(float x) {
  x = clamp01(x);
  return x < 0.5f ? 2 * x * x : 1 - 2 * (1 - x) * (1 - x);
}
inline int lerpi(int a, int b, float t) { return a + (int)((b - a) * t + (b > a ? 0.5f : -0.5f)); }

// 0 -> 1 fade-in, hold, 1 -> 0 fade-out (slide amount)
inline float envelope(uint32_t t, uint32_t dur, uint32_t fade = 450) {
  if (t < fade) return easeOut(t / (float)fade);
  if (t + fade > dur) return easeOut((dur > t ? dur - t : 0) / (float)fade);
  return 1.0f;
}

// Small blocky robot hand/fist: rounded box + finger lines
template<class Canvas> inline void fist(Canvas &g, int x, int y, int w, int h, bool wiggle,
                 uint16_t FG, uint16_t BG) {
  g.fillRoundRect(x, y, w, h, 4, BG);
  g.drawRoundRect(x, y, w, h, 4, FG);
  for (int i = 1; i <= 2; i++) {
    int ly = y + i * h / 3 + ((wiggle && i == 1) ? 1 : 0);
    g.drawFastHLine(x + 2, ly, w - 4, FG);
  }
}

// ------------------------------------------------------------ GAMER
// Eyes look forward, hands grip a controller, buttons flash, stick wobbles.
template<class Canvas> inline void drawGamer(Canvas &g, uint32_t t, uint32_t dur,
                      uint16_t FG = 1, uint16_t BG = 0) {
  float e = envelope(t, dur);
  if (e <= 0.01f) return;

  const int cx = 80;
  int cy = 98 + (int)((1.0f - e) * 50);          // slide up from bottom
  if ((t / 110) % 9 == 0) cy += 1;                // tiny "intense gaming" jitter

  // controller body: centre bar + two grips
  g.fillRoundRect(cx - 32, cy - 14, 64, 22, 9, FG);
  g.fillRoundRect(cx - 38, cy - 6, 22, 30, 10, FG);
  g.fillRoundRect(cx + 16, cy - 6, 22, 30, 10, FG);

  // left analog stick, wobbling
  float a = t * 0.006f;
  int sx = (int)(sinf(a) * 2), sy = (int)(cosf(a * 1.3f) * 2);
  g.fillCircle(cx - 14, cy - 4, 5, BG);
  g.fillCircle(cx - 14 + sx, cy - 4 + sy, 3, FG);

  // face buttons (diamond), one "pressed" at pseudo-random intervals
  const int bx[4] = {cx + 16, cx + 9, cx + 23, cx + 16};
  const int by[4] = {cy - 9, cy - 4, cy - 4, cy + 1};
  uint32_t k = t / 140;
  int pressed = (int)((k * 7 + (k >> 2) * 3) % 6);   // 0..3 = press, 4..5 = idle
  for (int i = 0; i < 4; i++) {
    if (i == pressed) {
      g.fillCircle(bx[i], by[i], 2, FG);
      g.drawCircle(bx[i], by[i], 3, BG);
    } else {
      g.fillCircle(bx[i], by[i], 2, BG);
    }
  }

  // centre logo dot
  g.fillCircle(cx, cy + 3, 2, BG);

  // hands over the grips (finger wiggle)
  bool wig = ((t / 120) & 1);
  fist(g, cx - 42, cy + 4, 22, 22, wig, FG, BG);
  fist(g, cx + 20, cy + 4, 22, 22, !wig, FG, BG);
}

// ------------------------------------------------------------ POPCORN
// Right hand holds the bucket, left hand picks a kernel, brings it to a tiny
// mouth that appears, chews 3 times, then vanishes.
template<class Canvas> inline void drawPopcorn(Canvas &g, uint32_t t, uint32_t dur,
                        uint16_t FG = 1, uint16_t BG = 0) {
  float e = envelope(t, dur);
  if (e <= 0.01f) return;

  const int cx = 80;
  const int oy = (int)((1.0f - e) * 50);
  const int topY = 90 + oy, botY = 124 + oy;

  // popcorn puffs (back row first, then front row)
  static const int8_t puffX[9] = {-14, -5, 4, 13, -19, -10, -1, 8, 17};
  static const int8_t puffY[9] = {-10, -12, -11, -10, -3, -5, -4, -5, -3};
  for (int i = 0; i < 9; i++) {
    g.fillCircle(cx + puffX[i], topY + puffY[i], 5, BG);
    g.drawCircle(cx + puffX[i], topY + puffY[i], 5, FG);
  }

  // bucket: trapezoid fill, outline, stripes, rim
  for (int y = topY; y <= botY; y++) {
    int half = 24 - (6 * (y - topY)) / (botY - topY);
    g.drawFastHLine(cx - half, y, 2 * half + 1, BG);
  }
  for (int i = -3; i <= 3; i++) g.drawLine(cx + i * 8, topY, cx + i * 6, botY, FG);
  g.drawFastHLine(cx - 25, topY, 51, FG);
  g.drawFastHLine(cx - 25, topY + 1, 51, FG);
  g.drawFastHLine(cx - 18, botY, 37, FG);

  // right hand holding the bucket
  fist(g, cx + 16, topY + 8, 14, 16, false, FG, BG);

  // ---- bite cycle (2.8 s): pick -> mouth -> chew x3 -> vanish -> rest
  const uint32_t period = 2800, lead = 450;
  uint32_t tc = (t > lead ? t - lead : 0) % period;

  float hp;                                   // 0 = at bucket, 1 = at mouth
  if (tc < 450)       hp = easeInOut(tc / 450.0f);
  else if (tc < 700)  hp = 1.0f;
  else if (tc < 1150) hp = 1.0f - easeInOut((tc - 700) / 450.0f);
  else                hp = 0.0f;
  bool kernel = (t < lead) || (tc < 500);

  int myc = 68 + oy;                          // mouth centre y
  int ax = lerpi(cx - 8, cx - 15, hp);
  int ay = lerpi(topY - 8, myc + 2, hp);

  fist(g, ax - 6, ay - 6, 12, 12, false, FG, BG);
  if (kernel) {
    g.fillCircle(ax + 5, ay - 6, 3, BG);
    g.drawCircle(ax + 5, ay - 6, 3, FG);
  }

  // tiny cute mouth: pops in, chews, shrinks away
  if (t >= lead && tc >= 500 && tc < 1900) {
    float s = clamp01((tc - 500) / 100.0f);
    float out = clamp01((1900 - tc) / 150.0f);
    if (out < s) s = out;
    float chew = fabsf(sinf((tc - 500) * 3.14159f / 380.0f));
    int w = (int)(10 * s);
    int h = (int)((2 + 3 * chew) * s);
    if (w >= 2 && h >= 1) {
      int r = h / 2;
      g.fillRoundRect(cx - w / 2, myc - h / 2, w, h, r, FG);
    }
  }
}

}  // namespace FeniAnim
