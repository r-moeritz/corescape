#include <c64/vic.h>
#include <c64/sprites.h>
#include "enemies.h"
#include "player.h"
#include "status.h"
#include "display.h"

const signed char sintab[] = {
0, 4, 9, 13, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 57, 60, 64, 67, 70, 72, 75, 77, 79, 81, 83, 85, 86, 87, 88, 89, 90, 90, 90, 90, 90, 89, 88, 87, 86, 85, 83, 81, 79, 77, 75, 72, 70, 67, 64, 60, 57, 54, 50, 46, 42, 38, 34, 30, 26, 22, 18, 13, 9, 4, 0, -4, -9, -13, -18, -22, -26, -30, -34, -38, -42, -46, -50, -54, -57, -60, -64, -67, -70, -72, -75, -77, -79, -81, -83, -85, -86, -87, -88, -89, -90, -90, -90, -90, -90, -89, -88, -87, -86, -85, -83, -81, -79, -77, -75, -72, -70, -67, -64, -60, -57, -54, -50, -46, -42, -38, -34, -30, -26, -22, -18, -13, -9, -4
};

char bulls, bulle, bulld;
char enemys, enemye;
EnemyWave	ewave;
char		ewave_cnt;
bool 		hardcore, intermission;

#pragma bss(xbss)
__striped struct Bullet		bullet[8];
__striped struct Enemy		enemies[8];
#pragma bss(bss)

void enemies_init(void)
{
	bulls = bulle = bulld = 0;
	enemys = enemye = 0;
	ewave = WAVE_NONE;

	for(char i=0; i<16; i++)
		vspr_hide(i + 8);
}

char enemies_alloc(void)
{
	char i;
	if ((char)(enemys + 8) != enemye)
	{
		i = enemye & 7;
		enemye++;
	}
	else
	{
		i = 0;
		while (i < 8 && enemies[i].type != ET_FREE)
			i++;
		if (i == 8)
			return 0xff;
	}

	return i;
}

static const signed char BossletX[] = {-48, -24, 0, 24, 48};
static const signed char BossletY[] = {-4, -2, 21, -2, -4};
static const char BossletI[] = {SPIMAGE_BOSS, SPIMAGE_BOSS + 1, SPIMAGE_BOSS + 5, SPIMAGE_BOSS + 3, SPIMAGE_BOSS + 4};
static const char BossletC[] = {VCOL_DARK_GREY, VCOL_RED, VCOL_PURPLE, VCOL_RED, VCOL_DARK_GREY};
static const EnemyType BossletT[] = {ET_BOSS_LASER, ET_BOSS_MISSILE, ET_BOSS_FRONT, ET_BOSS_MISSILE, ET_BOSS_LASER};

char enemies_add(int x, int y, EnemyType type, int p0, int p1)
{

	char i = enemies_alloc();
	if (i == 0xff)
		return 0xff;

	auto	e = enemies + i;

	e->type = type;
	e->phase = 16 * i;
	e->cnt = 0;
	e->x = x;
	e->cx = x;
	e->y = y;
	e->cy = y;
	e->vy = 0;
	e->vx = 0;
	e->p0 = p0;
	e->p1 = p1;
	e->hits = hardcore ? 2 : 1;
	e->ext = 0xff;
	e->flash = 0;

	switch (e->type)
	{
	case ET_UFO:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_UFO, i + 2);
		break;
	case ET_GUN:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_GUN, VCOL_LT_GREY);
		e->cnt = 0;
		break;
	case ET_EVDOOR:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_EVDOOR, VCOL_YELLOW);
		break;
	case ET_PINGPONG:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_PINGPONG, VCOL_ORANGE);
		if (hardcore)
			e->vx = 7;
		else
			e->vx = 5;
		break;
	case ET_LEFTGUARD:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_LEFTGUARD, VCOL_YELLOW);
		break;
	case ET_RIGHTGUARD:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_RIGHTGUARD, VCOL_YELLOW);
		break;
	case ET_SPARKSPHERE:
		e->phase = 0x40;
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_SPHERE, VCOL_YELLOW);
		break;
	case ET_ALIEN_1:
	case ET_ALIEN_2:
	case ET_ALIEN_3:
	case ET_ALIEN_4:
		e->cnt = y;
		e->y = 28;
		vspr_set(i + 8, x - vscreenx, 28, SPIMAGE_ALIEN + e->type - ET_ALIEN_1, VCOL_GREEN);
		break;
	case ET_POPCORN:
		vspr_set(i + 8, x - vscreenx, 28, SPIMAGE_POPCORN, hardcore ? VCOL_BLUE : VCOL_ORANGE);
		break;		
	case ET_BOMBER_LEFT:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_BOMBER_LEFT, VCOL_PURPLE);
		break;		
	case ET_BOMBER_RIGHT:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_BOMBER_RIGHT, VCOL_PURPLE);
		break;		
	case ET_SHIP_1:
	case ET_SHIP_2:
	case ET_SHIP_3:
	case ET_SHIP_4:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_SHIP_1 + 4 * (e->type - ET_SHIP_1), VCOL_LT_GREY);
		break;
	case ET_RETRO:
		vspr_set(i + 8, x - vscreenx, y,  SPIMAGE_RETRO, VCOL_YELLOW);
		e->cnt = 8 + (rand() & 15);
		if (!hardcore)
			e->cnt += 12;
		break;
	case ET_CORVETTE:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_CORVETTE, VCOL_YELLOW);
		break;
	case ET_STAR:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_STAR, VCOL_YELLOW);
		break;
	case ET_COIN:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_COIN, VCOL_YELLOW);
		break;
	case ET_LASER:
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_LASER, VCOL_YELLOW);
		break;		
	case ET_MISSILE:
		e->vy = - 8;
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_MISSILE, VCOL_YELLOW);		
		break;		
	case ET_PEACHES_1:
	case ET_PEACHES_2:
	case ET_PEACHES_3:
	case ET_PEACHES_4:
	case ET_PEACHES_5:
		e->phase += 5 * i;
		e->vx = 3;
		vspr_set(i + 8, x - vscreenx, y, SPIMAGE_PEACHES, VCOL_PURPLE);		
		break;		
	case ET_DESTROYER:
		{
			char j = enemies_alloc();
			if (j == 0xff)
			{
				e->type = ET_FREE;
				return 0xff;
			}

			vspr_set(i + 8, x - vscreenx     , y, SPIMAGE_DESTROYER + 0, VCOL_YELLOW);
			vspr_set(j + 8, x - vscreenx + 24, y, SPIMAGE_DESTROYER + 1, VCOL_YELLOW);

			e->phase += j;
			e->hits = hardcore ? 16 : 8;
			e->ext = j;
			enemies[j].type = ET_EXT;

		}
		break;
	case ET_FRIGATE:
		{
			char j = enemies_alloc();
			if (j == 0xff)
			{
				e->type = ET_FREE;
				return 0xff;
			}

			vspr_set(i + 8, x - vscreenx     , y, SPIMAGE_FRIGATE + 0, VCOL_YELLOW);
			vspr_set(j + 8, x - vscreenx + 24, y, SPIMAGE_FRIGATE + 1, VCOL_YELLOW);

			e->phase += j;
			e->hits = hardcore ? 16 : 8;
			e->ext = j;
			enemies[j].type = ET_EXT;

		}
		break;

	case ET_TRANSPORTER:
		{
			char j = enemies_alloc();
			if (j == 0xff)
			{
				e->type = ET_FREE;
				return 0xff;
			}

			vspr_set(i + 8, x - vscreenx     , y, SPIMAGE_TRANSPORTER + 0, VCOL_CYAN);
			vspr_set(j + 8, x - vscreenx + 24, y, SPIMAGE_TRANSPORTER + 1, VCOL_CYAN);

			e->phase += j;
			e->hits = 16;
			e->ext = j;
			enemies[j].type = ET_EXT;
		}
		break;

	case ET_BOSS:
		{
			char	bl[5];

			for(char j=0; j<5; j++)
			{
				char t = enemies_alloc();
				bl[j] = t;
				auto	b = enemies + t;
				b->type = BossletT[j];
				b->vx = BossletX[j];
				b->vy = BossletY[j];
				b->x = x + b->vx;
				b->y = y + b->vy;
				b->cnt = 0;
				b->ext = i;
				b->hits = hardcore ? 16 : 8;
				vspr_set(t + 8, b->x - vscreenx, b->y, BossletI[j], BossletC[j]);
			}

			enemies[bl[1]].link = bl[0];
			enemies[bl[3]].link = bl[4];

			vspr_set(i + 8, x - vscreenx, y, SPIMAGE_BOSS + 2, VCOL_PURPLE);

			e->ext = bl[2];
			e->hits = hardcore ? 64 : 32;
			break;
		}
	case ET_LEVEL:
		e->x <<= 2;
		vspr_set(i + 8, x, y, SPIMAGE_TEXT_1 + p0, hardcore ? VCOL_CYAN : VCOL_YELLOW);
		e->vx = -47;
		break;
	}

	return i;
}

