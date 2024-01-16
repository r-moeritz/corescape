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
#include "levelrows.h"

// The charpad resource in lz compression, without the need
// for binary export
__export const char LevelAttr[] = {
	#embed ctm_attr1 "tiles.ctm"
};

// The tile char map mapping tile indices to blocks of 4x4 char indices
const char LevelTiles[] = {
	#embed ctm_tiles8sw "tiles.ctm"
};
#pragma align(LevelTiles, 256)

// The tile map, a 16x64 grid of 8 bit tile indices
const char LevelMap[] = {
	#embed ctm_map8 "tiles.ctm"
};
#pragma align(LevelMap, 256)


// Background tiles
const char BackgroundFont[] = {
	#embed ctm_chars "background.ctm"
};

// Temporary buffer for expanded tiles
#pragma bss(xbss)
char tile_buffer[32][64];
#pragma bss(bss)

#pragma align(tile_buffer, 256)


__striped char * const tilerows[32 + 4] = {
#for(i,36) tile_buffer[i & 31],
};

const char * level_seq;
const char * level_wave;

#pragma bss(xbss)
char tile_cache[8][16];
#pragma align(tile_cache, 256)
#pragma bss(bss)

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
void expand_tiles(char sy, char ry, char dy)
{
	// Get source tile row
	const char * sp = tile_cache[sy & 7];

	// Get target tile buffer pointers
	char * dp = tilerows[dy + ry];

	// Get tile char map address for this tile
	const char * tp0 = LevelTiles + NUM_TILES * (4 * ry + 0);
	const char * tp1 = LevelTiles + NUM_TILES * (4 * ry + 1);
	const char * tp2 = LevelTiles + NUM_TILES * (4 * ry + 2);
	const char * tp3 = LevelTiles + NUM_TILES * (4 * ry + 3);

	// We have 16 tiles (64 chars) for tile buffer
	for(char tx=0; tx<16; tx++)
	{
		// Get actual tile
		char t = sp[tx];


		// Expand 4x4 block into tile buffer
		dp[0] = tp0[t];
		dp[1] = tp1[t];
		dp[2] = tp2[t];
		dp[3] = tp3[t];

		dp += 4;
	}
}

void expand_tiles_0(char sy, char dy)
{
	// Get source tile row
	const char * sp = tile_cache[sy & 7];

	// Get target tile buffer pointers
	char * dp = tilerows[dy + 0];

	// We have 16 tiles (64 chars) for tile buffer
	for(char tx=0; tx<16; tx++)
	{
		// Get actual tile
		char t = sp[tx];

		// Expand 4x4 block into tile buffer
		dp[ 0] = LevelTiles[0 * NUM_TILES + t];
		dp[ 1] = LevelTiles[1 * NUM_TILES + t];
		dp[ 2] = LevelTiles[2 * NUM_TILES + t];
		dp[ 3] = LevelTiles[3 * NUM_TILES + t];
		dp[64] = LevelTiles[4 * NUM_TILES + t];
		dp[65] = LevelTiles[5 * NUM_TILES + t];
		dp[66] = LevelTiles[6 * NUM_TILES + t];
		dp[67] = LevelTiles[7 * NUM_TILES + t];

		dp += 4;
	}
}

