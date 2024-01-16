#pragma once

// Custom screen address
static char * const Screen0 = (char *)0xf000;
static char * const Screen1 = (char *)0xf400;

// Custom spriteset address
static char * const Sprites = (char *)0xd000;

// Custom charset address
static char * const Charset = (char *)0xf800;

// Color mem address
static char * const Color = (char *)0xd800;

extern const char LevelAttr[];

extern char tile_buffer[32][64];

extern __striped char * const tilerows[32 + 4];

extern unsigned levely;
extern char screenx, screeny;
extern char screeni;
extern char phase;
extern int vscreenx;

#define SPIMAGE_PLAYER			0x40
#define SPIMAGE_SHOT			0x43
#define SPIMAGE_UFO				0x44
#define SPIMAGE_BULLET			0x45
#define SPIMAGE_GUN				0x46
#define SPIMAGE_LEFTGUARD		0x49
#define SPIMAGE_RIGHTGUARD		0x4a
#define SPIMAGE_RETRO			0x4b
#define SPIMAGE_EVDOOR			0x4c
#define SPIMAGE_EXPLOSION		0x50
#define SPIMAGE_STAR			0x60
#define SPIMAGE_COIN			0x64

#define SPIMAGE_POPCORN			0x68

#define SPIMAGE_SHIP_1			0x70
#define SPIMAGE_SHIP_2			0x74
#define SPIMAGE_SHIP_3			0x78
#define SPIMAGE_SHIP_4			0x7c

#define SPIMAGE_BOMBER_LEFT		0x80
#define SPIMAGE_BOMBER_RIGHT	0x84
#define SPIMAGE_CORVETTE		0x88
#define SPIMAGE_PINGPONG		0x8c
#define SPIMAGE_SPHERE			0x90
#define SPIMAGE_ALIEN			0x94

#define SPIMAGE_DESTROYER		0x98
#define SPIMAGE_LASER			0x9a
#define SPIMAGE_FRIGATE			0x9b
#define SPIMAGE_MISSILE			0x9d
#define SPIMAGE_BOSS			0xa0

#define SPIMAGE_SCORE			0xa6

#define SPIMAGE_TEXT_1			0xb0
#define SPIMAGE_TEXT_2			0xb8

void display_init(void);

void background_init(char bg);

void level_init(const char * seq, const char * wave, char lsize);

void rebuild_screen(char phase);

void display_loop(void);

void tile_remove(char x, char y);

#pragma compile("display.cpp")

