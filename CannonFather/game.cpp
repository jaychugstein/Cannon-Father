// CANNON FATHER - Arduboy
#pragma GCC optimize("no-tree-loop-optimize","no-inline-small-functions","no-split-wide-types")
#include <Arduino.h>
#include <Arduboy2.h>
#include <ArduboyFX.h>
#include "fxdata.h"
#include "tables.h"
#include "font.h"
#include <ArduboyTones.h>

Arduboy2Base ab;
ArduboyTones snd(ab.audio.enabled);
// "Joyful, Joyful, We Adore Thee" (Beethoven's Ode to Joy, hymn tune Hymn to Joy, public domain): menus and aiming
const uint16_t SONG[] PROGMEM = {330,402,0,40,330,402,0,40,349,402,0,40,392,402,0,40,392,402,0,40,349,402,0,40,330,402,0,40,294,402,0,40,262,402,0,40,262,402,0,40,294,402,0,40,330,402,0,40,330,623,0,40,294,181,0,40,294,844,0,40,330,402,0,40,330,402,0,40,349,402,0,40,392,402,0,40,392,402,0,40,349,402,0,40,330,402,0,40,294,402,0,40,262,402,0,40,262,402,0,40,294,402,0,40,330,402,0,40,294,623,0,40,262,181,0,40,262,844,0,40,0,884, TONES_END};
ARDUBOY_NO_USB   // saves space; uploading a new game needs the reset button / FX menu, like Pocket Rocket
// NES/SNES style: the outside button (Arduboy B) is GO, the inside one (Arduboy A) is the extra button
#define BTN_GO B_BUTTON
#define BTN_ALT A_BUTTON
inline void pset(int16_t x, int16_t y) { if ((uint16_t)x < 128 && (uint16_t)y < 64) ab.sBuffer[(y >> 3) * 128 + x] |= 1 << (y & 7); }
void spr(int16_t x, int16_t y, uint24_t a, uint8_t f) { FX::drawBitmap(x, y, a, f, dbmMasked); }
int16_t isin(int8_t d); int16_t icos(int8_t d);
int16_t sin360(int16_t d) { d %= 360; if (d < 0) d += 360; if (d < 90) return isin(d); if (d < 180) return isin(180 - d); if (d < 270) return -isin(d - 180); return -isin(360 - d); }

// ---------------- world ----------------
// x: world pixels to the right, h: pixels above the ground. 4 px = 1 metre.
#define GY 58          // screen row of the ground line when camera is at ground level
#define PX_PER_M 4
#define GRAV 0.077      // a little floaty so rounds last longer
#define CANNON_X 24    // world x of the cannon wheel
#define SEG 56         // ground object spacing
#define ASEG 40        // air object spacing

enum { M_TITLE, M_STORY, M_CONTROLS, M_AIM, M_POWER, M_FLY, M_SOUND };
uint8_t mode = M_TITLE;
uint16_t seed;
uint32_t best;

float mx, mh, vx, vh;     // man position/velocity
float camX, camH;
int8_t angleDeg = 45;     // cannon angle
float aimF = 45, aimV;     // smooth aiming: position and speed
float power;              // 0..1
int8_t pdir = 1;
bool perfect, flying, bounced, stopped;
uint8_t spin;             // man rotation frame 0..15 (x16 for smooth spin)
uint32_t points;
uint16_t tick, doneT, fireT;
uint8_t endReason;        // 0 ground, 2 hay, 3 cactus, 4 stuck in mud
uint8_t miniT; int16_t miniSeg = -1;   // inside a ground cannon
uint8_t wingsT;                        // angel wings power-up
// how Father moves through the air
enum { PO_SUPER, PO_TUMBLE, PO_BALL, PO_FLOAT, PO_PRAY };
#define GLIDE_MAX 150                 // frames of cassock glide per round (2.5 s)
uint8_t glide; bool gliding;
// flight report stats
float topV, maxH, spun; uint16_t airT, bounces, hits, biblesOut, souls;
enum { SET_A, SET_B, SET_C, SET_D, SET_SUPER, SET_BALL, SET_PRAY };
uint8_t pose, poseT, dizzyT, sootT;
float ang, angW;                       // rotation (0..16 = full turn) and spin speed
void setPose(uint8_t p, uint8_t t, float w) { pose = p; poseT = t; angW = w; }
int8_t shx, shy; uint8_t shakeT;          // screen shake
uint8_t flashT;                            // muzzle flash
float boomX; uint8_t boomT;                // bomb blast ring
int16_t sqSeg = -1, hitSeg = -1; uint8_t sqT;           // squashed trampoline/spring

// remembered used things (popped balloons, exploded bombs...)
uint16_t used[16]; uint8_t usedN;
bool isUsed(uint16_t k) { for (uint8_t i = 0; i < 16; i++) if (used[i] == k) return true; return false; }
void markUsed(uint16_t k) { used[usedN++ & 15] = k; }

// popups
struct Pop { int16_t x, h; uint8_t t; uint8_t kind; uint16_t v; };
Pop pops[4];
const char* const POP_TXT[] = {"+", "PERFECT!", "BOOM!", "RAMP!", "HAY!", "OUCH!", "SPLAT!", "WHOOSH!", "RELAUNCH!", "DING!", "HALLELUJAH!", "STUCK!", "SAVED!"};
enum { P_PTS, P_PERFECT, P_BOOM, P_RAMP, P_HAY, P_OUCH, P_SPLAT, P_WHOOSH, P_RELAUNCH, P_DING, P_HALLELUJAH, P_STUCK, P_SAVED };
void addPopAt(uint8_t kind, uint16_t v, int16_t x, int16_t y) {
  uint8_t j = 0; for (uint8_t i = 1; i < 4; i++) if (pops[i].t < pops[j].t) j = i;
  for (uint8_t i = 0; i < 4; i++)        // stack above any popup still showing nearby
    if (i != j && pops[i].t > 0 && abs(pops[i].h - (45 - pops[i].t) / 4 - y) < 11) y = pops[i].h - (45 - pops[i].t) / 4 - 11;
  pops[j] = {x, y, 45, kind, v};
}
void addPop(uint8_t kind, uint16_t v) {
  int16_t x = (int16_t)(mx - camX) + 12, y = (int16_t)(GY - (mh - camH)) - 18;
  if (y < 10) y = (int16_t)(GY - (mh - camH)) + 14;   // no room above him: show it below
  addPopAt(kind, v, x, y);
}

// explosion / dust particles
struct Pt { float x, h, vx, vh; uint8_t t, k; };   // k: 0 spark, 1 smoke puff
#define NPT 20
Pt pts[NPT];
void burst(float x, float h, uint8_t n, float sp, uint8_t k = 0) {
  for (uint8_t i = 0; i < NPT && n; i++) if (!pts[i].t) {
    int16_t a = random(0, 360); float s = sp * random(30, 100) / 25600.0;
    pts[i].x = x; pts[i].h = h; pts[i].vx = sin360(a + 90) * s; pts[i].vh = sin360(a) * s;
    pts[i].t = random(15, 35); pts[i].k = k; n--;
  }
}