const int StarPos[] = {
	24 + 20,
	24 + 20 + 64,
	512 - 16 - 64,
	512 - 16
};

void wave_start(EnemyWave wave)
{
	switch(wave)
	{
	case WAVE_UFO_4:
		enemies_add(256, 14, ET_UFO, 0, 0);		
	case WAVE_UFO_3:
		enemies_add(128, 14, ET_UFO, 0, 0);				
	case WAVE_UFO_2:
		enemies_add(384, 14, ET_UFO, 0, 0);				
	case WAVE_UFO_1:
		enemies_add(256, 14, ET_UFO, 0, 0);				
		break;
	case WAVE_ALIEN_1:
	case WAVE_ALIEN_2:
	case WAVE_ALIEN_3:
	case WAVE_ALIEN_4:
		for(char i=0; i<8; i++)
			enemies_add(0,  (5 + 5 * (wave - WAVE_ALIEN_1)) * i, ET_ALIEN_1 + wave - WAVE_ALIEN_1, 0, 0);
		break;		
	case WAVE_POPCORN_1:
	case WAVE_POPCORN_2:
	case WAVE_POPCORN_3:
	case WAVE_POPCORN_4:
	case WAVE_POPCORN_5:
	case WAVE_BOMBER_L1:
	case WAVE_BOMBER_L2:
	case WAVE_BOMBER_L3:
	case WAVE_BOMBER_L4:
	case WAVE_BOMBER_L5:
	case WAVE_BOMBER_R1:
	case WAVE_BOMBER_R2:
	case WAVE_BOMBER_R3:
	case WAVE_BOMBER_R4:
	case WAVE_BOMBER_R5:
	case WAVE_SHIP_1:
	case WAVE_SHIP_2:
	case WAVE_SHIP_3:
	case WAVE_SHIP_4:
	case WAVE_RETRO_1:
	case WAVE_RETRO_2:
	case WAVE_RETRO_3:
	case WAVE_RETRO_4:
	case WAVE_CORVETTE_1:
	case WAVE_CORVETTE_2:
	case WAVE_CORVETTE_3:
	case WAVE_CORVETTE_4:
	case WAVE_PEACHES_1:
	case WAVE_PEACHES_2:
	case WAVE_PEACHES_3:
	case WAVE_PEACHES_4:
	case WAVE_PEACHES_5:
		ewave = wave;
		ewave_cnt = 0;
		break;
	case WAVE_STAR_1:
	case WAVE_STAR_2:
	case WAVE_STAR_3:
	case WAVE_STAR_4:
	case WAVE_STAR_5:
	case WAVE_STAR_6:
	case WAVE_STAR_7:
	case WAVE_STAR_8:
		enemies_add(StarPos[(wave - WAVE_STAR_1) & 3], 14 + 56 * (((wave - WAVE_STAR_1) >> 2) & 1) , ET_STAR, 0, 0);
		break;
	case WAVE_DESTROYER_1:
		enemies_add(128, 30, ET_DESTROYER, 0, 0);
		break;
	case WAVE_DESTROYER_2:
		enemies_add(384, 30, ET_DESTROYER, 0, 0);
		break;
	case WAVE_DESTROYER_3:
		enemies_add(128, 30, ET_DESTROYER, 0, 0);
		enemies_add(384, 30, ET_DESTROYER, 0, 0);
		break;
	case WAVE_DESTROYER_4:	
		enemies_add(128,  30, ET_DESTROYER, 0, 0);
		enemies_add(256, 30, ET_DESTROYER, 0, 0);
		enemies_add(384, 30, ET_DESTROYER, 0, 0);
		break;
	case WAVE_FRIGATE_1:
		enemies_add(128, 30, ET_FRIGATE, 0, 0);
		break;
	case WAVE_FRIGATE_2:
		enemies_add(384, 30, ET_FRIGATE, 0, 0);
		break;
	case WAVE_FRIGATE_3:
		enemies_add(128, 30, ET_FRIGATE, 0, 0);
		enemies_add(384, 30, ET_FRIGATE, 0, 0);
		break;
	case WAVE_FRIGATE_4:	
		enemies_add(128,  30, ET_FRIGATE, 0, 0);
		enemies_add(256, 30, ET_FRIGATE, 0, 0);
		enemies_add(384, 30, ET_FRIGATE, 0, 0);
		break;

	case WAVE_TRANSPORTER:
		enemies_add(0, 50, ET_TRANSPORTER, 0, 0);
		break;

	case WAVE_BOSS:
		enemies_add(256, 10, ET_BOSS, 0, 0);
		break;

	case WAVE_COINS:
		if (enemys == enemye)
		{
			for(char i=0; i<8; i++)
				enemies_add(128 + 32 * i, 20 + (rand() & 31), ET_COIN, 0, 0);
		}
		break;
	}
}

