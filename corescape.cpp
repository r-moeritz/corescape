#include <c64/vic.h>
#include <c64/memmap.h>
#include <c64/joystick.h>
#include <c64/sprites.h>
#include <c64/rasterirq.h>
#include <oscar.h>
#include <string.h>
#include <c64/cia.h>
#include "gamemusic.h"

#pragma section( music, 0)
#pragma region( music, 0xa000, 0xc000, , , {music} )

#pragma data( music )

__export const char GameMusic[] = {
	#embed 0x2000 0x88 "AcidShmupV5-1.sid" 
};

#pragma data(data)

// Custom screen address
char * const Screen0 = (char *)0xc000;
char * const Screen1 = (char *)0xc400;

// Custom spriteset address
char * const Sprites = (char *)0xd000;

// Custom charset address
char * const Charset = (char *)0xc800;

// Color mem address
char * const Color = (char *)0xd800;

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

void copy_screen_rows5(char * sp, char sx, char sy)
{
	sy &= 31;

	const char * mp0 = tilerows[sy + 0] + sx;
	const char * mp1 = tilerows[sy + 1] + sx;
	const char * mp2 = tilerows[sy + 2] + sx;
	const char * mp3 = tilerows[sy + 3] + sx;
	const char * mp4 = tilerows[sy + 4] + sx;

	for(signed char x=39; x>=0; x--)
	{
		sp[x +   0] = mp0[x];
		sp[x +  40] = mp1[x];
		sp[x +  80] = mp2[x];
		sp[x + 120] = mp3[x];
		sp[x + 160] = mp4[x];
	}	
}