// fast 5x7 text straight into the screen buffer (white)
void col8(int16_t x, int16_t y, uint8_t b) {
  if ((uint16_t)x > 127 || !b) return;
  if (y < 0) { if (y <= -8) return; b >>= -y; y = 0; }
  if (y > 63) return;
  uint8_t* d = &ab.sBuffer[(y >> 3) * 128 + x]; uint8_t sh = y & 7;
  d[0] |= b << sh; if (sh && y < 56) d[128] |= b >> (8 - sh);
}
void txc(int16_t x, int16_t y, char c) { uint8_t g = (uint8_t)c == 0xF8 ? 59 : ((uint8_t)c < 32 || (uint8_t)c > 90) ? 0 : c - 32; const uint8_t* f = &FONT[g * 5]; for (uint8_t i = 0; i < 5; i++) col8(x + i, y, pgm_read_byte(f + i)); }
void txt(int16_t x, int16_t y, const char* s) { for (; *s; s++, x += 6) txc(x, y, *s); }
void txt(int16_t x, int16_t y, const __FlashStringHelper* fs) { const char* s = (const char*)fs; char c; while ((c = pgm_read_byte(s++))) { txc(x, y, c); x += 6; } }

// tiny 3x5 font (chars 32..90), 3 column bytes per glyph
const uint8_t TINY[177] PROGMEM = {0,0,0,0,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,4,14,4,16,8,0,4,4,4,0,16,0,24,4,3,31,17,31,18,31,16,25,21,18,17,21,10,7,4,31,23,21,9,30,21,29,1,29,3,31,21,31,23,21,15,0,10,0,0,0,0,0,0,0,0,0,0,0,0,0,1,21,3,0,0,0,30,5,30,31,21,10,14,17,17,31,17,14,31,21,17,31,5,1,14,17,29,31,4,31,17,31,17,8,16,15,31,4,27,31,16,16,31,6,31,31,1,30,14,17,14,31,5,2,14,25,30,31,5,26,18,21,9,1,31,1,31,16,31,15,16,15,31,12,31,27,4,27,3,28,3,25,21,19};
int16_t tiny(int16_t x, int16_t y, const __FlashStringHelper* fs) {
  const char* s = (const char*)fs; char c;
  while ((c = pgm_read_byte(s++))) { for (uint8_t i = 0; i < 3; i++) col8(x + i, y, pgm_read_byte(&TINY[(c - 32) * 3 + i])); x += 4; }
  return x;
}
int16_t tinyN(int16_t x, int16_t y, int32_t n) {
  char b[12]; ltoa(n, b, 10);
  for (char* p = b; *p; p++) { for (uint8_t i = 0; i < 3; i++) col8(x + i, y, pgm_read_byte(&TINY[(*p - 32) * 3 + i])); x += 4; }
  return x;
}

uint16_t hsh(uint16_t i, uint8_t salt) {
  uint16_t h = i * 25173u + seed + salt * 13849u; h ^= h >> 7; h *= 0x2F5B; h ^= h >> 9; return h;
}

// ---------------- ground things ----------------
enum { G_NONE, G_TRAMP, G_SPRING, G_BOMB, G_RAMP, G_HAY, G_CACTUS, G_MINI, G_FAN, G_MUD };
const uint24_t GSPR[] = {0, FX_TRAMP, FX_SPRING, FX_BOMB, FX_RAMP, FX_HAY, FX_CACTUS, FX_MINI, FX_FAN, FX_MUD};
const uint8_t GW[] = {0, TRAMP_W, SPRING_W, BOMB_W, RAMP_W, HAY_W, CACTUS_W, MINI_W, FAN_W, MUD_W};
const uint8_t GH[] = {0, TRAMP_H, SPRING_H, BOMB_H, RAMP_H, HAY_H, CACTUS_H, MINI_H, FAN_H, MUD_H};
uint8_t gType(uint16_t s) {
  if (s < 3) return G_NONE;
  uint8_t r = hsh(s, 1) % 100;
  if (r < 13) return G_TRAMP;
  if (r < 21) return G_SPRING;
  if (r < 29) return G_BOMB;
  if (r < 38) return G_RAMP;
  if (r < 43) return G_MINI;
  if (r < 50) return G_FAN;
  if (s < 11) return G_NONE;          // no stoppers in the first ~150 m
  if (r < 56) return G_MUD;
  if (r < 62) return G_HAY;
  if (r < 68) return G_CACTUS;
  return G_NONE;
}
int32_t gX(uint16_t s) { return (int32_t)s * SEG + (hsh(s, 2) & 31); }

// ---------------- air things ----------------
enum { A_NONE, A_STAR, A_BALLOON, A_DOVE, A_BELL, A_WINGS };
uint8_t aType(uint16_t s) {
  if (s < 4) return A_NONE;
  uint8_t r = hsh(s, 3) % 100;
  if (r < 32) return A_NONE;
  if (r < 60) return A_STAR;
  if (r < 76) return A_BALLOON;
  if (r < 88) return A_DOVE;
  if (r < 95) return A_BELL;
  return A_WINGS;
}
int32_t aX(uint16_t s) { return (int32_t)s * ASEG + (hsh(s, 4) & 15) + (aType(s) == A_DOVE ? -(int16_t)((tick >> 1) % 60) : 0); }
int16_t aH(uint16_t s) { uint16_t top = 30 + s * 3; if (top > 220) top = 220; return 18 + hsh(s, 5) % top; }

// ---------------- helpers ----------------
int16_t sx(float x) { return (int16_t)(x - camX) + shx; }
int16_t sy(float h) { return GY - (int16_t)(h - camH) + shy; }

void boxText(int16_t x, int8_t y, const char* s) {
  int8_t w = strlen(s) * 6 + 3; ab.fillRect(x - 2, y - 2, w, 11, BLACK); ab.drawRect(x - 2, y - 2, w, 11, WHITE);
  txt(x, y, s);
}

// cannon: pivot (wheel hub) at CANNON_X, PIVOT_H above the ground; barrel 32 px long
#define PIVOT_H 11
float muzX() { return CANNON_X + icos(angleDeg) * 32 / 256.0; }
float muzH() { return PIVOT_H + isin(angleDeg) * 32 / 256.0; }

int16_t isin(int8_t d) { return pgm_read_word(&SIN90[d]); }
int16_t icos(int8_t d) { return pgm_read_word(&SIN90[90 - d]); }
uint8_t wheelF;
void drawCannon(int8_t recoil) {
  int16_t px = sx(CANNON_X) - recoil, py = sy(PIVOT_H);
  spr(px - 18, sy(0) - 5, FX_BASE, 0);
  spr(px - 12, py - 36, FX_CANNON, angleDeg - 10);
  spr(px - 11, py - 11, FX_WHEEL, wheelF & 3);
}