void wave_loop(void)
{
	switch (ewave)
	{
	case WAVE_POPCORN_1:
	case WAVE_POPCORN_2:
	case WAVE_POPCORN_3:
	case WAVE_POPCORN_4:
	case WAVE_POPCORN_5:
		if (!(ewave_cnt & 1))
			enemies_add(136 + 64 * (ewave - WAVE_POPCORN_1) + (rand() & 7), 20, ET_POPCORN, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 13)
			ewave = WAVE_NONE;
		break;
	case WAVE_BOMBER_L1:
	case WAVE_BOMBER_L2:
	case WAVE_BOMBER_L3:
	case WAVE_BOMBER_L4:
	case WAVE_BOMBER_L5:
		if (!(ewave_cnt & 1))
			enemies_add(0, 20 + (rand() & 7), ET_BOMBER_LEFT, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == (2 * (ewave - WAVE_BOMBER_L1) + 3))
			ewave = WAVE_NONE;
		break;
	case WAVE_BOMBER_R1:
	case WAVE_BOMBER_R2:
	case WAVE_BOMBER_R3:
	case WAVE_BOMBER_R4:
	case WAVE_BOMBER_R5:
		if (!(ewave_cnt & 1))
			enemies_add(511, 20 + (rand() & 7), ET_BOMBER_RIGHT, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == (2 * (ewave - WAVE_BOMBER_R1) + 3))
			ewave = WAVE_NONE;
		break;
	case WAVE_SHIP_1:
	case WAVE_SHIP_2:
	case WAVE_SHIP_3:
	case WAVE_SHIP_4:
		if (!(ewave_cnt & 7))
			enemies_add(235 - 32 * (2 * (ewave - WAVE_SHIP_1) - 3) + 3 * ewave_cnt + (rand() & 7), 20, ewave - WAVE_SHIP_1 + ET_SHIP_1, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 25)
			ewave = WAVE_NONE;
		break;
	case WAVE_RETRO_1:
	case WAVE_RETRO_2:
	case WAVE_RETRO_3:
	case WAVE_RETRO_4:
		if (!(ewave_cnt & 7))
		{
			for(char i=0; i< ewave - WAVE_RETRO_1 + 1; i++)
				enemies_add(64 + (rand() & 127) + (rand() & 127) + (rand() & 127), 244, ET_RETRO, 0, 0);
		}
		ewave_cnt++;
		if (ewave_cnt == 9)
			ewave = WAVE_NONE;
		break;
	case WAVE_CORVETTE_1:
	case WAVE_CORVETTE_2:
	case WAVE_CORVETTE_3:
	case WAVE_CORVETTE_4:
		if (!(ewave_cnt & 3))
		{
			enemies_add(512 * (rand() & 1), 130, ET_CORVETTE, 0, 0);
		}
		ewave_cnt++;
		if (ewave_cnt == (8 * (ewave - WAVE_CORVETTE_1) + 1))
			ewave = WAVE_NONE;
		break;

	case WAVE_PEACHES_1:
		if (!(ewave_cnt & 1))
			enemies_add(128 + (rand() & 7), 20, ET_PEACHES_1, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 13)
			ewave = WAVE_NONE;
		break;

	case WAVE_PEACHES_2:
		if (!(ewave_cnt & 1))
			enemies_add(384 + (rand() & 7), 20, ET_PEACHES_2, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 13)
			ewave = WAVE_NONE;
		break;

	case WAVE_PEACHES_3:
		if (!(ewave_cnt & 1))
			enemies_add(64, 20, ET_PEACHES_3, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 13)
			ewave = WAVE_NONE;
		break;

	case WAVE_PEACHES_4:
		if (!(ewave_cnt & 1))
			enemies_add(0, 220, ET_PEACHES_4, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 13)
			ewave = WAVE_NONE;
		break;

	case WAVE_PEACHES_5:
		if (!(ewave_cnt & 1))
			enemies_add(64 + 32 * ewave_cnt, 30, ET_PEACHES_5, 0, 0);
		ewave_cnt++;
		if (ewave_cnt == 13)
			ewave = WAVE_NONE;
		break;
	}
}

char enemies_collide(char hx, char y)
{
	for(char ei=enemys; ei!=enemye; ei++)
	{
		char	i = ei & 7;
		auto	e = enemies + i;

		switch (e->type)
		{
		case ET_EVDOOR:
			if ((e->phase & 63) >= 48 && (char)((e->x >> 1) + 4 - hx) < 8 && (char)(e->y + 32 - y) < 32)
				return i;
			break;
		case ET_LASER:
		case ET_MISSILE:
			if ((char)((e->x >> 1) + 4 - hx) < 8 && (char)(e->y + 32 - y) < 32)
				return i;
			break;
		case ET_PINGPONG:
			if ((char)((e->x >> 1) + 16 - hx) < 32 && (char)(e->y + 12 - y) < 16)
				return i;
			break;
		case ET_SPARKSPHERE:
			if ((char)((e->x >> 1) + 8 - hx) < 16 && (char)(e->y + 14 - y) < 28)
				return i;
			break;
		case ET_UFO:
			if (hardcore && (char)((e->x >> 1) + 8 - hx) < 16 && (char)(e->y + 14 - y) < 28)
				return i;
			break;			
		case ET_ALIEN_1:
		case ET_ALIEN_2:
		case ET_ALIEN_3:
		case ET_ALIEN_4:
		case ET_POPCORN:
		case ET_SHIP_1:
		case ET_SHIP_2:
		case ET_SHIP_3:
		case ET_SHIP_4:
		case ET_RETRO:
		case ET_CORVETTE:
		case ET_STAR:
		case ET_COIN:
		case ET_BOSS:
		case ET_BOSS_2:
		case ET_BOSS_3:
		case ET_BOSS_MISSILE:
		case ET_BOSS_LASER:
		case ET_BOSS_SUPPORT:
		case ET_PEACHES_1:
		case ET_PEACHES_2:
		case ET_PEACHES_3:
		case ET_PEACHES_4:
		case ET_PEACHES_5:
			if ((char)((e->x >> 1) + 8 - hx) < 16 && (char)(e->y + 14 - y) < 28)
				return i;
			break;
		case ET_DESTROYER:
		case ET_FRIGATE:
		case ET_TRANSPORTER:
			if ((char)((e->x >> 1) + 8 - hx) < 28 && (char)(e->y + 14 - y) < 28)
				return i;
			break;
		}
	}

	return 0xff;
}