// Expand a row of tiles into tile buffer
void expand_tiles(char sy, char ry, char dy)
{
	// Get source tile row
	const char * sp = LevelMap + 16 * (sy & 15);

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


char screenx, screeny, levely;
char screeni;

void rebuild_screen(char phase)
{
	char * screen = screeni ? Screen0 : Screen1;

	switch (phase & 7)
	{
	case 7:
		if (!(screeny & 3))
			levely--;

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

int shipx, shipy;
char shots, shotd;
char bulls, bulle, bulld;

__striped struct Shot
{
	int 	x;
	char 	y;
}	shot[4];

__striped struct Bullet
{
	int 	x, y, dx, dy;
}	bullet[8];

static const signed char sintab[] = {
0, 4, 9, 13, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 57, 60, 64, 67, 70, 72, 75, 77, 79, 81, 83, 85, 86, 87, 88, 89, 90, 90, 90, 90, 90, 89, 88, 87, 86, 85, 83, 81, 79, 77, 75, 72, 70, 67, 64, 60, 57, 54, 50, 46, 42, 38, 34, 30, 26, 22, 18, 13, 9, 4, 0, -4, -9, -13, -18, -22, -26, -30, -34, -38, -42, -46, -50, -54, -57, -60, -64, -67, -70, -72, -75, -77, -79, -81, -83, -85, -86, -87, -88, -89, -90, -90, -90, -90, -90, -89, -88, -87, -86, -85, -83, -81, -79, -77, -75, -72, -70, -67, -64, -60, -57, -54, -50, -46, -42, -38, -34, -30, -26, -22, -18, -13, -9, -4
};

int main(void)
{
	cia_init();

	mmap_trampoline();

	mmap_set(MMAP_RAM);

	// Expand charset
	oscar_expand_lzo(Sprites, SpriteImages);

	mmap_set(MMAP_NO_ROM);

	// Expand charset
	oscar_expand_lzo(Charset, LevelFont);

	// Clear screen, prepare color area for multicolor
	memset(Color, VCOL_WHITE + 8, 1000);
	memset(Screen0, 0, 1000);
	memset(Screen1, 0, 1000);

	// Background and border colors
	vic.color_border = VCOL_LT_BLUE;
	vic.color_back = VCOL_LT_BLUE;
	vic.color_back1 = VCOL_BLACK;
	vic.color_back2 = VCOL_LT_GREY;

	// Set VIC to show custom screen with custom charset and multicolor
	vic_setmode(VICM_TEXT_MC, Screen0, Charset);

	// Init the raster IRQ system to use the kernal iterrupt vector
	rirq_init_io();

	music_init(TUNE_MAIN_1);

	shipx = 160;
	shipy = 200;

	vic.spr_mcolor0 = VCOL_BLACK;
	vic.spr_mcolor1 = VCOL_WHITE;
	vic.spr_multi = 0xff;

	vspr_init(Screen0);
	vspr_set(0, shipx, shipy, 65, VCOL_ORANGE);

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

	levely = 0;
	for(char i=0; i<8; i++)
	{
		levely--;
		expand_tiles(levely, 0, ((levely - 1) & 7) * 4);
		expand_tiles(levely, 1, ((levely - 1) & 7) * 4);
		expand_tiles(levely, 2, ((levely - 1) & 7) * 4);
		expand_tiles(levely, 3, ((levely - 1) & 7) * 4);
	}

	for(char i=0; i<8; i++)
		vspr_set(i + 8, 10, 10, 68, 4 + i);
	
	vspr_sort();
	vspr_update();
	rirq_sort();

	rirq_start();

	char phase = 0;
	char px = 4;
	signed char ndx = 0;
	signed char dx = 0;
	int vscreenx = 0;
	for(;;)
	{
		music_play();

		vic.color_border = VCOL_WHITE;
		rebuild_screen(phase);
		vic.color_border = VCOL_LT_BLUE;

		phase++;		

		vscreenx -= dx;
		if (dx < 0)
			px--;
		else if (dx > 0)
			px++;

		joy_poll(0);
		shipx += 2 * joyx[0];
		shipy += 2 * joyy[0];

		if (shipx < 48)
			shipx = 48;
		else if (shipx > 500)
			shipx = 500;

		char	scx0 = (shipx - 24) >> 3;
		char	scx1 = (shipx - 20) >> 3;

		char	scy = (shipy + (7 - (phase & 7)) - 46) >> 3;

		char * scl0 = tilerows[(screeny + scy + 0) & 31];
		char * scl1 = tilerows[(screeny + scy + 1) & 31];
		char * scl2 = tilerows[(screeny + scy + 2) & 31];

		if (LevelAttr[scl2[scx0 + 1]] >= 0x90 || LevelAttr[scl2[scx1 + 1]] >= 0x90)
			shipy -= 2;

		if (LevelAttr[scl0[scx0 + 1]] >= 0x90 || LevelAttr[scl0[scx1 + 1]] >= 0x90)
		{
			shipy = scy * 8 + (phase & 7) + 46;
			scl0 = scl1;
			scl1 = scl2;
		}

		if (LevelAttr[scl0[scx0]] >= 0x90 || LevelAttr[scl1[scx1]] >= 0x90)
		{
			shipx += 2;
		}

		if (LevelAttr[scl0[scx1 + 2]] >= 0x90 || LevelAttr[scl1[scx1 + 2]] >= 0x90)
		{
			shipx -= 2;
		}

		vspr_move(0, shipx - vscreenx, shipy);
		vspr_image(0, 65 + joyx[0]);

		for(char i=0; i<4; i++)
		{
			if (shot[i].y != 0)
			{
				char	sy = (char)(shot[i].y - 46) >> 3;
				char	sx = (char)((char)(shot[i].x >> 1) - 10) >> 2;

				char * scl0 = tilerows[(screeny + sy + 0) & 31];
				if (LevelAttr[scl0[sx]] >= 0xa0)
					shot[i].y = 0;
				else
					shot[i].y -= 4;

				if (shot[i].y < 20)
				{
					shot[i].y = 0;
					vspr_hide(i + 1);
				}
				else
					vspr_move(i + 1, shot[i].x - vscreenx, shot[i].y);
			}
		}

		if (shotd > 0)
			shotd--;
		else if (shot[shots].y == 0 && joyb[0])
		{
			shot[shots].y = shipy - 16;
			shot[shots].x = shipx;

			vspr_set(shots + 1, shot[shots].x - vscreenx, shot[shots].y, 67, VCOL_YELLOW);
			shots = (shots + 1) & 3;
			shotd = 8;
		}

		for(char i=bulls; i!=bulle; i++)
		{
			char j = i & 7;
			if (bullet[j].y != 0)
			{
				bullet[j].x += bullet[j].dx;
				bullet[j].y += bullet[j].dy;
				if (bullet[j].y >= 16 * 240)
				{
					bullet[j].y = 0;
					vspr_hide(j + 16);
				}
				else
					vspr_move(j + 16, (bullet[j].x >> 4) - vscreenx, (bullet[j].y >> 4));
			}
		}

		while (bulls != bulle && bullet[bulls & 7].y == 0)
			bulls++;

		if (bulld > 0)
			bulld--;
		else if ((char)(bulls + 8) != bulle)
		{
			char j = bulle & 7;
			char k = phase & 7;
			bulle++;

			int x = 264 + sintab[(k * 16 + phase) & 0x7f];
			int y = 120 + (sintab[(k * 16 + phase + 32) & 0x7f] >> 1);

			bullet[j].x = x << 4;
			bullet[j].y = y << 4;
			if (shipx + 8 < x)
				bullet[j].dx = -16;
			else if (shipx > x + 8)
				bullet[j].dx = 16;
			else
				bullet[j].dx = 0;

			bullet[j].dy = 32;
			vspr_set(j + 16, (bullet[j].x >> 4) - vscreenx, (bullet[j].y >> 4), 69, VCOL_YELLOW);
			bulld = 13;
		}


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

		for(char i=0; i<8; i++)
			vspr_move(i + 8, 256 + sintab[(i * 16 + phase) & 0x7f] - vscreenx, 100 + (sintab[(i * 16 + phase + 32) & 0x7f] >> 1));

		vic.color_border = VCOL_BLACK;
		vspr_sort();
		vic.color_border = VCOL_LT_BLUE;

		music_play();

		rirq_wait();

		if (!(phase & 7))
		{
//			vscreenx = screenx * 8;
			screeni = 1 - screeni;
			vic.memptr = (vic.memptr & 0xee) | (screeni << 4);
			vspr_screen(screeni ? Screen1 : Screen0);
			screenx -= ndx;
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
	}


}