void drawBackground() {
  // stars (far, slow)
  int16_t ox = (int16_t)(camX / 8), oh = (int16_t)(camH / 8);
  for (int16_t c = (ox >> 4) - 1; c <= (ox >> 4) + 9; c++)
    for (int8_t r = 0; r < 6; r++) {
      uint16_t h = hsh(c * 7 + r + 9000, 7);
      if (h & 3) continue;
      int16_t x = c * 16 + (h >> 4 & 15) - ox, y = r * 10 + (h >> 8 & 7) + oh - 6;
      if (y < 50) pset(x, y);
    }
  // far hills (parallax 1/4)
  {
    int16_t base = GY + (int16_t)(camH * 0.25);
    if (base - 20 < 64) {
      int16_t o = (int16_t)(camX / 4);
      for (int16_t x = 0; x < 128; x++) {
        int16_t y = base - (int8_t)pgm_read_byte(&HILL[((x + o) >> 1) & 255]);
        if (y >= 0 && y < 64) pset(x, y);
      }
    }
  }
  // clouds (parallax 1/2)
  int32_t cxo = (int32_t)(camX / 2);
  for (int16_t c = cxo / 70 - 1; c <= cxo / 70 + 2; c++) {
    uint16_t h = hsh(c + 5000, 8);
    if (h % 3 == 0) continue;
    int16_t x = c * 70 + (h >> 3 & 31) - cxo;
    int16_t y = GY - 30 - (h >> 6 & 127) + (int16_t)(camH / 2);
    if (y > -14 && y < 64) spr(x, y, FX_CLOUD, 0);
  }
}

void drawGround() {
  int16_t gy = sy(0);
  if (gy > 70) return;
  ab.drawFastHLine(0, gy, 128, WHITE);
  for (int16_t x = 0; x < 128; x++) {         // grass + pebbles
    int32_t wx = x + (int32_t)camX;
    uint16_t h = hsh(wx, 11);
    if ((h & 7) == 0) pset(x, gy - 1);
    if ((h & 31) == 3) pset(x, gy + 2);
    if ((h & 63) == 9) pset(x, gy + 4);
  }
  // distance signs every 25 m
  int32_t first = ((int32_t)camX / 100) * 100;
  for (int32_t wx = first; wx < camX + 140; wx += 100) {
    if (wx < 100) continue;
    int16_t x = sx(wx) - 10;
    spr(x, gy - 13, FX_SIGN, 0);
    char b[6]; itoa(wx / PX_PER_M, b, 10);
    txt(x + 10 - strlen(b) * 3, gy - 11, b);
  }
  // circus tent at the start
  spr(sx(-60), gy - BIGTENT_H + 2, FX_BIGTENT, 0);
}

// ---------------- the good word: bibles knocked loose, crowds to save ----------------
#define NBIB 10
struct Bible { float x, h, vx, vh; uint8_t st; };   // st: 0 none, 1 flying, 2 landed
Bible bib[NBIB];
#define CSEG 100
bool crowdAt(int16_t s) { return s >= 1 && hsh(s, 9) % 100 < 45; }
int32_t crowdX(int16_t s) { return (int32_t)s * CSEG + 37 + (hsh(s, 10) & 15); }
void dropBibles(uint8_t n) {
  while (n--) {
    uint8_t j = 0;
    for (uint8_t i = 0; i < NBIB; i++) { if (!bib[i].st) { j = i; break; } if (bib[i].st == 2 && bib[i].x < bib[j].x) j = i; }
    Bible& b = bib[j];
    b.x = mx; b.h = mh + 6; b.vx = vx * 0.5 + random(-60, 61) / 50.0; b.vh = 1.5 + random(0, 100) / 40.0; b.st = 1;
    biblesOut++;
  }
}
void stepBibles() {
  for (uint8_t i = 0; i < NBIB; i++) {
    Bible& b = bib[i];
    if (b.st != 1) continue;
    b.vh -= 0.09; b.x += b.vx; b.h += b.vh;
    if (b.h <= 0) {
      b.h = 0; b.st = 2;
      int16_t s = (int32_t)b.x / CSEG;
      for (int16_t k = s - 1; k <= s; k++) {       // did it land among people?
        if (!crowdAt(k) || isUsed(k | 0x4000)) continue;
        int32_t cx = crowdX(k);
        if (b.x < cx - 4 || b.x > cx + CROWD_W + 2) continue;
        markUsed(k | 0x4000); souls += 4; points += 50;
        addPopAt(P_SAVED, 0, (int16_t)(b.x - camX), GY - (int16_t)(0 - camH) - 22);
        snd.tone(NOTE_C6, 40, NOTE_E6, 40, NOTE_G6, 80);   // a soul saved
        break;
      }
    }
  }
}
void drawBibles() {
  for (uint8_t i = 0; i < NBIB; i++) if (bib[i].st)
    spr(sx(bib[i].x) - 4, sy(bib[i].h) - (bib[i].st == 2 ? 6 : 5), FX_BIBLE, bib[i].st == 2 ? 1 : (tick >> 2) & 1);
}
void drawCrowds() {
  int16_t gy = sy(0);
  for (int16_t s = (int32_t)(camX - 40) / CSEG; s <= (int32_t)(camX + 140) / CSEG; s++) {
    if (!crowdAt(s)) continue;
    bool saved = isUsed(s | 0x4000);
    spr(sx(crowdX(s)), gy - CROWD_H + 2 - (saved && (tick & 8) ? 1 : 0), FX_CROWD, saved ? 1 : 0);
  }
}

void drawGroundThings() {
  int16_t gy = sy(0);
  for (int16_t s = (int32_t)(camX - 20) / SEG; s <= (int32_t)(camX + 140) / SEG; s++) {
    if (s < 0) continue;
    uint8_t t = gType(s); if (!t) continue;
    if (t == G_BOMB && isUsed(s)) continue;
    if (t == G_FAN) {   // rising air above the fan
      int16_t fx = sx(gX(s));
      for (uint8_t k = 0; k < 4; k++) {
        int16_t lx = fx + 3 + k * 5, ly = gy - 12 - (int16_t)((tick * 2 + k * 13) % 40);
        ab.drawFastVLine(lx, ly, 4, WHITE);
      }
    }
    int8_t sq = (s == sqSeg && sqT) ? (sqT > 4 ? 3 : sqT >> 1) : 0;   // squash when landed on
    if (t == G_HAY && stopped && endReason == 2 && s == hitSeg) { spr(sx(gX(s)), gy - HAYLEGS_H + 2, FX_HAYLEGS, 0); continue; }
    int8_t shake = (t == G_MINI && s == miniSeg && miniT) ? (int8_t)((tick & 2) ? 1 : -1) : 0;
    uint8_t fr = (t == G_BOMB) ? (tick >> 3) & 1 : (t == G_FAN) ? (tick >> 1) & 1 : 0;
    spr(sx(gX(s)) + shake, gy - GH[t] + 2 + sq, GSPR[t], fr);
  }
}

