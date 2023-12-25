#include "display.h"
#include <c64/vic.h>
#include <c64/memmap.h>
#include <c64/joystick.h>
#include <c64/sprites.h>
#include <c64/rasterirq.h>
#include <oscar.h>
#include <string.h>
#include <c64/cia.h>
#include "gamemusic.h"
#include "player.h"
#include "enemies.h"

#define TIME_DEBUG	0


// The charpad resource in lz compression, without the need
// for binary export
const char LevelFont[] = {
	#embed ctm_chars lzo "tiles.ctm"
};

__export const char LevelAttr[] = {
	#embed ctm_attr1 "tiles.ctm"
};

// The tile char map mapping tile indices to blocks of 4x4 char indices
const char LevelTiles[] = {
	#embed ctm_tiles8 "tiles.ctm"
};
#pragma align(LevelTiles, 256)

// The tile map, a 16x64 grid of 8 bit tile indices
const char LevelMap[] = {
	#embed ctm_map8 "tiles.ctm"
};
#pragma align(LevelMap, 256)

// Ship sprite image
char const SpriteImages[] = {
	#embed spd_sprites lzo "sprites.spd"
};

// Temporary buffer for expanded tiles
char tile_buffer[32][64];
#pragma align(tile_buffer, 256)

__striped char * const tilerows[32 + 4] = {
#for(i,36) tile_buffer[i & 31],
};

char tile_cache[8][16];

static const char LevelSequence[] = {
	0, 0, 11, 11, 25, 41, 42, 0, 0, 54, 0, 53, 47, 51, 48, 49, 
	51, 50, 51, 52, 0, 46, 45, 5, 44, 6, 7, 43, 8, 42, 41, 39, 
	40, 0, 37, 38, 35, 36, 0, 33, 34, 31, 32, 27, 28, 29, 30, 21, 
	24, 12, 13, 25, 26, 14, 15, 24, 21, 22, 23, 21, 19, 20, 21, 12, 
	13, 14, 15, 0, 11, 7, 8, 11, 56, 57, 5, 55, 4, 0, 1, 2, 
};