#define ETF_SHOT	0x01

static const char EnemyFlags[NUM_ENEMY_TYPES] = 
{
	[ET_UFO] = ETF_SHOT,
	[ET_GUN] = ETF_SHOT,
	[ET_LEFTGUARD] = ETF_SHOT,
	[ET_RIGHTGUARD] = ETF_SHOT,

	[ET_ALIEN_1] = ETF_SHOT,
	[ET_ALIEN_2] = ETF_SHOT,
	[ET_ALIEN_3] = ETF_SHOT,
	[ET_ALIEN_4] = ETF_SHOT,

	[ET_POPCORN] = ETF_SHOT,

	[ET_BOMBER_LEFT] = ETF_SHOT,
	[ET_BOMBER_RIGHT] = ETF_SHOT,

	[ET_SHIP_1] = ETF_SHOT,
	[ET_SHIP_2] = ETF_SHOT,
	[ET_SHIP_3] = ETF_SHOT,
	[ET_SHIP_4] = ETF_SHOT,

	[ET_RETRO] = ETF_SHOT,
	[ET_CORVETTE] = ETF_SHOT,

	[ET_DESTROYER] = ETF_SHOT,
	[ET_FRIGATE] = ETF_SHOT,
	[ET_TRANSPORTER] = ETF_SHOT,

	[ET_PEACHES_1] = ETF_SHOT,
	[ET_PEACHES_2] = ETF_SHOT,
	[ET_PEACHES_3] = ETF_SHOT,
	[ET_PEACHES_4] = ETF_SHOT,
	[ET_PEACHES_5] = ETF_SHOT,

	[ET_BOSS_MISSILE] = ETF_SHOT,
	[ET_BOSS_SUPPORT] = ETF_SHOT,
	[ET_BOSS_3] = ETF_SHOT,
};

static const char EnemyWidth[NUM_ENEMY_TYPES] = 
{
	[ET_UFO] = 20,
	[ET_GUN] = 20,
	[ET_LEFTGUARD] = 20,
	[ET_RIGHTGUARD] = 20,

	[ET_ALIEN_1] = 20,
	[ET_ALIEN_2] = 20,
	[ET_ALIEN_3] = 20,
	[ET_ALIEN_4] = 20,

	[ET_POPCORN] = 20,

	[ET_BOMBER_LEFT] = 20,
	[ET_BOMBER_RIGHT] = 20,

	[ET_SHIP_1] = 20,
	[ET_SHIP_2] = 20,
	[ET_SHIP_3] = 20,
	[ET_SHIP_4] = 20,

	[ET_PEACHES_1] = 20,
	[ET_PEACHES_2] = 20,
	[ET_PEACHES_3] = 20,
	[ET_PEACHES_4] = 20,
	[ET_PEACHES_5] = 20,

	[ET_RETRO] = 20,
	[ET_CORVETTE] = 20,

	[ET_DESTROYER] = 44,
	[ET_FRIGATE] = 44,
	[ET_TRANSPORTER] = 44,

	[ET_BOSS] = 20,
	[ET_BOSS_2] = 20,
	[ET_BOSS_3] = 20,
	[ET_BOSS_MISSILE] = 20,
	[ET_BOSS_LASER] = 20,
	[ET_BOSS_FRONT] = 20,
	[ET_BOSS_SUPPORT] = 20,
};

static const unsigned EnemyScore[NUM_ENEMY_TYPES] =
{
	[ET_UFO] = 100,
	[ET_GUN] = 20,
	[ET_LEFTGUARD] = 50,
	[ET_RIGHTGUARD] = 50,

	[ET_ALIEN_1] = 100,
	[ET_ALIEN_2] = 100,
	[ET_ALIEN_3] = 100,
	[ET_ALIEN_4] = 100,

	[ET_POPCORN] = 10,

	[ET_BOMBER_LEFT] = 50,
	[ET_BOMBER_RIGHT] = 50,

	[ET_SHIP_1] = 100,
	[ET_SHIP_2] = 150,
	[ET_SHIP_3] = 200,
	[ET_SHIP_4] = 300,

	[ET_PEACHES_1] = 500,
	[ET_PEACHES_2] = 500,
	[ET_PEACHES_3] = 500,
	[ET_PEACHES_4] = 500,
	[ET_PEACHES_5] = 500,

	[ET_RETRO] = 250,
	[ET_CORVETTE] = 500,
	[ET_DESTROYER] = 1000,
	[ET_FRIGATE] = 1500,
	[ET_TRANSPORTER] = 2500,

	[ET_BOSS_3] = 20000,
	[ET_BOSS_MISSILE] = 1000,
	[ET_BOSS_SUPPORT] = 5000,
};

void enemies_check(void)
{
	for(char ei=enemys; ei!=enemye; ei++)
	{
		char	i = ei & 7;
		auto	e = enemies + i;

		if (EnemyFlags[e->type] & ETF_SHOT)
		{
			for(char j=0; j<4; j++)
			{
				auto s = shot + j;

				if (s->y != 0)
				{
					if ((char)(s->y - e->y) < 20)
					{
						if ((unsigned)(s->x - e->x + 7) < EnemyWidth[e->type])
						{
							s->y = 0;
							vspr_hide(j + 1);
							e->flash = 4;
							e->hits--;							
							if (e->hits == 0)
							{
								score_inc(EnemyScore[e->type]);
								if (e->type == ET_BOSS_MISSILE)
								{
									enemies[e->link].type = ET_BOSS_SUPPORT;
									enemies[e->link].ext = 0xff;
									enemies[e->link].vx >>= 1;
									enemies[e->link].vy = -8;
									enemies[e->ext].type++;
								}
								else if (e->ext != 0xff)
								{
									enemies[e->ext].type = ET_EXPLOSION;
									enemies[e->ext].phase = 0;
								}
								e->type = ET_EXPLOSION;
								e->phase = 0;
							}
							else if (e->type == ET_TRANSPORTER)
							{
								enemies_add(e->x + 12, e->y + 12, ET_COIN, 0, 0);
							}
							break;
						}
					}
				}
			}
		}
	}

}