void drawAirThings() {
  for (int16_t s = (int32_t)(camX - 20) / ASEG; s <= (int32_t)(camX + 160) / ASEG; s++) {
    if (s < 0) continue;
    uint8_t t = aType(s); if (!t || (t != A_BELL && isUsed(s | 0x8000))) continue;
    int16_t x = sx(aX(s)), y = sy(aH(s));
    if (y < -20 || y > 74) continue;
    if (t == A_STAR) spr(x - 7, y - 7, FX_STAR, (tick >> 4) & 1);
    else if (t == A_BALLOON) spr(x - 6, y - 7, FX_BALLOON, 0);
    else if (t == A_DOVE) spr(x - 11, y - 6, FX_DOVE, (tick >> 3) & 3);
    else if (t == A_BELL) spr(x - 7, y - 8, FX_BELL, isUsed(s | 0x8000) ? (tick >> 2) & 1 : (tick >> 5) & 1);
    else spr(x - 9, y - 6, FX_WINGS, 0);
  }
}

void drawMan() {
  int16_t x = sx(mx), y = sy(mh);
  if (stopped) {
    if (endReason == 2) return;                       // legs sticking out of the hay
    // flat on his back, head toward where he was going, with dizzy stars
    spr(x - 16, y - 16, FX_MAN, 4);
    int8_t ox, oy;
    uint8_t ph = (tick >> 2) & 15;
    static const int8_t RX[16] = {0, 2, 4, 5, 6, 5, 4, 2, 0, -2, -4, -5, -6, -5, -4, -2};
    static const int8_t RY[16] = {-2, -2, -1, -1, 0, 1, 1, 2, 2, 2, 1, 1, 0, -1, -1, -2};
    for (uint8_t i = 0; i < 2; i++) {
      uint8_t q = (ph + i * 8) & 15;
      ox = RX[q]; oy = RY[q];
      int16_t px = x + 10 + ox, py = y - 10 + oy;
      pset(px, py); pset(px - 1, py); pset(px + 1, py);
      pset(px, py - 1); pset(px, py + 1);
    }
    return;
  }
  if (miniT) return;                                  // inside the ground cannon
  uint8_t set;
  if (gliding) set = SET_A;                                  // arms out, robe spread
  else if (pose == PO_SUPER) set = SET_SUPER;
  else if (pose == PO_BALL) set = SET_BALL;
  else if (pose == PO_PRAY) set = SET_PRAY;
  else if (pose == PO_FLOAT) set = (tick & 16) ? SET_B : SET_A;   // arms going up and down
  else {                                                     // ragdoll: limbs flop around
    uint8_t rate = (angW > 0.3 || angW < -0.3) ? 2 : 3;
    static const uint8_t FL[4] = {SET_A, SET_C, SET_B, SET_D};
    set = FL[(tick >> rate) & 3];
  }
  spr(x - 16, y - 16, FX_MAN, set * 16 + (((uint8_t)(ang + 0.5)) & 15));
  if (dizzyT) {                                              // dizzy stars after the bell
    static const int8_t DX[8] = {0, 4, 6, 4, 0, -4, -6, -4}, DY[8] = {-3, -2, 0, 2, 3, 2, 0, -2};
    for (uint8_t i = 0; i < 2; i++) { uint8_t q = ((tick >> 2) + i * 4) & 7; int16_t px = x + DX[q], py = y - 14 + DY[q];
      pset(px, py); pset(px - 1, py); pset(px + 1, py); pset(px, py - 1); pset(px, py + 1); }
  }
  if (wingsT && (wingsT > 40 || (tick & 4))) {        // halo while the angel wings last
    ab.drawFastHLine(x - 4, y - 19, 9, WHITE); ab.drawFastHLine(x - 4, y - 17, 9, WHITE);
    pset(x - 5, y - 18); pset(x + 5, y - 18);
  }
}

void drawEffects() {
  if (flashT) {    // muzzle flash
    int16_t x = sx(muzX()), y = sy(muzH());
    ab.fillCircle(x, y, flashT, WHITE);
    if (flashT > 3) ab.fillCircle(x, y, flashT - 3, BLACK);
  }
  if (boomT) {     // bomb blast
    int16_t x = sx(boomX), y = sy(0);
    uint8_t r = (24 - boomT) * 2;
    if (boomT > 20) ab.fillCircle(x, y, r + 4, WHITE);
    else { ab.drawCircle(x, y, r, WHITE); if (r > 4) ab.drawCircle(x, y, r - 3, WHITE); }
  }
}

void drawHud() {
  // keep it clean: just the points (top right) and the glide meter (top left)
  char b[8]; ltoa(points, b, 10);
  int16_t w = strlen(b) * 4;
  ab.fillRect(126 - w, 0, w + 2, 7, BLACK); tinyN(127 - w, 1, points);
  ab.fillRect(0, 0, 50, 7, BLACK);
  tiny(1, 1, F("GLIDE"));
  ab.drawRect(22, 1, 26, 5, WHITE);
  ab.fillRect(23, 2, (uint16_t)glide * 24 / GLIDE_MAX, 3, WHITE);
}

void drawPops() {
  for (uint8_t i = 0; i < 4; i++) if (pops[i].t) {
    Pop& p = pops[i];
    char b[12];
    if (p.kind == 0) { b[0] = '+'; itoa(p.v, b + 1, 10); } else strcpy(b, POP_TXT[p.kind]);
    int16_t x = p.x - strlen(b) * 3, y = p.h - (45 - p.t) / 4;
    if (x < 2) x = 2; if (x > 124 - (int16_t)strlen(b) * 6) x = 124 - strlen(b) * 6;
    if (y < 10) y = 10; if (y > 52) y = 52;
    boxText(x, y, b);
  }
}

void drawParticles() {
  for (uint8_t i = 0; i < NPT; i++) if (pts[i].t) {
    int16_t x = sx(pts[i].x), y = sy(pts[i].h);
    if (pts[i].k == 1) {   // smoke puff grows and thins out
      uint8_t r = (40 - pts[i].t) >> 3;
      ab.fillCircle(x, y, r + 1, BLACK); ab.drawCircle(x, y, r + 1, WHITE);
    } else if (pts[i].t > 20) ab.fillCircle(x, y, 1, WHITE); else pset(x, y);
  }
}

