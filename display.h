#pragma once

// Custom screen address
static char * const Screen0 = (char *)0xc000;
static char * const Screen1 = (char *)0xc400;

// Custom spriteset address
static char * const Sprites = (char *)0xd000;

// Custom charset address
static char * const Charset = (char *)0xc800;

// Color mem address
static char * const Color = (char *)0xd800;

extern const char LevelAttr[];

extern char tile_buffer[32][64];

extern __striped char * const tilerows[32 + 4];

extern unsigned levely;
extern char screenx, screeny;
extern char screeni;
extern char LevelScrollFont[4][2][2][32];
extern char phase;
extern int vscreenx;

void display_init(void);

void level_init(const char * seq, const char * wave, char lsize);

void rebuild_screen(char phase);

void display_loop(void);

void tile_remove(char x, char y);

#pragma compile("display.cpp")