static const char LevelRows[] = {
	0, 1, 2, 3, 
	112, 113, 114, 115, 
	116, 113, 117, 118, 
	0, 4, 5, 6, 
	0, 7, 8, 9, 
	0, 10, 11, 12, 
	13, 14, 15, 3, 
	0, 16, 17, 18, 
	19, 20, 21, 3, 
	0, 22, 23, 24, 
	25, 26, 27, 3, 
	36, 36, 36, 36, 
	0, 1, 92, 93, 
	94, 95, 96, 97, 
	98, 99, 100, 101, 
	102, 103, 2, 3, 
	0, 80, 85, 86, 
	87, 88, 89, 90, 
	91, 84, 2, 3, 
	0, 80, 119, 81, 
	82, 83, 119, 84, 
	0, 56, 2, 3, 
	0, 49, 50, 51, 
	52, 53, 54, 55, 
	0, 16, 28, 21, 
	0, 1, 75, 76, 
	77, 78, 79, 3, 
	44, 104, 105, 106, 
	107, 108, 109, 110, 
	111, 104, 105, 106, 
	107, 120, 2, 3, 
	0, 44, 72, 73, 
	74, 48, 2, 3, 
	0, 44, 45, 46, 
	47, 48, 2, 3, 
	0, 29, 30, 31, 
	32, 33, 34, 35, 
	4, 37, 38, 39, 
	39, 40, 41, 9, 
	7, 41, 121, 42, 
	42, 43, 37, 6, 
	0, 4, 57, 6, 
	0, 7, 58, 9, 
	19, 59, 61, 18, 
	60, 14, 11, 60, 
	0, 4, 62, 6, 
	0, 7, 63, 9, 
	0, 1, 80, 119, 
	128, 129, 130, 140, 
	131, 132, 133, 140, 
	134, 135, 136, 140, 
	137, 138, 139, 140, 
	140, 84, 2, 3, 
	141, 141, 141, 141, 
	142, 142, 142, 142, 
	13, 143, 144, 3, 
	0, 69, 145, 146, 
	147, 148, 48, 3, 
};
#if 0
static const char LevelSequence[256] = {
	0, 1, 2, 3,
	10, 11, 12, 13, 
	14, 15, 2, 3,
	
	7, 8, 9, 3,
	10, 11, 60, 13, 
	14, 15, 2, 3,

	16, 17, 18, 19,
	20, 21, 2, 3,

	4, 5, 6, 3,
	22, 23, 24, 25,
	26, 27, 2, 3,

	16, 28, 21, 3,
	0, 1, 2, 16,
	28, 21, 2, 3,

	49, 50, 51, 52,
	53, 54, 55, 3,

	0, 1, 2, 3,

	29, 30, 31, 32,
	33, 34, 35, 3,

	0, 1, 2, 3,
	4, 37, 37, 38,
	39, 39, 39, 39,
	39, 40, 41, 9,

	0, 56, 2, 56,

	36, 36, 36, 36,
	36, 36, 36, 36,
	0, 1, 2, 44,
	45, 46, 47, 48,

	0, 1, 2, 16,
	
	17, 59, 19, 61,
	17, 61, 59, 20,
	17, 61, 19, 20,
	21, 1, 2, 16,
	
	17, 61, 59, 61,
	17, 59, 19, 20,
	21, 1, 2, 7,
	8, 58, 8, 9,
	
	0, 4, 5, 57,
	5, 6, 2, 3,
	
	4, 62, 6, 3,
	7, 63, 9, 3,
	0, 1, 2, 3,
	
	64, 65, 66, 67,
	67, 68, 2, 3,
	69, 70, 71, 67,
	67, 68, 2, 3,
	
	44, 72, 73, 74,
	48, 1, 2, 80,
	81, 82, 83, 84,
	0, 1, 75, 76,
	77, 78, 79, 3,
	
	80, 85, 86, 87,
	88, 89, 90, 91,

	84, 1, 2, 3,
	92, 93, 94, 95,

	96, 97, 98, 99,
	100, 101, 102, 103,

	0, 1, 2, 3,
	29, 104, 105, 106,
	107, 108, 109, 110,
	111, 104, 105, 106,
	107, 35, 2, 3,

	0, 1, 2, 3,
	0, 1, 2, 3,
};
#endif

void copy_screen_rows3(char * sp, char sx, char sy)
{
	sy &= 31;

	const char * mp0 = tilerows[sy + 0] + sx;
	const char * mp1 = tilerows[sy + 1] + sx;
	const char * mp2 = tilerows[sy + 2] + sx;

	for(signed char x=39; x>=0; x--)
	{
		sp[x +   0] = mp0[x];
		sp[x +  40] = mp1[x];
		sp[x +  80] = mp2[x];
	}	
}

void copy_screen_rows4(char * sp, char sx, char sy)
{
	sy &= 31;

	const char * mp0 = tilerows[sy + 0] + sx;
	const char * mp1 = tilerows[sy + 1] + sx;
	const char * mp2 = tilerows[sy + 2] + sx;
	const char * mp3 = tilerows[sy + 3] + sx;

	for(signed char x=39; x>=0; x--)
	{
		sp[x +   0] = mp0[x];
		sp[x +  40] = mp1[x];
		sp[x +  80] = mp2[x];
		sp[x + 120] = mp3[x];
	}	
}


// Expand a row of tiles into tile buffer
void expand_tiles(unsigned sy, char ry, char dy)
{
	// Get source tile row
	const char * sp = tile_cache[sy & 7];

	// Get target tile buffer pointers
	char * dp = tilerows[dy + ry];

	// We have 16 tiles (64 chars) for tile buffer
	for(char tx=0; tx<16; tx++)
	{
		// Get actual tile
		char t = sp[tx];

		// Get tile char map address for this tile
		const char * tp = LevelTiles + (t * 16 | 4 * ry);

		// Expand 4x4 block into tile buffer
		#pragma unroll(full)
		for(char x=0; x<4; x++)
			dp[x] = tp[x];

		dp += 4;
	}
}