// ---------------- game ----------------
void newGame() {
  seed = random(0, 65535); usedN = 0; for (uint8_t i = 0; i < 16; i++) used[i] = 0xFFFF;
  for (uint8_t i = 0; i < 4; i++) pops[i].t = 0;
  for (uint8_t i = 0; i < NPT; i++) pts[i].t = 0;
  glide = GLIDE_MAX; gliding = false; topV = maxH = spun = 0; airT = bounces = hits = biblesOut = souls = 0; for (uint8_t i = 0; i < NBIB; i++) bib[i].st = 0;
  shakeT = flashT = boomT = sqT = miniT = wingsT = dizzyT = sootT = 0; pose = PO_SUPER; ang = angW = 0; sqSeg = miniSeg = -1;
  camX = -40; camH = 0; points = 0; hitSeg = -1; perfect = false; flying = bounced = stopped = false;
  power = 0; pdir = 1; mode = M_AIM; endReason = 0; spin = 0;
}

void fire() {
  float v = 2.6 + power * 5.4;
  perfect = power > 0.965;
  if (perfect) { v *= 1.18; addPop(P_PERFECT, 0); }
  if (perfect) snd.tone(NOTE_C3, 60, NOTE_G2, 60, NOTE_C6, 60); else snd.tone(NOTE_C3, 70, NOTE_G2, 80, NOTE_C2, 90);   // BOOM (with a ding on top for PERFECT)
  mx = muzX(); mh = muzH();
  vx = icos(angleDeg) * v / 256; vh = isin(angleDeg) * v / 256;
  flying = true; fireT = 0; mode = M_FLY; setPose(PO_SUPER, 0, 0);
  flashT = 9; shakeT = 8;
  burst(mx, mh, 8, 1.5);
  burst(mx, mh, 5, 0.5, 1);
}

void stopMan(uint8_t why) {
  vx = vh = 0; stopped = true; endReason = why; doneT = 0;
  static const uint8_t EP[] = {0, 0, P_HAY, P_OUCH, P_STUCK};
  if (why) addPop(EP[why], 0);
  if (why == 2) snd.tone(NOTE_E3, 40, NOTE_C3, 60);            // soft thud into the hay
  else if (why == 3) snd.tone(NOTE_A5, 50, NOTE_D5, 90);       // OUCH! a cactus
  else if (why == 4) snd.tone(NOTE_D3, 60, NOTE_A2, 120);      // stuck in the mud
}

// ramp surface height at world x (30 degree slope rising to the right)
int16_t rampTop(int32_t x0, float x) { float d = x - x0; if (d < 0) d = 0; if (d > RAMP_W - 2) d = RAMP_W - 2; return (int16_t)(d * 0.577); }

// things that act every frame, not only on landing: ramps, fans, mud
void groundTouch() {
  int16_t s = (int32_t)mx / SEG;
  for (int16_t k = s - 1; k <= s; k++) {
    if (k < 0) continue;
    uint8_t t = gType(k);
    int32_t x0 = gX(k);
    if (t == G_RAMP) {
      if (mx < x0 || mx > x0 + RAMP_W - 2) continue;
      int16_t top = rampTop(x0, mx);
      if (mh > top + 1 || vx <= 0) continue;
      float sp = sqrt(vx * vx + vh * vh); if (sp < 3.5) sp = 3.5;
      sp += 0.6;
      vx = sp * 0.866; vh = sp * 0.5; mh = top + 1;   // fly off at 30 degrees
      setPose(PO_SUPER, 0, 0);
      if (!isUsed(k)) { markUsed(k); points += 60; addPop(P_RAMP, 0); hits++; dropBibles(1); snd.tone(NOTE_C4, 30, NOTE_E4, 30, NOTE_G4, 40); }
      bounced = true;
    } else if (t == G_FAN) {
      if (mx < x0 - 2 || mx > x0 + FAN_W + 2 || mh > 90) continue;
      vh += 0.32; if (vh > 4.2) vh = 4.2;
      if (pose != PO_PRAY) setPose(PO_FLOAT, 20, 0);
      if (!isUsed(k)) { markUsed(k); points += 40; addPop(P_WHOOSH, 0); hits++; snd.tone(NOTE_A3, 40, NOTE_C4, 40, NOTE_E4, 60); }
    } else if (t == G_MUD) {
      if (mx < x0 || mx > x0 + MUD_W - 2 || mh > 2) continue;
      if (!isUsed(k)) { markUsed(k); addPop(P_SPLAT, 0); burst(mx, 1, 6, 1.0); snd.tone(NOTE_E2, 50, NOTE_C2, 70); }
      vx *= 0.86; if (vh < 0) vh = 0;
      if (vx < 0.25) stopMan(4);
    }
  }
}

void groundHit() {
  // which ground thing is under him?
  int16_t s = (int32_t)mx / SEG;
  for (int16_t k = s - 1; k <= s; k++) {
    if (k < 0) continue;
    uint8_t t = gType(k); if (!t || t == G_RAMP || t == G_FAN || t == G_MUD) continue;
    if ((t == G_BOMB || t == G_MINI) && isUsed(k)) continue;
    int32_t x0 = gX(k); int16_t w = GW[t] - 2;
    if (mx < x0 - 2 || mx > x0 + w + 2) continue;
    if (mh > GH[t] - 4) continue;
    hitSeg = k; hits++;
    switch (t) {
      case G_TRAMP: dropBibles(2); vh = max(-vh * 0.9, 3.6); points += 50; addPop(P_PTS, 50); sqSeg = k; sqT = 10; setPose(PO_FLOAT, 45, 0); snd.tone(NOTE_C4, 30, NOTE_G4, 30, NOTE_C5, 50); break;   // boing
      case G_SPRING: dropBibles(2); vh = 5.2; vx += 0.8; points += 75; addPop(P_PTS, 75); sqSeg = k; sqT = 10; setPose(PO_TUMBLE, 0, 0.75); snd.tone(NOTE_G4, 25, NOTE_D5, 25, NOTE_G5, 40); break;   // sproing
      case G_BOMB: dropBibles(5); vh = 6.0; vx += 2.5; points += 100; addPop(P_BOOM, 0); markUsed(k); burst(mx, 4, 14, 2.5); snd.tone(NOTE_D2, 90, NOTE_A1, 120, NOTE_D1, 160);
        burst(mx, 4, 4, 0.6, 1); boomX = x0 + 9; boomT = 24; shakeT = 14; setPose(PO_BALL, 55, 1.4); sootT = 70;
        ab.digitalWriteRGB(RED_LED, RGB_ON); break;
      case G_MINI:   // dropped into a ground cannon: it fires him again
        markUsed(k); miniSeg = k; miniT = 36; vx = vh = 0; mx = x0 + 9; mh = 8; snd.tone(NOTE_C3, 30, 0, 40, NOTE_C3, 30); return;   // clunk-clunk, loaded
      case G_HAY: stopMan(2); return;
      case G_CACTUS: stopMan(3); mx = x0 - 6; mh = 0; return;
    }
    mh = GH[t] - 4; bounced = true;
    return;
  }
  if (mh <= 0) {   // plain ground
    mh = 0;
    if (vh < 0) {
      if (vh < -0.7) burst(mx, 0, 3, 0.8);
      if (vh < -1.2) dropBibles(1);
      if (vh < -0.7) snd.tone(NOTE_G2 - (int16_t)(vh * 8), 25);   // thump
      vh = -vh * 0.52; vx *= 0.84; bounced = true; bounces++;
      if (pose != PO_TUMBLE || angW * angW < 0.01) setPose(PO_TUMBLE, 0, vx * 0.12 + (random(0, 2) ? 0.15 : -0.15));
      if (vh < 0.6) vh = 0;
    }
    if (vh == 0) { vx *= 0.955; if (vx < 0.05) stopMan(0); }
  }
}

