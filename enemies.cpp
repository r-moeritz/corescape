#include <c64/vic.h>
#include <c64/sprites.h>
#include "enemies.h"
#include "player.h"
#include "status.h"

static const signed char sintab[] = {
0, 4, 9, 13, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 57, 60, 64, 67, 70, 72, 75, 77, 79, 81, 83, 85, 86, 87, 88, 89, 90, 90, 90, 90, 90, 89, 88, 87, 86, 85, 83, 81, 79, 77, 75, 72, 70, 67, 64, 60, 57, 54, 50, 46, 42, 38, 34, 30, 26, 22, 18, 13, 9, 4, 0, -4, -9, -13, -18, -22, -26, -30, -34, -38, -42, -46, -50, -54, -57, -60, -64, -67, -70, -72, -75, -77, -79, -81, -83, -85, -86, -87, -88, -89, -90, -90, -90, -90, -90, -89, -88, -87, -86, -85, -83, -81, -79, -77, -75, -72, -70, -67, -64, -60, -57, -54, -50, -46, -42, -38, -34, -30, -26, -22, -18, -13, -9, -4
};


char bulls, bulle, bulld;
char enemys, enemye;
EnemyWave	ewave;
char		ewave_cnt;


__striped struct Bullet		bullet[8];

__striped struct Enemy		enemies[8];

void enemies_init(void)
{
	bulls = bulle = bulld = 0;
	enemys = enemye = 0;
	ewave = WAVE_NONE;

	for(char i=0; i<16; i++)
		vspr_hide(i + 8);
}

void enemies_add(int x, int y, EnemyType type, int p0, int p1)
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
			return;
	}

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

	switch (e->type)
	{
	case ET_UFO:
		vspr_set(i + 8, x - vscreenx, y, 68, i + 2);
		break;
	case ET_GUN:
		vspr_set(i + 8, x - vscreenx, y, 71, VCOL_LT_GREY);
		e->cnt = 0;
		break;
	case ET_EVDOOR:
		vspr_set(i + 8, x - vscreenx, y, 108, VCOL_YELLOW);
		break;
	case ET_PINGPONG:
		vspr_set(i + 8, x - vscreenx, y, 112, VCOL_ORANGE);
		e->vx = 5;
		break;
	case ET_LEFTGUARD:
		vspr_set(i + 8, x - vscreenx, y, 73, VCOL_YELLOW);
		break;
	case ET_RIGHTGUARD:
		vspr_set(i + 8, x - vscreenx, y, 74, VCOL_YELLOW);
		break;
	case ET_SPARKSPHERE:
		e->phase = 0x40;
		vspr_set(i + 8, x - vscreenx, y, 116, VCOL_YELLOW);
		break;
	case ET_ALIEN_1:
	case ET_ALIEN_2:
	case ET_ALIEN_3:
	case ET_ALIEN_4:
		e->cnt = y;
		e->y = 28;
		vspr_set(i + 8, x - vscreenx, 28, 120 + e->type - ET_ALIEN_1, VCOL_GREEN);
		break;
	case ET_POPCORN:
		vspr_set(i + 8, x - vscreenx, 28, 124, VCOL_ORANGE);
		break;		
	case ET_BOMBER_LEFT:
		vspr_set(i + 8, x - vscreenx, y, 132, VCOL_PURPLE);
		break;		
	case ET_BOMBER_RIGHT:
		vspr_set(i + 8, x - vscreenx, y, 136, VCOL_PURPLE);
		break;		
	case ET_SHIP_1:
	case ET_SHIP_2:
	case ET_SHIP_3:
	case ET_SHIP_4:
		vspr_set(i + 8, x - vscreenx, y, 140 + e->type - ET_SHIP_1, VCOL_LT_GREY);
		break;
	case ET_RETRO:
		vspr_set(i + 8, x - vscreenx, y, 144, VCOL_YELLOW);
		e->cnt = 16 + (rand() & 15);
		break;
	case ET_CORVETTE:
		vspr_set(i + 8, x - vscreenx, y, 144, VCOL_YELLOW);
		break;
	case ET_STAR:
		vspr_set(i + 8, x - vscreenx, y, 96, VCOL_YELLOW);
		break;
	}
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
		enemies_add(StarPos[(wave - WAVE_STAR_1) & 3], 14 + 64 * (((wave - WAVE_STAR_1) >> 2) & 1) , ET_STAR, 0, 0);
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
		case ET_PINGPONG:
			if ((char)((e->x >> 1) + 16 - hx) < 32 && (char)(e->y + 12 - y) < 16)
				return i;
			break;
		case ET_SPARKSPHERE:
			if ((char)((e->x >> 1) + 8 - hx) < 16 && (char)(e->y + 14 - y) < 28)
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
			if ((char)((e->x >> 1) + 8 - hx) < 16 && (char)(e->y + 14 - y) < 28)
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
};