unsigned levely;
char screenx, screeny, pscreenx;
char screeni;

void rebuild_screen(char phase)
{
	char * screen = screeni ? Screen0 : Screen1;

	switch (phase & 7)
	{
	case 7:
		if (!(screeny & 3))
		{
			levely--;
			memcpy(tile_cache[levely & 7], LevelMap + 16 * LevelRows[LevelSequence[levely >> 2] * 4 + (levely & 3)], 16);
		}

		expand_tiles(levely, screeny & 3, ((levely - 1) & 7) * 4);

		screeny--;
		break;
	case 0:
		copy_screen_rows4(screen + 40 *  0, screenx, screeny +  0);
		break;
	case 1:
		copy_screen_rows3(screen + 40 *  4, screenx, screeny +  4);
		break;
	case 2:
		copy_screen_rows4(screen + 40 *  7, screenx, screeny +  7);
		break;
	case 3:
		copy_screen_rows3(screen + 40 * 11, screenx, screeny + 11);
		break;
	case 4:
		copy_screen_rows4(screen + 40 * 14, screenx, screeny + 14);
		break;
	case 5:
		copy_screen_rows3(screen + 40 * 18, screenx, screeny + 18);
		break;
	case 6:
		copy_screen_rows4(screen + 40 * 21, screenx, screeny + 21);
		break;
	}
}

char LevelScrollFont[4][2][2][32];
#pragma align(LevelScrollFont, 256)


char phase;

void display_init(void)
{
	cia_init();

	mmap_trampoline();

	mmap_set(MMAP_RAM);

	// Expand charset
	oscar_expand_lzo(Sprites, SpriteImages);

	mmap_set(MMAP_NO_ROM);

	// Expand charset
	oscar_expand_lzo(Charset, LevelFont);

	for(char i=0; i<8; i++)
		Charset[255 * 8 + i] = 0xff;

	// Clear screen, prepare color area for multicolor
	memset(Color, VCOL_WHITE + 8, 1000);
	memset(Screen0, 0, 1000);
	memset(Screen1, 0, 1000);

	// Background and border colors
	vic.color_border = VCOL_BLUE;
	vic.color_back = VCOL_LT_BLUE;
	vic.color_back1 = VCOL_BLACK;
	vic.color_back2 = VCOL_LT_GREY;

	// Set VIC to show custom screen with custom charset and multicolor
	vic_setmode(VICM_TEXT_MC, Screen0, Charset);

	// Init the raster IRQ system to use the kernal iterrupt vector
	rirq_init_io();

	vic.spr_mcolor0 = VCOL_BLACK;
	vic.spr_mcolor1 = VCOL_WHITE;
	vic.spr_multi = 0xff;

	vspr_init(Screen0);

	// We have two sets of background 2x2 chars, one with and
	// one without shadows
	for(char k=0; k<2; k++)
	{
		// Two chars vertical is 16 lines
		for(char j=0; j<16; j++)
		{
			// Get a 16 bit version of the two neighbouring chars
			unsigned u = (Charset[j + 32 * k] << 8) | Charset[j + 16 + 32 * k];

			// Create four scrolled versions
			for(char i=0; i<4; i++)
			{
				// Cache scrolled lines
				LevelScrollFont[i][k][0][j     ] = u >> 8;
				LevelScrollFont[i][k][0][j + 16] = u >> 8;
				LevelScrollFont[i][k][1][j     ] = u & 0xff;
				LevelScrollFont[i][k][1][j + 16] = u & 0xff;

				// Rotate two bits to the left
				u = (u << 2) | (u >> 14);
			}
		}
	}

	memset(Charset + 64, 0x55, 128);


	for(char i=0; i<8; i++)
		vspr_set(i + 8, 10, 10, 68, 4 + i);
	
	vspr_sort();
	vspr_update();
	rirq_sort();

	rirq_start();
}