void airHits() {
  int16_t s = (int32_t)mx / ASEG;
  for (int16_t k = s - 1; k <= s + 2; k++) {
    if (k < 0) continue;
    uint8_t t = aType(k); if (!t || isUsed(k | 0x8000)) continue;
    int32_t dx = aX(k) - (int32_t)mx; int16_t dh = aH(k) - (int16_t)mh;
    if (dx < -8 || dx > 8 || dh < -8 || dh > 8) continue;
    markUsed(k | 0x8000); hits++;
    if (t == A_STAR) { points += 10; addPop(P_PTS, 10); snd.tone(NOTE_E6, 25, NOTE_B6, 40); }
    else if (t == A_BALLOON) { points += 25; addPop(P_PTS, 25); snd.tone(NOTE_C6, 10, NOTE_C3, 30); vh += 2.0; burst(mx, mh, 6, 1); if (pose != PO_PRAY) setPose(PO_FLOAT, 30, 0); }
    else if (t == A_DOVE) { dropBibles(1); points += 50; addPop(P_PTS, 50); snd.tone(NOTE_A4, 60, NOTE_F4, 80); vx += 0.6; vh += 1; burst(mx, mh, 5, 1); if (pose != PO_PRAY) setPose(PO_TUMBLE, 0, -0.5); }
    else if (t == A_BELL) { dropBibles(2); points += 100; addPop(P_DING, 0); snd.tone(NOTE_A5, 160, NOTE_E5, 220); if (vh < 3.2) vh = 3.2; shakeT = 6; dizzyT = 90; if (pose != PO_PRAY) setPose(PO_TUMBLE, 0, 0.35); }
    else { points += 30; addPop(P_HALLELUJAH, 0); snd.tone(NOTE_C5, 70, NOTE_E5, 70, NOTE_G5, 140); wingsT = 200; setPose(PO_PRAY, 200, 0); }
  }
}

void stepFly() {
  if (miniT) {          // waiting inside the ground cannon, then BOOM
    if (--miniT == 0) {
      mx = gX(miniSeg) + 20; mh = 18; vx = 5.6; vh = 5.6; setPose(PO_BALL, 25, 1.2); dropBibles(3);
      points += 150; addPop(P_RELAUNCH, 0); flashT = 0; shakeT = 10; snd.tone(NOTE_G2, 60, NOTE_D2, 60, NOTE_G4, 120);   // boom with a whistle
      burst(mx, mh, 10, 2); burst(mx, mh, 4, 0.5, 1);
    }
  } else if (!stopped) {
    gliding = ab.pressed(BTN_ALT) && glide && !wingsT && mh > 2;
    if (gliding) {      // cassock spread like a parachute: fall slowly, drift on
      glide--; vh -= 0.03;
      if (vh < -0.7) vh = vh * 0.85 - 0.7 * 0.15;
    } else vh -= wingsT ? 0.03 : GRAV;
    vx *= 0.9995;
    if (wingsT) { wingsT--; if (vh < -1.2) vh = -1.2; }
    mx += vx; mh += vh;
    { float v2 = vx * vx + vh * vh; if (v2 > topV) topV = v2; }
    if (mh > maxH) maxH = mh;
    if (mh > 1) airT++;
    airHits();
    groundTouch();
    if (!stopped && !miniT) {
      if (mh <= 24 && vh < 0) groundHit();
      else if (mh <= 0) groundHit();
    }
    // pose: how he turns and what his arms and legs do
    if (poseT && --poseT == 0) { if (pose == PO_SUPER || pose == PO_FLOAT || pose == PO_PRAY) angW = 0.2; pose = PO_TUMBLE; }
    if (pose == PO_PRAY && !wingsT) pose = PO_TUMBLE;
    if (pose == PO_SUPER && !gliding) {     // point his head where he is flying (nearest of 16 directions)
      float bestD = -1e9; uint8_t bi = 0;
      for (uint8_t k = 0; k < 16; k++) { float dd = vx * sin360(k * 22 + (k >> 1)) + vh * sin360(k * 22 + (k >> 1) + 90); if (dd > bestD) { bestD = dd; bi = k; } }
      ang = bi;
    }
    else if (pose == PO_BALL) spun += angW < 0 ? -angW : angW;
    else if (gliding || pose == PO_FLOAT || pose == PO_PRAY) {   // settle upright with a gentle wobble
      float target = sin360(tick * 9) * (0.6 / 256);
      float d = target - ang; while (d > 8) d -= 16; while (d < -8) d += 16;
      ang += d * 0.2;
    } else { ang += angW; angW *= 0.996; spun += angW < 0 ? -angW : angW; }
    if (ang >= 16) ang -= 16; if (ang < 0) ang += 16;
    if (sootT) { sootT--; if (!(sootT & 3)) burst(mx - vx * 2, mh, 1, 0.2, 1); }
    if (dizzyT) dizzyT--;
  } else if (doneT < 255) doneT++;
  for (uint8_t i = 0; i < NPT; i++) if (pts[i].t) {
    pts[i].t--; pts[i].x += pts[i].vx; pts[i].h += pts[i].vh;
    if (pts[i].k == 1) { pts[i].vx *= 0.9; pts[i].vh = pts[i].vh * 0.9 + 0.02; } else pts[i].vh -= 0.05;
  }
  if (flashT) flashT--; if (boomT) boomT--; if (sqT) sqT--;
  if (boomT == 18) ab.digitalWriteRGB(RED_LED, RGB_OFF);
  for (uint8_t i = 0; i < 4; i++) if (pops[i].t) pops[i].t--;
  stepBibles();
  // camera follows
  float tx = mx - 44, th = mh - 30; if (th < 0) th = 0;
  camX += (tx - camX) * 0.2; camH += (th - camH) * 0.2;
  if (camH > mh - 8) camH = mh - 8; if (camH < mh - 32) camH = mh - 32; if (camH < 0) camH = 0;
  if (camX < mx - 100) camX = mx - 100; if (camX > mx - 20) camX = mx - 20;
  fireT++;
  if (fireT == 6) ab.digitalWriteRGB(RED_LED, RGB_OFF);
  if (stopped && doneT == 70) {
    snd.tone(NOTE_G4, 90, NOTE_C5, 90, NOTE_E5, 180);   // the Flight Report
    uint32_t total = (uint32_t)((mx - CANNON_X) / PX_PER_M) + points;
    if (total > best) { best = total; EEPROM.put(64, best ^ 0xC0FFEE00UL); }
  }
}