void expand_tiles_1(char sy, char dy)
{
	// Get source tile row
	const char * sp = tile_cache[sy & 7];

	// Get target tile buffer pointers
	char * dp = tilerows[dy + 2];

	// We have 16 tiles (64 chars) for tile buffer
	for(char tx=0; tx<16; tx++)
	{
		// Get actual tile
		char t = sp[tx];

		// Expand 4x4 block into tile buffer
		dp[ 0] = LevelTiles[ 8 * NUM_TILES + t];
		dp[ 1] = LevelTiles[ 9 * NUM_TILES + t];
		dp[ 2] = LevelTiles[10 * NUM_TILES + t];
		dp[ 3] = LevelTiles[11 * NUM_TILES + t];
		dp[64] = LevelTiles[12 * NUM_TILES + t];
		dp[65] = LevelTiles[13 * NUM_TILES + t];
		dp[66] = LevelTiles[14 * NUM_TILES + t];
		dp[67] = LevelTiles[15 * NUM_TILES + t];

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
		switch (screeny & 3)
		{
		case 0:
			levely--;
			memcpy(tile_cache[levely & 7], LevelMap + 16 * LevelRows[level_seq[levely >> 2] * 4 + (levely & 3)], 16);
			break;
		case 1:
			expand_tiles_0(levely, ((levely - 1) & 7) * 4);
			break;
		case 2:
			expand_tiles_1(levely, ((levely - 1) & 7) * 4);
			break;
		case 3:
			break;
		}
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

#pragma bss(xbss)
char LevelScrollFont[2][2][4 * 32];
#pragma align(LevelScrollFont, 256)
#pragma bss(bss)


char phase;

void background_init(char bg)
{
	// We have two sets of background 2x2 chars, one with and
	// one without shadows
	for(char k=0; k<2; k++)
	{
		// Two chars vertical is 16 lines
		for(char j=0; j<16; j++)
		{
			char t = (j & 7) + ((j & 8) << 1) + k * 32 + 64 * bg;

			// Get a 16 bit version of the two neighbouring chars
			unsigned u = (BackgroundFont[t] << 8) | BackgroundFont[t + 8];

			// Create four scrolled versions
			for(char i=0; i<4; i++)
			{
				// Cache scrolled lines
				LevelScrollFont[k][0][i * 32 + j     ] = u >> 8;
				LevelScrollFont[k][0][i * 32 + j + 16] = u >> 8;
				LevelScrollFont[k][1][i * 32 + j     ] = u & 0xff;
				LevelScrollFont[k][1][i * 32 + j + 16] = u & 0xff;

				// Rotate two bits to the left
				u = (u << 2) | (u >> 14);
			}
		}
	}

	char ty = 0;
	for(char i=0; i<16; i++)
	{
		Charset[i +  0] = LevelScrollFont[0][1][ty];
		Charset[i + 16] = LevelScrollFont[0][0][ty];

		Charset[i + 32] = LevelScrollFont[1][1][ty];
		Charset[i + 48] = LevelScrollFont[1][0][ty];

		// Next line
		ty++;
	}			
}

void display_fade_out(void)
{
	vic_waitBottom();
	vic_waitTop();
	vic.color_border = VCOL_BLUE;
	vic.color_back = VCOL_LT_BLUE;
	vic.color_back1 = VCOL_BLACK;
	vic.color_back2 = VCOL_LT_GREY;

	vic_waitFrames(2);
	vic_waitTop();

	vic.spr_enable = 0x00;
	vic.color_border = VCOL_LT_BLUE;
	vic.color_back = VCOL_WHITE;
	vic.color_back1 = VCOL_MED_GREY;
	vic.color_back2 = VCOL_WHITE;

	vic_waitFrames(2);
	vic_waitTop();

	vic.color_border = VCOL_WHITE;
	vic.color_back = VCOL_WHITE;
	vic.color_back1 = VCOL_WHITE;
	vic.color_back2 = VCOL_WHITE;

	vic_waitBottom();
}

void display_fade_in(void)
{
	vic_waitBottom();
	vic_waitTop();
	vic.color_border = VCOL_WHITE;
	vic.color_back = VCOL_WHITE;
	vic.color_back1 = VCOL_WHITE;
	vic.color_back2 = VCOL_WHITE;

	vic_waitFrames(2);
	vic_waitTop();

	vic.color_border = VCOL_LT_BLUE;
	vic.color_back = VCOL_WHITE;
	vic.color_back1 = VCOL_MED_GREY;
	vic.color_back2 = VCOL_WHITE;

	vic_waitFrames(2);
	vic_waitTop();

	vic.spr_enable = 0xff;
	vic.color_border = VCOL_BLUE;
	vic.color_back = VCOL_LT_BLUE;
	vic.color_back1 = VCOL_BLACK;
	vic.color_back2 = VCOL_LT_GREY;

	vic_waitBottom();
}

void display_init(void)
{
	// White flash char, can't use 255 - would clobber up interrupt vector
	for(char i=0; i<8; i++)
		Charset[254 * 8 + i] = 0xff;

	// Clear screen, prepare color area for multicolor
	memset(Color, VCOL_WHITE + 8, 1000);
	memset(Screen0, 0, 1000);
	memset(Screen1, 0, 1000);

	// Background and border colors
	vic.color_border = VCOL_WHITE;
	vic.color_back = VCOL_WHITE;
	vic.color_back1 = VCOL_WHITE;
	vic.color_back2 = VCOL_WHITE;

	// Set VIC to show custom screen with custom charset and multicolor
	vic_setmode(VICM_TEXT_MC, Screen0, Charset);

	// Init the raster IRQ system to use the kernal iterrupt vector
	rirq_init_io();

	vic.spr_mcolor0 = VCOL_BLACK;
	vic.spr_mcolor1 = VCOL_WHITE;
	vic.spr_multi = 0xff;

	vspr_init(Screen0);

	background_init(3);

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

void level_init(const char * seq, const char * wave, char lsize)
{
	level_seq = seq;
	level_wave = wave;
	levely = lsize * 4;
	screeny = 0;

	for(char i=0; i<8; i++)
	{
		levely--;
		memcpy(tile_cache[levely & 7], LevelMap + 16 * LevelRows[level_seq[levely >> 2] * 4 + (levely & 3)], 16);
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

	screeni = 1 - screeni;
	vic.memptr = (vic.memptr & 0xee) | (screeni << 4);
	vspr_screen(screeni ? Screen1 : Screen0);

	vic.ctrl1 = VIC_CTRL1_DEN;
	vic.ctrl2 = VIC_CTRL2_MCM | (px & 7);	

	phase = 0;
	rebuild_screen(phase);
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

	if (!(phase & 7))
	{
#if TIME_DEBUG
		vic.color_border = VCOL_YELLOW;
#endif

	 	if ((screeny & 3) == 2)
		{
			const char * rt = tile_cache[(levely + 1) & 7];

			char si = 0;
			for(char i=0; i<16; i++)
			{
				switch (rt[i])
				{
				case 20:
					enemies_add(32 + 32 * i, 30, ET_GUN, 0, 0);
					break;
				case 26:
					enemies_add(32 + 32 * i, 56, ET_EVDOOR, 0, 0);
					break;
				case 28:
					si = i;
					break;
				case 29:
					enemies_add(64 + 32 * si, 30, ET_PINGPONG, 64 + 32 * si, 4 + 32 * i);
					break;
				case 45:
					enemies_add(28 + 32 * i, 22, ET_LEFTGUARD, 0, 0);
					break;
				case 46:
					enemies_add(36 + 32 * i, 22, ET_RIGHTGUARD, 0, 0);
					break;
				case 47:
					enemies_add(32 * i - 16, 28, ET_SPARKSPHERE, 0, 0);
					break;
				}
			}
		
			if (!(levely & 3))
				wave_start(level_wave[levely >> 2]);
		}
		wave_loop();

#if TIME_DEBUG
		vic.color_border = VCOL_BLUE;		
#endif
	}

#if TIME_DEBUG
	vic.color_border = VCOL_BLACK;
#endif
	vspr_sort();
#if TIME_DEBUG
	vic.color_border = VCOL_RED;
#endif

	music_play();
#if TIME_DEBUG
	vic.color_border = VCOL_LT_BLUE;
#endif

	score_update();

	rirq_wait();

#if TIME_DEBUG
	vic.color_border = VCOL_ORANGE;
#endif

	if (!(phase & 7))
	{
		screeni = 1 - screeni;
		vic.memptr = (vic.memptr & 0xee) | (screeni << 4);
		vspr_screen(screeni ? Screen1 : Screen0);
		pscreenx = screenx;
		screenx -= ndx;
	}

	vic.ctrl1 = VIC_CTRL1_DEN | (phase & 7);
	vic.ctrl2 = VIC_CTRL2_MCM | (px & 7);

	vspr_update();

	// sort raster IRQs
	rirq_sort();

#if TIME_DEBUG
	vic.color_border = VCOL_PURPLE;
#endif

	// Offset of parallax scrolled regions, half in x due
	// to multicolor mode
	char	ty = (phase >> 1) & 15;
	char	tx = (px >> 2) & 7;

	ty += 32 * (tx & 3);

	if (tx & 4)
	{
		// Copy the char data from the cached shifted chars
		for(char i=0; i<16; i++)
		{
			Charset[i +  0] = LevelScrollFont[0][0][ty];
			Charset[i + 16] = LevelScrollFont[0][1][ty];

			Charset[i + 32] = LevelScrollFont[1][0][ty];
			Charset[i + 48] = LevelScrollFont[1][1][ty];

			// Next line
			ty++;
		}
	}
	else
	{
		for(char i=0; i<16; i++)
		{
			Charset[i +  0] = LevelScrollFont[0][1][ty];
			Charset[i + 16] = LevelScrollFont[0][0][ty];

			Charset[i + 32] = LevelScrollFont[1][1][ty];
			Charset[i + 48] = LevelScrollFont[1][0][ty];

			// Next line
			ty++;
		}		
	}

	static const char yo[4] = {10, 23, 45, 38};
	static const char xmask[16] = {
#for(i, 16) 0x55 | (128 >> (i & 6)),
	};
	static const char yoffset[64] = {
#for(i, 64) (i & 7) + (i * 2 & 0x70),
	};

#if TIME_DEBUG
	vic.color_border = VCOL_YELLOW;
#endif
	for(char i=0; i<4; i++)
	{
		char x = (4 * i - px) & 15;
		char y = (yo[i] - phase) & 63;

		char p = yoffset[y] | (x & 8); //(y & 7) + (y * 2 & 0x70) + (x & 0x8);
		Charset[64 + starp[i]] = 0x55;
		starp[i] = p;
		Charset[64 + p] = xmask[x]; //0x55 | (128 >> (x & 6));
	}
#if TIME_DEBUG
	vic.color_border = VCOL_BLUE;
#endif


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

void tile_redraw(char sy, const char * tis)
{
	char y = ((sy - 1) & 7) * 4;

	char * scl0 = tilerows[y & 31];
	char * scl1 = scl0 + 64;
	char * scl2 = scl0 + 128;
	char * scl3 = scl0 + 192;

	const char * tp = LevelTiles;

	char x = 0;
	for(char i=0; i<16; i++)
	{
		char t = tis[i];

		scl0[x] = tp[t + NUM_TILES *  0];
		scl1[x] = tp[t + NUM_TILES *  4];
		scl2[x] = tp[t + NUM_TILES *  8];
		scl3[x] = tp[t + NUM_TILES * 12];
		x++;

		scl0[x] = tp[t + NUM_TILES *  1];
		scl1[x] = tp[t + NUM_TILES *  5];
		scl2[x] = tp[t + NUM_TILES *  9];
		scl3[x] = tp[t + NUM_TILES * 13];
		x++;

		scl0[x] = tp[t + NUM_TILES *  2];
		scl1[x] = tp[t + NUM_TILES *  6];
		scl2[x] = tp[t + NUM_TILES * 10];
		scl3[x] = tp[t + NUM_TILES * 14];
		x++;

		scl0[x] = tp[t + NUM_TILES *  3];
		scl1[x] = tp[t + NUM_TILES *  7];
		scl2[x] = tp[t + NUM_TILES * 11];
		scl3[x] = tp[t + NUM_TILES * 15];
		x++;
	}

	char scy = screeny;
	if ((phase & 7) == 7)
		scy++;

	char * screen0 = screeni ? Screen1 : Screen0;
	char * screen1 = screeni ? Screen0 : Screen1;

	char y1 = (y - scy) & 31;
	char n = 4;
	if (y1 > 28)
	{
		n = y1 & 3;
		y1 = 0;
	}

	if (y1 < 25)
	{
		char * sp = screen1 + 40 * y1;

		char * scl = tilerows[(y1 + scy) & 31] + screenx;
		for(char i=0; i<n; i++)
		{
			for(signed char x=39; x>=0; x--)
				sp[x] = scl[x];
			sp += 40;
			scl += 64;
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
			sp = screen0 + 40 * y0;
			scl = tilerows[(y0 + scy + 1) & 31] + pscreenx;

			for(char i=0; i<n; i++)
			{
				for(signed char x=39; x>=0; x--)
					sp[x] = scl[x];
				sp += 40;
				scl += 64;
			}
		}
	}
}

void tile_replace(char sx, char sy, char ti)
{
	const char * tp = LevelTiles + ti;

	char * screen0 = screeni ? Screen1 : Screen0;
	char * screen1 = screeni ? Screen0 : Screen1;

	char y = ((sy - 1) & 7) * 4;
	char x = sx * 4;

	char scy = screeny;
	if ((phase & 7) == 7)
		scy++;

	char * scl = tilerows[y & 31];

	char	tx = x;
	#pragma unroll(full)
	for(char i=0; i<4; i++)
	{
		scl[tx + 0] = tp[0 * NUM_TILES];
		scl[tx + 1] = tp[1 * NUM_TILES];
		scl[tx + 2] = tp[2 * NUM_TILES];
		scl[tx + 3] = tp[3 * NUM_TILES];
		
		tp += 4 * NUM_TILES;
		tx += 64;
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
				char ty = 0;
				for(char i=0; i<n; i++)
				{
					sp[ty++] = 0xfe;
					sp[ty++] = 0xfe;
					sp[ty++] = 0xfe;
					sp[ty++] = 0xfe;
					ty += 36;
				}
			}
		}
	}
}

void tile_collide(char x, char y)
{
	char sy = ((y + screeny) >> 2) + 1, sx = x >> 2;

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

	if (ti == 34)
	{
		score_inc(15);

		for(char i=0; i<16; i++)
		{
			ti = lp[i];
			switch (ti)
			{
			case 31:
				ti = 0;
				break;
			case 30:
				ti = 11;
				break;
			case 32:
				ti = 10;
				break;
			case 33:
				ti = 34;
				break;
			default:
				continue;
			}
			lp[i] = ti;
		}
		tile_redraw(sy, lp);
	}
	else
	{
		score_inc(3);

		lp[sx] = ti;
		tile_replace(sx, sy, ti);
	}
}