static const unsigned EnemyScore[NUM_ENEMY_TYPES] =
{
	[ET_UFO] = 10,
	[ET_GUN] = 5,
	[ET_LEFTGUARD] = 20,
	[ET_RIGHTGUARD] = 20,

	[ET_ALIEN_1] = 25,
	[ET_ALIEN_2] = 25,
	[ET_ALIEN_3] = 25,
	[ET_ALIEN_4] = 25,

	[ET_POPCORN] = 10,

	[ET_BOMBER_LEFT] = 25,
	[ET_BOMBER_RIGHT] = 25,

	[ET_SHIP_1] = 50,
	[ET_SHIP_2] = 55,
	[ET_SHIP_3] = 60,
	[ET_SHIP_4] = 65,

	[ET_RETRO] = 100,
	[ET_CORVETTE] = 33,
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
						if ((unsigned)(s->x - e->x + 7) < 20)
						{
							s->y = 0;
							vspr_hide(j + 1);
							score_inc(EnemyScore[e->type]);
							e->type = ET_EXPLOSION;
							e->phase = 0;
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
		vspr_set(j + 16, x - vscreenx, y, 69, VCOL_YELLOW);
	}
}

static const char BulletColors[] = 
	{VCOL_ORANGE, VCOL_YELLOW, VCOL_WHITE, VCOL_LT_BLUE, VCOL_WHITE, VCOL_YELLOW, VCOL_ORANGE, VCOL_RED};

static const char CorvetteColor[] = 
	{VCOL_PURPLE, VCOL_YELLOW, VCOL_PURPLE, VCOL_PURPLE, 
	 VCOL_PURPLE, VCOL_BLUE, VCOL_PURPLE, VCOL_PURPLE };

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

					vspr_image(i + 8, dir + 71);
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
				vspr_image(i + 8, 108 + (rand() & 3));
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
				vspr_image(i + 8, 116 + (rand() & 3));
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
				vspr_set(i + 8, e->x - vscreenx, e->y, 112 + (e->phase & 3), VCOL_ORANGE);
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
				e->type = ET_FREE;
				vspr_hide(i + 8);
			}
			else
			{
				vspr_image(i + 8, e->phase + 64 + 16);
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
		case ET_POPCORN:
			e->y += 2;
			if (e->y < 250)
			{
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, 124 + ((e->phase >> 2) & 7));
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
				vspr_image(i + 8, 132 + ((e->phase >> 1) & 3));
				e->phase++;
				int dx = shipx - e->x;
				if (dx >= -8 && dx <= 8)
					bullet_add(e->x + 8, e->y + 20, 4 * ((rand() & 1) * 2 - 1), 2);
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
				vspr_image(i + 8, 136 + ((e->phase >> 1) & 3));
				e->phase++;
				int dx = shipx - e->x;
				if (dx >= -8 && dx <= 8)
					bullet_add(e->x + 8, e->y + 20, 4 * ((rand() & 1) * 2 - 1), 2);
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
					if (e->cnt == 50)
					{
						bullet_add(e->x + 8, e->y + 20, 0, 4);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 20, -13, 3);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 20, 13, 3);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 20, -24, 2);
						bulld = 0;
						bullet_add(e->x + 8, e->y + 20, 24, 2);
					}
					if (e->cnt >= 50)
						e->x += 2 * (e->type - ET_SHIP_1) - 3;

					e->y = y;
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
					vspr_image(i + 8, 145 + ((e->phase >> 1) & 3));

				}
				else
				{
					e->type = ET_FREE;
					vspr_hide(i + 8);
				}
			} break;

		case ET_STAR:
			if (e->y < 250)
			{
				e->y++;
				e->phase++;
				vspr_move(i + 8, e->x - vscreenx, e->y);
				vspr_image(i + 8, 96 + ((e->phase >> 2) & 3));
			}
			else
			{
				e->type = ET_FREE;
				vspr_hide(i + 8);
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