void drawPowerBar() {
  ab.drawRect(118, 6, 8, 52, WHITE);
  ab.fillRect(119, 7, 6, 50, BLACK);
  int8_t h = power * 48;
  ab.fillRect(120, 56 - h, 4, h, WHITE);
  ab.drawFastHLine(116, 9, 12, WHITE);   // perfect zone mark
  ab.fillRect(102, 4, 14, 7, BLACK); tiny(103, 5, F("MAX"));
}

// priestly ranks by total score
const char R0[] PROGMEM = "ALTAR BOY"; const char R1[] PROGMEM = "DEACON"; const char R2[] PROGMEM = "PRIEST";
const char R3[] PROGMEM = "BISHOP"; const char R4[] PROGMEM = "ARCHBISHOP"; const char R5[] PROGMEM = "CARDINAL";
const char R6[] PROGMEM = "POPE"; const char R7[] PROGMEM = "SAINT";
const char* const RANKS[] PROGMEM = {R0, R1, R2, R3, R4, R5, R6, R7};
const uint16_t RANK_AT[] PROGMEM = {0, 300, 700, 1300, 2200, 3300, 5000, 7500};

int16_t row(int16_t y, const __FlashStringHelper* label, int32_t v, const __FlashStringHelper* unit) {
  tiny(2, y, label);
  char b[12]; ltoa(v, b, 10);
  int16_t x = 62 - (strlen(b) + strlen_P((const char*)unit)) * 4;
  x = tinyN(x, y, v); tiny(x, y, unit);
  return y + 6;
}

void drawResult() {      // the Flight Report
  uint32_t dist = (uint32_t)((mx - CANNON_X) / PX_PER_M), total = dist + points;
  ab.clear();
  txt(25, 0, F("FLIGHT REPORT"));
  ab.drawFastHLine(0, 8, 128, WHITE); ab.drawFastVLine(65, 10, 44, WHITE);
  // left: how the flight went
  int16_t y = 11;
  y = row(y, F("DISTANCE"), dist, F("M"));
  y = row(y, F("TOP SPD"), (int16_t)(sqrt(topV) * (60.0 / PX_PER_M * 3.6)), F("KMH"));
  y = row(y, F("MAX HEIGHT"), (int16_t)(maxH / PX_PER_M), F("M"));
  { tiny(2, y, F("AIR TIME")); int16_t t10 = airT / 6; char b[7]; itoa(t10 / 10, b, 10);
    int16_t x = 62 - (strlen(b) + 3) * 4; x = tinyN(x, y, t10 / 10); x = tiny(x, y, F(".")); x = tinyN(x, y, t10 % 10); tiny(x, y, F("S")); y += 6; }
  y = row(y, F("FLIPS"), (int16_t)(spun / 16), F(""));
  y = row(y, F("BOUNCES"), bounces, F(""));
  y = row(y, F("BIBLES"), biblesOut, F(""));
  // right: points and rank
  tiny(69, 11, F("DISTANCE")); { char b[8]; ltoa(dist, b, 10); tinyN(126 - strlen(b) * 4, 11, dist); }
  tiny(69, 17, F("TRICKS")); { char b[8]; ltoa(points, b, 10); tinyN(126 - strlen(b) * 4, 17, points); }
  tiny(69, 23, F("THINGS HIT")); { char b[7]; itoa(hits, b, 10); tinyN(126 - strlen(b) * 4, 23, hits); }
  tiny(69, 29, F("SOULS SAVED")); { char b[7]; itoa(souls, b, 10); tinyN(126 - strlen(b) * 4, 29, souls); }
  ab.drawFastHLine(68, 35, 59, WHITE);
  tiny(69, 37, F("TOTAL")); { char b[8]; ltoa(total, b, 10); tinyN(126 - strlen(b) * 4, 37, total); }
  uint8_t r = 0; for (uint8_t i = 1; i < 8; i++) if (total >= pgm_read_word(&RANK_AT[i])) r = i;
  tiny(69, 43, F("RANK"));
  { const char* rn = (const char*)pgm_read_word(&RANKS[r]); int16_t w = strlen_P(rn) * 6; int16_t x = 97 - w / 2; if (x < 67) x = 67;
    txt(x, 49, (const __FlashStringHelper*)rn); }
  ab.drawFastHLine(0, 56, 128, WHITE);
  if (total >= best && total > 0 && (tick & 64)) { if (tick & 16) txt(37, 57, F("NEW BEST!")); }
  else if (tick & 32) tiny(32, 58, F("PRESS ANY BUTTON"));
}

void txt2(int16_t x, int16_t y, const __FlashStringHelper* fs) {
  const char* s = (const char*)fs; char c;
  while ((c = pgm_read_byte(s++))) {
    const uint8_t* f = &FONT[(c - 32) * 5];
    for (uint8_t i = 0; i < 5; i++) {
      uint8_t b = pgm_read_byte(f + i); uint16_t w = 0;
      for (uint8_t k = 0; k < 8; k++) if (b & (1 << k)) w |= 3 << (k * 2);
      for (uint8_t q = 0; q < 2; q++) { col8(x + i * 2 + q, y, w & 255); col8(x + i * 2 + q, y + 8, w >> 8); }
    }
    x += 12;
  }
}

// the Pope's orders, typed out one letter at a time
const char STORY[] PROGMEM =
  "FATHER,\n"
  "THE FLOCK HAS WANDERED FAR\n"
  "AND THE BUS ISN'T COMING.\n"
  "CLIMB IN THE CANNON. FLY FAR.\n"
  "SPREAD THE GOOD WORD.\n"
  "SAVE EVERY SOUL YOU CAN.\n"
  "BLESS YOU. MIND THE CACTUS.";
uint16_t storyN;
bool drawStory() {     // returns true once the whole text is showing
  txt(4, 0, F("BY ORDER OF THE POPE"));
  ab.drawFastHLine(0, 8, 128, WHITE);
  int16_t x = 2, y = 12; uint16_t i = 0; char c;
  while ((c = pgm_read_byte(&STORY[i])) && i < storyN) {
    if (c == '\n') { x = 2; y += 6; }
    else { for (uint8_t k = 0; k < 3; k++) col8(x + k, y, pgm_read_byte(&TINY[(c - 32) * 3 + k])); x += 4; }
    i++;
  }
  bool done = !pgm_read_byte(&STORY[i]);
  if (!done) { if (tick % 3 == 0) storyN++; if (tick & 8) ab.fillRect(x, y, 3, 5, WHITE); }   // blinking cursor
  else if (tick & 32) tiny(32, 58, F("PRESS ANY BUTTON"));
  return done;
}