void bullet_add(int x, char y, int dx, char dy)
{
	if (bulld == 0 && (char)(bulls + 8) != bulle)
	{
		char j = bulle & 7;
		char k = phase & 7;
		bulle++;
		bulld = 8;

		bullet[j].x = x << 4;
		bullet[j].y = y;
		bullet[j].dx = dx;
		bullet[j].dy = dy;
		vspr_set(j + 16, x - vscreenx, y, SPIMAGE_BULLET, VCOL_YELLOW);
	}
}

static const char BulletColors[] = 
	{VCOL_ORANGE, VCOL_YELLOW, VCOL_WHITE, VCOL_LT_BLUE, VCOL_WHITE, VCOL_YELLOW, VCOL_ORANGE, VCOL_RED};

static const char CorvetteColor[] = 
	{VCOL_PURPLE, VCOL_YELLOW, VCOL_PURPLE, VCOL_PURPLE, 
	 VCOL_PURPLE, VCOL_BLUE, VCOL_PURPLE, VCOL_PURPLE };

static const char DestroyerColor[] = 
	{VCOL_DARK_GREY, VCOL_LT_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, 
	 VCOL_DARK_GREY, VCOL_RED, VCOL_DARK_GREY, VCOL_DARK_GREY };

static const char FrigateColor[] = 
	{VCOL_RED, VCOL_YELLOW, VCOL_RED, VCOL_RED, 
	 VCOL_RED, VCOL_PURPLE, VCOL_RED, VCOL_RED };

static const signed char sphere_dxy[16] = {3, 2, 1, 0, 0, -1, -2, -3, -3, -2, -1, 0, 0, 1, 2, 3};

static const char ufo_enter[32] = {
	#for(i,32) (31 - i) * (31 - i) / 10,
};

static const char bomber_enter[64] = {
	#for(i,64) (63 - i) * (63 - i) / 60,
};

static const char ufo_leave[64] = {
	#for(i,64) i * i / 16,
};

