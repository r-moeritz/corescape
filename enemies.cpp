#include <c64/vic.h>
#include <c64/sprites.h>
#include "enemies.h"
#include "player.h"


static const signed char sintab[] = {
0, 4, 9, 13, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 57, 60, 64, 67, 70, 72, 75, 77, 79, 81, 83, 85, 86, 87, 88, 89, 90, 90, 90, 90, 90, 89, 88, 87, 86, 85, 83, 81, 79, 77, 75, 72, 70, 67, 64, 60, 57, 54, 50, 46, 42, 38, 34, 30, 26, 22, 18, 13, 9, 4, 0, -4, -9, -13, -18, -22, -26, -30, -34, -38, -42, -46, -50, -54, -57, -60, -64, -67, -70, -72, -75, -77, -79, -81, -83, -85, -86, -87, -88, -89, -90, -90, -90, -90, -90, -89, -88, -87, -86, -85, -83, -81, -79, -77, -75, -72, -70, -67, -64, -60, -57, -54, -50, -46, -42, -38, -34, -30, -26, -22, -18, -13, -9, -4
};


char bulls, bulle, bulld;
char enemys, enemye;

__striped struct Bullet		bullet[8];

__striped struct Enemy		enemies[8];

void enemies_init(void)
{
	bulls = bulle = bulld = 0;
	enemys = enemye = 0;

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
	e->x = x;
	e->y = y;
	e->cy = y;
	e->vy = 0;
	e->vx = 0;
	e->p0 = p0;
	e->p1 = p1;

	switch (e->type)
	{
	case ET_UFO:
		vspr_set(i + 8, x, y, 68, i + 2);
		break;
	case ET_GUN:
		vspr_set(i + 8, x, y, 71, i + 2);
		e->cnt = 0;
		break;
	case ET_EVDOOR:
		vspr_set(i + 8, x, y, 108, VCOL_YELLOW);
		break;
	case ET_PINGPONG:
		vspr_set(i + 8, x, y, 112, VCOL_ORANGE);
		e->vx = 5;
		break;
	}
}

bool enemies_collide(char hx, char y)
{
	for(char ei=enemys; ei!=enemye; ei++)
	{
		char	i = ei & 7;
		auto	e = enemies + i;

		switch (e->type)
		{
		case ET_EVDOOR:
			if ((e->phase & 63) >= 48 && (char)((e->x >> 1) + 4 - hx) < 8 && (char)(e->y + 32 - y) < 32)
				return true;
			break;
		case ET_PINGPONG:
			if ((char)((e->x >> 1) + 16 - hx) < 32 && (char)(e->y + 12 - y) < 16)
				return true;
			break;
		}
	}

	return false;
}

void enemies_check(void)
{
	for(char ei=enemys; ei!=enemye; ei++)
	{
		char	i = ei & 7;
		auto	e = enemies + i;

		if (e->type == ET_UFO || e->type == ET_GUN)
		{
			for(char j=0; j<4; j++)
			{
				auto s = shot + j;

				if (s->y != 0)
				{
					if ((char)(s->y - e->y) < 20)
					{
						if ((unsigned)(s->x - e->x) < 20)
						{
							s->y = 0;
							vspr_hide(j + 1);
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

void bullet_add(int x, int y, int dx, int dy)
{
	if ((char)(bulls + 8) != bulle)
	{
		char j = bulle & 7;
		char k = phase & 7;
		bulle++;

		bullet[j].x = x << 4;
		bullet[j].y = y << 4;
		bullet[j].dx = dx;
		bullet[j].dy = dy;
		vspr_set(j + 16, x - vscreenx, y, 69, VCOL_YELLOW);
	}
}

static const char BulletColors[] = 
	{VCOL_ORANGE, VCOL_YELLOW, VCOL_WHITE, VCOL_LT_BLUE, VCOL_WHITE, VCOL_YELLOW, VCOL_ORANGE, VCOL_RED};

void enemies_move(void)
{
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
			e->x = 256 + sintab[(e->phase) & 0x7f];
			e->y = (e->y * 15 + 100 + (sintab[(e->phase + 32) & 0x7f] >> 1)) >> 4;
			e->phase++;
			vspr_move(i + 8, e->x - vscreenx, e->y);
			if (!(rand() & 63))
				bullet_add(e->x + 8, e->y + 20, 0, 32);
			break;
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
						bullet_add(e->x + 8 + 12 * dir, e->y + 20, 16 * dir, 32);
				}

				vspr_move(i + 8, e->x - vscreenx, e->y);
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
		}		
	}

	for(char i=bulls; i!=bulle; i++)
	{
		char j = i & 7;
		if (bullet[j].y != 0)
		{
			bullet[j].x += bullet[j].dx;
			bullet[j].y += bullet[j].dy;
			bullet[j].hx = bullet[j].x >> 5;
			bullet[j].hy = bullet[j].y >> 4;
			if (bullet[j].y >= 16 * 240)
			{
				bullet[j].y = 0;
				vspr_hide(j + 16);
			}
			else
			{
				vspr_move(j + 16, (bullet[j].x >> 4) - vscreenx, (bullet[j].y >> 4));
				vspr_color(j + 16, BulletColors[(phase + i) & 7]);
			}
		}
	}	

	while (bulls != bulle && bullet[bulls & 7].y == 0)
		bulls++;
}