int vscreenx;
char px = 4;
signed char ndx = 0;
char	starp[4];
signed char dx = 0;

void level_init(void)
{
	levely = sizeof(LevelSequence) * 4;
	screeny = 0;

	for(char i=0; i<8; i++)
	{
		levely--;
		memcpy(tile_cache[levely & 7], LevelMap + 16 * LevelRows[LevelSequence[levely >> 2] * 4 + (levely & 3)], 16);
		expand_tiles(levely, 0, ((levely - 1) & 7) * 4);
		expand_tiles(levely, 1, ((levely - 1) & 7) * 4);
		expand_tiles(levely, 2, ((levely - 1) & 7) * 4);
		expand_tiles(levely, 3, ((levely - 1) & 7) * 4);
	}
	px = 4;
	dx = 0;
	ndx = 0;
	vscreenx = 96;
	pscreenx = screenx = 12;

	for(char i=0; i<8; i++)
		rebuild_screen(i);
	phase = 7;
}

void display_loop(void)
{
	phase++;		

	if ((phase & 7) == 4)
	{
		dx = ndx;
		int tscreenx = (shipx - 64) >> 1;

		if (vscreenx + 16 < tscreenx)
			ndx = -1;
		else if (vscreenx > tscreenx + 16)
			ndx = 1;
		else
			ndx = 0;//joyx[0];

		if (ndx > 0 && screenx == 0)
			ndx = 0;
		else if (ndx < 0 && screenx == 25)
			ndx = 0;

		if (dx >= 0)
			px = (px & ~7) | 4;
		else
			px = (px & ~7) | 3;
	}

#if TIME_DEBUG
	vic.color_border = VCOL_BLACK;
#endif
	vspr_sort();
#if TIME_DEBUG
	vic.color_border = VCOL_LT_BLUE;
#endif

	music_play();

	rirq_wait();

	if (!(phase & 7))
	{
		screeni = 1 - screeni;
		vic.memptr = (vic.memptr & 0xee) | (screeni << 4);
		vspr_screen(screeni ? Screen1 : Screen0);
		pscreenx = screenx;
		screenx -= ndx;

		if (!(screeny & 3))
		{
			const char * rt = tile_cache[levely & 7];

			char si = 0;
			for(char i=0; i<16; i++)
			{
				switch (rt[i])
				{
				case 20:
					enemies_add(32 + 32 * i, 14, ET_GUN, 0, 0);
					break;
				case 26:
					enemies_add(32 + 32 * i, 40, ET_EVDOOR, 0, 0);
					break;
				case 28:
					si = i;
					break;
				case 29:
					enemies_add(64 + 32 * si, 14, ET_PINGPONG, 64 + 32 * si, 4 + 32 * i);
					break;
				case 45:
					enemies_add(28 + 32 * i, 6, ET_LEFTGUARD, 0, 0);
					break;
				case 46:
					enemies_add(36 + 32 * i, 6, ET_RIGHTGUARD, 0, 0);
					break;
				}
			}

			if ((levely & 31) == 16)
			{
				enemies_add(256, 14, ET_UFO, 0, 0);				
				enemies_add(256, 14, ET_UFO, 0, 0);
			}

		}
	}

	vic.ctrl1 = VIC_CTRL1_DEN | (phase & 7);
	vic.ctrl2 = VIC_CTRL2_MCM | (px & 7);

	vspr_update();

	// sort raster IRQs
	rirq_sort();

	// Offset of parallax scrolled regions, half in x due
	// to multicolor mode
	char	ty = (phase >> 1) & 15;
	char	tx = (px >> 2) & 7;

	// Char and pixel components of x
	char	rx = tx >> 2;
	tx &= 3;

	// Copy the char data from the cached shifted chars
	for(char i=0; i<16; i++)
	{
		Charset[i +  0] = LevelScrollFont[tx][0][1 - rx][ty];
		Charset[i + 16] = LevelScrollFont[tx][0][    rx][ty];

		Charset[i + 32] = LevelScrollFont[tx][1][1 - rx][ty];
		Charset[i + 48] = LevelScrollFont[tx][1][    rx][ty];

		// Next line
		ty++;
	}

	static const char yo[4] = {10, 23, 45, 38};

	for(char i=0; i<4; i++)
	{
		char x = (4 * i - px) & 15;
		char y = (yo[i] - phase) & 63;

		char p = (y & 7) + (y * 2 & 0x70) + (x & 0x8);
		Charset[64 + starp[i]] = 0x55;
		starp[i] = p;
		Charset[64 + p] = 0x55 | (128 >> (x & 6));
	}


	music_play();

#if TIME_DEBUG
	vic.color_border = VCOL_WHITE;
#endif
	rebuild_screen(phase);
#if TIME_DEBUG
	vic.color_border = VCOL_LT_BLUE;
#endif

	vscreenx -= dx;
	if (dx < 0)
		px--;
	else if (dx > 0)
		px++;
}