// sound screen: a church bell. ON: it swings and rings, DING... DONG. OFF: it hangs still. SHHH...
uint8_t popT;
void drawSound() {
  ab.drawRect(0, 0, 128, 64, WHITE);
  txt2(34, 3, F("SOUND"));
  bool on = ab.audio.enabled();
  int8_t j = -(popT >> 1); if (popT) popT--;
  ab.drawFastHLine(54, 22, 20, WHITE); ab.drawFastVLine(63, 22, 3, WHITE);      // beam and rope
  uint8_t f = on ? (tick >> 4) & 1 : 0;
  spr(56 + (on ? (f ? 1 : -1) : 0), 24, FX_BELL, f);
  if (on) {
    for (uint8_t k = 1; k <= ((tick >> 2) & 3); k++) {                    // ringing lines on both sides
      ab.drawFastVLine(53 - k * 3, 31 - k, 3 + k * 2, WHITE); ab.drawFastVLine(74 + k * 3, 31 - k, 3 + k * 2, WHITE);
    }
    if (f) tiny(88, 30, F("DONG")); else tiny(25, 30, F("DING"));
  } else {
    static const char SH[] PROGMEM = "SHHH...";
    uint8_t n = (tick >> 3) & 15; if (n > 7) n = 7;                       // typed out a letter at a time
    for (uint8_t i = 0; i < n; i++) { char c = pgm_read_byte(&SH[i]); for (uint8_t q = 0; q < 3; q++) col8(80 + i * 4 + q, 30, pgm_read_byte(&TINY[(c - 32) * 3 + q])); }
  }
  txt(on ? 46 : 43, 44 + j, on ? F("< ON >") : F("< OFF >"));
  if (tick & 32) tiny(40, 56, F("PRESS A OR B"));
}

void drawTitle() {
  FX::drawBitmap(0, 0, FX_TITLEANIM, (tick >> 2) & 31, dbmNormal);
  char b[12]; txt(2, 33, F("BEST")); ltoa(best, b, 10); txt(32, 33, b);
  if (tick & 32) { txt(2, 45, F("PRESS ANY")); txt(2, 54, F("BUTTON")); }
}

void setup() {
  ab.boot(); ab.display(); FX::begin(FX_DATA_PAGE); ab.audio.begin(); ab.setFrameRate(60); ab.initRandomSeed();
  EEPROM.get(64, best); best ^= 0xC0FFEE00UL; if (best > 9999999UL) best = 0;
}

void loop() {
  if (!ab.nextFrame()) return;
  ab.pollButtons(); tick++;
  ab.clear();
  switch (mode) {
    case M_TITLE:
      drawTitle();
      if (ab.justPressed(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON)) { mode = M_SOUND; tick = 0; }
      break;
    case M_SOUND:                                                          // sound on or off, then the story
      drawSound();
      if (tick > 10 && ab.justPressed(UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON)) {
        ab.audio.toggle(); ab.audio.saveOnOff(); popT = 8;
        if (ab.audio.enabled()) snd.tone(NOTE_A5, 120, NOTE_E5, 180);          // ding-dong
      }
      if (tick > 10 && ab.justPressed(A_BUTTON | B_BUTTON)) { mode = M_STORY; storyN = 0; }
      break;
    case M_STORY: {
      bool done = drawStory();
      if (ab.justPressed(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON)) { if (done) mode = M_CONTROLS; else storyN = 999; }
      break;
    }
    case M_CONTROLS:   // picture of the Arduboy with the controls labelled
      FX::drawBitmap(0, 0, FX_CONTROLS, (tick >> 4) & 1, dbmNormal);
      if (ab.justPressed(BTN_GO)) newGame();
      if (ab.justPressed(BTN_ALT)) mode = M_TITLE;
      break;
    case M_AIM:
    case M_POWER:
      { // up/left raise the cannon, down/right lower it; it eases in and speeds up while held
        int8_t dir = (ab.anyPressed(UP_BUTTON | LEFT_BUTTON) ? 1 : 0) - (ab.anyPressed(DOWN_BUTTON | RIGHT_BUTTON) ? 1 : 0);
        if (dir) { aimV += dir * 0.06; if (aimV > 1.2) aimV = 1.2; if (aimV < -1.2) aimV = -1.2; if (aimV * dir < 0.25) aimV = dir * 0.25; }
        else aimV *= 0.6;
        aimF += aimV;
        if (aimF > 80) { aimF = 80; aimV = 0; } if (aimF < 10) { aimF = 10; aimV = 0; }
        angleDeg = (int8_t)(aimF + 0.5);
      }
      if (mode == M_AIM) {
        if (ab.justPressed(BTN_GO)) { mode = M_POWER; power = 0; pdir = 1; }
        if (ab.justPressed(BTN_ALT)) mode = M_TITLE;
      } else {
        power += pdir * 0.032;
        if (power >= 1) { power = 1; pdir = -1; }
        if (power <= 0) { power = 0; pdir = 1; }
        if (ab.justPressed(BTN_GO)) fire();
      }
      camX = -40; camH = 0;
      drawBackground(); drawGround(); drawGroundThings(); drawAirThings();
      { // man's helmet peeking from the muzzle
        // his head pokes out of the muzzle; the barrel is drawn over his chin
        int16_t x = sx(CANNON_X + icos(angleDeg) * 34 / 256.0), y = sy(PIVOT_H + isin(angleDeg) * 34 / 256.0);
        spr(x - 9, y - 12, FX_PEEK, 0);
        drawCannon(0);
      }
      { // small instructions so the circus tent shows
        ab.fillRect(0, 0, 50, 8, BLACK);
        int16_t x;
        x = mode == M_AIM ? tiny(1, 1, F("AIM ")) : tiny(1, 1, F("POWER "));
        x = tinyN(x, 1, angleDeg);
        pset(x, 1); pset(x + 1, 1); pset(x, 2); pset(x + 1, 2);      // degree sign
      }
      if (mode == M_POWER) drawPowerBar();
      break;
    case M_FLY:
      stepFly();
      if (shakeT) { shakeT--; shx = random(-2, 3); shy = random(-2, 3); } else shx = shy = 0;
      drawBackground(); drawGround(); drawCrowds(); drawGroundThings(); drawAirThings();
      drawCannon(fireT < 10 ? (10 - fireT) >> 1 : 0);
      drawParticles(); drawBibles(); drawMan(); drawEffects(); drawPops(); drawHud();
      if (stopped && doneT >= 70) {
        drawResult();
        if (ab.justPressed(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON)) mode = M_TITLE;
      }
      break;
  }
  if (mode <= M_POWER || mode == M_SOUND) { if (!snd.playing()) snd.tones(SONG); }   // menu and aiming music; firing cuts in with the BOOM
  FX::display();
}