void enemies_move(void)
{

#if TIME_DEBUG
	vic.color_border = VCOL_LT_GREEN;
#endif

	if (bulld > 0)
		bulld--;

	for(char ei=enemys; ei!=enemye; ei++)
	{
		char	i = ei & 7;
		auto	e = enemies + i;

		switch (e->type)
		{
		case ET_FREE:
			if (ei==enemys)
				enemys++;
			break;
		case ET_UFO:
			{
				int y = 100 + (sintab[(e->phase + 32) & 0x7f] >> 1);
				if (e->cnt < 32)
					y -= ufo_enter[e->cnt];
				else if (e->cnt > 192)
					y += ufo_leave[e->cnt - 192];

				e->cnt++;
				if (y < 250 && e->cnt != 0)
				{
					if (y > 0)
					{
						e->x = 256 + sintab[(e->phase) & 0x7f];
						e->y = y;
						vspr_move(i + 8, e->x - vscreenx, e->y);
						if (!(rand() & 63))
							bullet_add(e->x + 8, e->y + 20, 0, 2);
					}
					e->phase++;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			} break;

		case ET_GUN:
			e->phase++;
			e->y++;

			if (e->y > 250)
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else
			{
				int dx = shipx - e->x;
				int dy = shipy - e->y;
				if (dy > 8)
				{
					signed char dir = 0;

					if (2 * dx + dy < 0)
						dir = -1;
					else if (2 * dx - dy > 0)
						dir = 1;

					vspr_image(i + 8, dir + (SPIMAGE_GUN + 1));
					if (!(rand() & 63))
						bullet_add(e->x + 8 + 12 * dir, e->y + 20, 16 * dir, 2);
				}

				vspr_move(i + 8, e->x - vscreenx, e->y);
			}
			break;			
		case ET_LEFTGUARD:
			e->y++;

			if (e->y > 250)
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				if (e->y < 200 && (e->y & 15) == 8)
					bullet_add(e->x + 10, e->y + 8, 32, 3);
			}
			break;
		case ET_RIGHTGUARD:
			e->y++;

			if (e->y > 250)
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				if (e->y < 200 && (e->y & 15) == 0)
					bullet_add(e->x + 8, e->y + 8, -32, 3);
			}
			break;

		case ET_EVDOOR:
			e->phase++;
			e->y++;

			if (e->y > 250)
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else if ((e->phase & 63) < 48)
			{
				vspr_hide(i + 8);
			}
			else
			{
				vspr_image(i + 8, SPIMAGE_EVDOOR + (rand() & 3));
				vspr_move(i + 8, e->x - vscreenx, e->y + (rand() & 7));	
			}
			break;

		case ET_SPARKSPHERE:
			e->y++;

			e->x += sphere_dxy[((e->phase >> 3) + 4) & 15];
			e->y += sphere_dxy[((e->phase >> 3)    ) & 15];

			e->phase++;

			if (e->y > 250)
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else
			{
				vspr_image(i + 8, SPIMAGE_SPHERE + (rand() & 3));
				vspr_move(i + 8, e->x - vscreenx, e->y);
			}
			break;

		case ET_PINGPONG:
			e->phase++;
			e->y++;
			if (e->y > 250)
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else
			{
				e->x += e->vx;
				vspr_set(i + 8, e->x - vscreenx, e->y, SPIMAGE_PINGPONG + (e->phase & 3), VCOL_ORANGE);
				if (e->vx > 0)
				{
					if (e->x >= e->p1)
						e->vx = -e->vx;
				}
				else
				{
					if (e->x < e->p0)
						e->vx = -e->vx;				
				}
			}
			break;

		case ET_EXPLOSION:
			if (e->phase == 16)
			{
				if (e->y < 160 && !(rand() & 7))
				{
					e->type = ET_COIN;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}
			else
			{
				vspr_color(i + 8, VCOL_YELLOW);
				vspr_image(i + 8, e->phase + SPIMAGE_EXPLOSION);
				e->phase++;
			}
			break;

		case ET_ALIEN_1:
		case ET_ALIEN_2:
		case ET_ALIEN_3:
		case ET_ALIEN_4:
			if (e->cnt)
				e->cnt--;
			else
			{
				e->y += 2;
				if (e->y < 250)
				{
					e->x = 256 + sintab[(e->phase) & 0x7f] + (sintab[((e->phase + 7) << 2) & 0x7f] >> 1);
					vspr_move(i + 8, e->x - vscreenx, e->y);
					if (!(rand() & 63))
						bullet_add(e->x + 8, e->y + 20, 16 * ((rand() & 1) * 2 - 1), 2);
					e->phase++;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}
			break;

		case ET_PEACHES_1:
			{
				switch (e->cnt)
				{
				case 0:
				case 2:
					e->y += 2;
					if (e->y >= 220)
						e->cnt++;
					break;
				case 1:
					e->y -= 2;
					e->x += 3;
					if (e->y < 50)
						e->cnt++;
					break;
				case 3:
					e->y += 2;
					break;
				}

				if (e->y < 250)
				{
					vspr_move(i + 8, e->x - vscreenx, e->y);
					vspr_image(i + 8, SPIMAGE_PEACHES + ((e->phase >> 1) & 7));
					e->phase++;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}	break;
		case ET_PEACHES_2:
			{
				switch (e->cnt)
				{
				case 0:
				case 2:
					e->y += 2;
					if (e->y >= 220)
						e->cnt++;
					break;
				case 1:
					e->y -= 2;
					e->x -= 3;
					if (e->y < 50)
						e->cnt++;
					break;
				case 3:
					e->y += 2;
					break;
				}

				if (e->y < 250)
				{
					vspr_move(i + 8, e->x - vscreenx, e->y);
					vspr_image(i + 8, SPIMAGE_PEACHES + ((e->phase >> 1) & 7));
					e->phase++;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}	break;

		case ET_PEACHES_3:
			e->x += e->vx;
			if (!(e->phase & 1))
				e->y += 1;

			if (e->x > 448 || e->x < 64)
				e->vx = -e->vx;

			if (e->y < 250)
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_PEACHES + ((e->phase >> 1) & 7));
				e->phase++;
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			break;
		case ET_PEACHES_4:
			{
				switch (e->cnt)
				{
				case 0:
					e->y -= 1;
					e->x += 3;
					if (e->y < 50)
						e->cnt++;
					break;
				case 1:
					e->x -= 2;
					if (e->x < 64)
						e->cnt++;
					break;
				case 2:
					e->y += 1;
					e->x += 4;
					if (e->x >= 384)
						e->cnt++;
					break;
				case 3:
					e->y += 1;
					e->x -= 2;
					break;
				}

				if (e->y < 250)
				{
					vspr_move(i + 8, e->x - vscreenx, e->y);
					vspr_image(i + 8, SPIMAGE_PEACHES + ((e->phase >> 1) & 7));
					e->phase++;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}	break;

		case ET_PEACHES_5:
			e->y += 2;

			if (e->y < 250)
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_PEACHES + ((e->phase >> 1) & 7));
				e->phase++;
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			break;

		case ET_POPCORN:
			e->y += 2;
			if (e->y < 250)
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_POPCORN + ((e->phase >> 2) & 7));
				if (hardcore && !(rand() & 255))
					bullet_add(e->x + 8, e->y + 20, 4 * ((rand() & 1) * 2 - 1), 3);
				e->phase++;
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			break;
		case ET_BOMBER_LEFT:
			if (e->cnt < 255)
			{
				e->cnt++;
				e->y = 100 - bomber_enter[e->cnt >> 2];
			}
			e->x += 3;
			if (e->x < 512)
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_BOMBER_LEFT + ((e->phase >> 1) & 3));
				e->phase++;
				int dx = shipx - e->x;
				if (dx >= -8 && dx <= 8)
				{
					bullet_add(e->x + 8, e->y + 20, 4 * ((rand() & 1) * 2 - 1), 2);
					if (hardcore)
						bulld = 1;
				}
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}			
			break;
		case ET_BOMBER_RIGHT:
			if (e->cnt < 255)
			{
				e->cnt++;
				e->y = 100 - bomber_enter[e->cnt >> 2];
			}
			e->x -= 3;
			if (e->x >= 0)
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_BOMBER_RIGHT + ((e->phase >> 1) & 3));
				e->phase++;
				int dx = shipx - e->x;
				if (dx >= -8 && dx <= 8)
				{
					bullet_add(e->x + 8, e->y + 20, 4 * ((rand() & 1) * 2 - 1), 2);
					if (hardcore)
						bulld = 1;
				}
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}			
			break;
		case ET_SHIP_1:
		case ET_SHIP_2:
		case ET_SHIP_3:
		case ET_SHIP_4:
			{
				int y = 2 * e->cnt + (sintab[((e->phase + 31) << 1) & 0x7f] >> 3);
				if (y < 250)
				{
					if (e->cnt >= 50)
					{
						if (e->cnt == 50)
						{
							bullet_add(e->x + 8, e->y + 20, 0, 4);
							bulld = 0;
						}
						else if (e->cnt == 51)
						{
							bullet_add(e->x + 8, e->y + 20, -13, 3);
							bulld = 0;
							bullet_add(e->x + 8, e->y + 20, 13, 3);
							bulld = 0;
						}
						else if (e->cnt == 52)
						{
							bullet_add(e->x + 8, e->y + 20, -24, 2);
							bulld = 0;
							bullet_add(e->x + 8, e->y + 20, 24, 2);
						}

						e->x += 2 * (e->type - ET_SHIP_1) - 3;
					}

					e->y = y;
					vspr_image(i + 8, SPIMAGE_SHIP_1 + 4 * (e->type - ET_SHIP_1) + ((e->phase >> 2) & 3));
					vspr_move(i + 8, e->x - vscreenx, y);
					e->phase++;
					e->cnt++;
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}
			break;

		case ET_RETRO:
			{
				if (e->cnt > 0)
				{
					if (e->y > 222)
						e->y--;
					e->cnt--;
					e->x = e->cx + (rand() & 7);
					vspr_move(i + 8, e->x - vscreenx, e->y);
				}
				else if (e->y > 40)
				{
					e->y -= 3;
					vspr_move(i + 8, e->x - vscreenx, e->y);
				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			}
			break;

		case ET_CORVETTE:
			{
				if (e->y < 250)
				{
					int y = 100 + (sintab[(e->phase + 32) & 0x7f] >> 1);
					int x = shipx + (sintab[(e->phase) & 0x7f] >> 1);

					if (e->x > x)
					{
						if (e->vx > -24)
							e->vx--;
					}
					else if (e->x < x)
					{
						if (e->vx < 24)
							e->vx++;
					}

					if (e->cnt > 64 || e->y < y)
					{
						if (e->vy < 24)
							e->vy++;
					}
					else if (e->y > y)
					{
 						if (e->vy > -16)
							e->vy--;
					}

					e->x += e->vx >> 3;
					e->y += e->vy >> 3;

					e->phase++;

					if (!(e->phase & 3))
					{
						e->cnt++;
						if (e->cnt == 64)
						{
							bullet_add(e->x + 8, e->y + 20, -6, 3);
							bulld = 0;
							bullet_add(e->x + 8, e->y + 20, 6, 3);
						}
					}

					vspr_color(i + 8, CorvetteColor[e->phase & 7]);
					vspr_move(i + 8, e->x - vscreenx, e->y);
					vspr_image(i + 8, SPIMAGE_CORVETTE + ((e->phase >> 1) & 3));

				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			} break;

		case ET_DESTROYER:
			if (e->y < 250)
			{
				int y = 100 + (sintab[(e->phase + 32) & 0x7f] >> 1);					
				int x = shipx;

				switch ((e->cnt >> 3) & 3)
				{
				case 0:
				case 2:
					x = shipx - 12;
					break;
				case 1:
					x = 128;
					break;
				case 3:
					x = 384;
					break;
				}

				if (e->x > x)
				{
					if (e->vx > -24)
						e->vx--;
				}
				else if (e->x < x)
				{
					if (e->vx < 24)
						e->vx++;
				}

				if (e->cnt > 128 || e->y < y)
				{
					if (e->vy < 24)
						e->vy++;
				}
				else if (e->y > y)
				{
						if (e->vy > -16)
						e->vy--;
				}

				e->x += e->vx >> 3;
				e->y += e->vy >> 3;

				e->phase++;

				if (!(e->phase & 3))
				{
					e->cnt++;
					switch (e->cnt & 31)
					{
					case 15:
						enemies_add(e->x + 12, e->y + 10, ET_LASER, 0, 0);
						break;
					case 31:
						bullet_add(e->x + 8, e->y + 20, -4, 3);
						bulld = 0;
						bullet_add(e->x + 32, e->y + 20, 4, 3);
						bulld = 0;
						break;
					}
				}

				if (e->flash)
				{
					e->flash--;
					vspr_color(i + 8, VCOL_WHITE);
					vspr_color(e->ext + 8, VCOL_WHITE);
				}
				else
				{
					vspr_color(i + 8, DestroyerColor[e->phase & 7]);
					vspr_color(e->ext + 8, DestroyerColor[e->phase & 7]);
				}

				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_move(e->ext + 8, e->x - vscreenx + 24, e->y);

			}
			else
			{
				enemies[e->ext].type = ET_FREE;
				e->type = ET_FREE;
				vspr_hide(i + 8);
				vspr_hide(e->ext + 8);
			}
			break;

		case ET_FRIGATE:
			if (e->y < 250)
			{
				int y = 80 + (sintab[e->phase >> 1] >> 2);					
				int x = shipx;

				switch ((e->cnt >> 3) & 3)
				{
				case 0:
				case 2:
					x = shipx - 12;
					break;
				case 1:
					x = 128;
					break;
				case 3:
					x = 384;
					break;
				}

				if (e->x > x)
				{
					if (e->vx > -16)
						e->vx--;
				}
				else if (e->x < x)
				{
					if (e->vx < 16)
						e->vx++;
				}

				if (e->cnt > 128 || e->y < y)
				{
					if (e->vy < 16)
						e->vy++;
				}
				else if (e->y > y)
				{
						if (e->vy > -12)
						e->vy--;
				}

				e->x += e->vx >> 3;
				e->y += e->vy >> 3;

				e->phase++;

				if (!(e->phase & 3))
				{
					e->cnt++;
					switch (e->cnt & 31)
					{
					case 15:
					case 31:
						enemies_add(e->x + 12, e->y, ET_MISSILE, 0, 0);
						break;
					}
				}

				if (e->flash)
				{
					e->flash--;
					vspr_color(i + 8, VCOL_WHITE);
					vspr_color(e->ext + 8, VCOL_WHITE);
				}
				else
				{
					vspr_color(i + 8, FrigateColor[e->phase & 7]);
					vspr_color(e->ext + 8, FrigateColor[e->phase & 7]);
				}

				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_move(e->ext + 8, e->x - vscreenx + 24, e->y);

			}
			else
			{
				enemies[e->ext].type = ET_FREE;
				e->type = ET_FREE;
				vspr_hide(i + 8);
				vspr_hide(e->ext + 8);
			}
			break;

		case ET_TRANSPORTER:
			if (e->x < 512)
			{
				e->x += 2;
				if (!(e->phase & 3))
					e->y ++;
				e->phase++;

				if (e->flash)
				{
					e->flash--;
					vspr_color(i + 8, VCOL_WHITE);
					vspr_color(e->ext + 8, VCOL_WHITE);
				}
				else
				{
					vspr_color(i + 8, VCOL_CYAN);
					vspr_color(e->ext + 8, VCOL_CYAN);
				}

				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_move(e->ext + 8, e->x - vscreenx + 24, e->y);

			}
			else
			{
				enemies[e->ext].type = ET_FREE;
				e->type = ET_FREE;
				vspr_hide(i + 8);
				vspr_hide(e->ext + 8);
			}
			break;

		case ET_STAR:
			if (e->y < 250)
			{
				e->y++;
				e->phase++;
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_STAR + ((e->phase >> 2) & 3));
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}

			break;
		case ET_COIN:
			if (e->y < 250)
			{
				e->y += 2;
				e->phase++;
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, SPIMAGE_COIN + ((e->phase >> 2) & 3));
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}

			break;
		case ET_LASER:
			if (e->y < 246)
			{
				e->y += 8;
				vspr_move(i + 8, e->x - vscreenx, e->y);
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}

			break;
		case ET_MISSILE:
			if (e->y < 246)
			{
				if (e->x < shipx)
				{
					if (e->vx < 16)
						e->vx++;
				}
				else if (e->x > shipx)
				{
					if (e->vx > -16)
						e->vx--;
				}

				if (e->vy < 32)
					e->vy++;

				e->y += e->vy >> 3;
				e->x += e->vx >> 3;
				vspr_move(i + 8, e->x - vscreenx, e->y);
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}

			break;

		case ET_BOSS_MISSILE:
			if (e->flash)
			{
				e->flash--;
				vspr_color(i + 8, VCOL_WHITE);
			}
			else
				vspr_color(i + 8, VCOL_RED);

			if ((enemies[e->ext].phase & 7) == i && (enemies[e->ext].cnt & 31) == 15)
				enemies_add(e->x, e->y, ET_MISSILE, 0, 0);
			e->x = enemies[e->ext].x + e->vx;
			e->y = enemies[e->ext].y + e->vy;
			vspr_move(i + 8, e->x - vscreenx, e->y);
			break;

		case ET_BOSS_LASER:
			if (!(enemies[e->ext].phase & 7) && (enemies[e->ext].cnt & 31) == 31)
				enemies_add(e->x, e->y + 10, ET_LASER, 0, 0);
			e->x = enemies[e->ext].x + e->vx;
			e->y = enemies[e->ext].y + e->vy;
			vspr_move(i + 8, e->x - vscreenx, e->y);
			break;

		case ET_BOSS_FRONT:
			e->x = enemies[e->ext].x + e->vx;
			e->y = enemies[e->ext].y + e->vy;
			vspr_move(i + 8, e->x - vscreenx, e->y);
			break;

		case ET_BOSS_SUPPORT:
			if (e->y < 250)
			{
				int y = 100 + (sintab[(e->phase + 32) & 0x7f] >> 1);					
				int x = shipx;

				switch ((e->cnt >> 3) & 3)
				{
				case 0:
				case 2:
					x = shipx - 12;
					break;
				case 1:
					x = 128;
					break;
				case 3:
					x = 384;
					break;
				}

				if (e->x > x)
				{
					if (e->vx > -24)
						e->vx--;
				}
				else if (e->x < x)
				{
					if (e->vx < 24)
						e->vx++;
				}

				if (e->cnt > 128 || e->y < y)
				{
					if (e->vy < 24)
						e->vy++;
				}
				else if (e->y > y)
				{
						if (e->vy > -16)
						e->vy--;
				}

				e->x += e->vx >> 3;
				e->y += e->vy >> 3;

				e->phase++;

				if (!(e->phase & 3))
				{
					e->cnt++;
					if (!(e->cnt & 15))
						enemies_add(e->x + 12, e->y + 10, ET_LASER, 0, 0);
				}

				if (e->flash)
				{
					e->flash--;
					vspr_color(i + 8, VCOL_WHITE);
				}
				else
					vspr_color(i + 8, DestroyerColor[e->phase & 7]);

				vspr_move(i + 8, e->x - vscreenx, e->y);
			}
			else
			{
				enemies[e->ext].type = ET_FREE;
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			break;

		case ET_BOSS:
		case ET_BOSS_2:
		case ET_BOSS_3:
			if (e->y < 250)
			{
				int y = 80 + (sintab[e->cnt & 0x7f] >> 2);					
				int x = shipx;

				switch ((e->cnt >> 3) & 3)
				{
				case 0:
				case 2:
					x = shipx;
					break;
				case 1:
					x = 128;
					break;
				case 3:
					x = 384;
					break;
				}

				if (e->x > x)
				{
					if (e->vx > -16)
						e->vx--;
				}
				else if (e->x < x)
				{
					if (e->vx < 16)
						e->vx++;
				}

				if (e->y < y)
				{
					if (e->vy < 16)
						e->vy++;
				}
				else if (e->y > y)
				{
						if (e->vy > -12)
						e->vy--;
				}

				e->x += e->vx >> 3;
				e->y += e->vy >> 3;

				e->phase++;

				if (!(e->phase & 7))
				{
					e->cnt++;
					switch (e->cnt & 31)
					{
					case 6:
						bullet_add(e->x + 8, e->y + 40, -4, 3);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 40, 4, 3);
						bulld = 0;
						break;
					case 8:
						bullet_add(e->x + 8, e->y + 40, -8, 2);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 40, 8, 2);
						bulld = 0;
						break;
					case 10:
						bullet_add(e->x + 8, e->y + 40, -1, 4);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 40, 1, 4);
						bulld = 0;
						break;

					}
				}

				if (e->flash)
				{
					e->flash--;
					vspr_color(i + 8, VCOL_WHITE);
					vspr_color(e->ext + 8, VCOL_WHITE);
				}
				else
				{
					vspr_color(i + 8, FrigateColor[e->phase & 7]);
					vspr_color(e->ext + 8, FrigateColor[e->phase & 7]);
				}


				vspr_move(i + 8, e->x - vscreenx, e->y);

			}
			else
			{
				enemies[e->ext].type = ET_FREE;
				e->type = ET_FREE;
				vspr_hide(i + 8);
				vspr_hide(e->ext + 8);
			}
			break;
		case ET_LEVEL:
			e->cnt++;

			if (e->cnt < 64)
			{
				e->vx += 1;
				e->x += e->vx;
				vspr_move(i + 8, e->x >> 2, e->y);
			}
			else if (e->cnt < 140)
			{
				e->vx = 0;
			}
			else
			{
				e->vx -= 1;
				e->x += e->vx;
				if (e->x > 0)
					vspr_move(i + 8, e->x >> 2, e->y);
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);				
				}
			}
			break;

		}		
	}

#if TIME_DEBUG
	vic.color_border = VCOL_GREEN;
#endif


	for(char i=bulls; i!=bulle; i++)
	{
		char j = i & 7;
		if (bullet[j].y != 0)
		{
			bullet[j].x += bullet[j].dx;
			bullet[j].y += bullet[j].dy;
			bullet[j].hx = bullet[j].x >> 5;

			if (bullet[j].y >= 240)
			{
				bullet[j].y = 0;
				vspr_hide(j + 16);
			}
			else
			{
				vspr_move(j + 16, (bullet[j].x >> 4) - vscreenx, bullet[j].y);
				vspr_color(j + 16, BulletColors[(phase + i) & 7]);
			}
		}
	}	

	while (bulls != bulle && bullet[bulls & 7].y == 0)
		bulls++;

#if TIME_DEBUG
	vic.color_border = VCOL_BLUE;	
#endif
}