void tile_replace(char sx, char sy, char ti)
{
	const char * tp = LevelTiles + ti * 16;

	char * screen0 = screeni ? Screen1 : Screen0;
	char * screen1 = screeni ? Screen0 : Screen1;

	char y = ((sy - 1) & 7) * 4;
	char x = sx * 4;

	char scy = screeny;
	if ((phase & 7) == 7)
		scy++;

	for(char i=0; i<4; i++)
	{
		char * scl = tilerows[(y + i) & 31];
		scl[x + 0] = tp[0];
		scl[x + 1] = tp[1];
		scl[x + 2] = tp[2];
		scl[x + 3] = tp[3];
		
		tp += 4;
	}

	char y1 = (y - scy) & 31;
	char n = 4;
	if (y1 > 28)
	{
		n = y1 & 3;
		y1 = 0;
	}

	if (x >= screenx && x < screenx + 36)
	{
		if (y1 < 25)
		{
			char * sp = screen1 + 40 * y1 + (x - screenx);

			for(char i=0; i<n; i++)
			{
				char * scl = tilerows[(y1 + scy + i) & 31];
				sp[0] = scl[x + 0];
				sp[1] = scl[x + 1];
				sp[2] = scl[x + 2];
				sp[3] = scl[x + 3];
				sp += 40;
			}

			char y0 = (y - scy - 1) & 31;
			n = 4;
			if (y0 > 28)
			{
				n = y0 & 3;
				y0 = 0;
			}
			if (y0 < 25)
			{
				sp = screen0 + 40 * y0 + (x - pscreenx);
				for(char i=0; i<n; i++)
				{
					sp[0] = 0xff;
					sp[1] = 0xff;
					sp[2] = 0xff;
					sp[3] = 0xff;
					sp += 40;
				}
			}
		}
	}
}

void tile_collide(char x, char y)
{
	unsigned sy = (y >> 2) + levely + 1, sx = x >> 2;

	char * lp  = tile_cache[sy & 7];

	char ti = lp[sx];

	switch (ti)
	{
	case 22:
		ti = 0;
		break;
	case 21:
		ti = 11;
		break;
	case 23:
		ti = 10;
		break;
	case 24:
		ti = 8;
		break;
	case 25:
		ti = 14;
		break;
	case 33:
		ti = 34;
		break;
	case 34:
		ti = 33;
		break;
	case 41:
		ti = 12;
		break;
	case 42:
		ti = 13;
		break;
	case 43:
		ti = 22;
		break;
	case 44:
		ti = 43;
		break;
	default:
		return;
	}

	lp[sx] = ti;
	tile_replace(sx, sy, ti);

	if (ti == 34)
	{
		for(char i=0; i<16; i++)
		{
			ti = lp[i];
			if (ti == 31)
				ti = 0;
			else if (ti == 30)
				ti = 11;
			else if (ti == 32)
				ti = 10;
			else
				continue;

			lp[sx] = ti;
			tile_replace(i, sy, ti);
		}
	}
}

