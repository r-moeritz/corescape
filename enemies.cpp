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
}

void enemies_add(int x, int y, EnemyType type)
{
	if ((char)(enemys + 8) != enemye)
	{
		char i = enemye & 7;
		auto	e = enemies + i;

		e->type = ET_GUN;
		e->phase = 16 * i;
		e->x = x;
		e->y = y;

		switch (e->type)
		{
		case ET_UFO:
			vspr_set(i + 8, x, y, 68, i + 2);
			break;
		case ET_GUN:
			vspr_set(i + 8, x, y, 71, i + 2);
			break;
		}

		enemye++;		
	}
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
					if (s->y < e->y + 20 && s->y >= e->y)
					{
						if (s->x < e->x + 20 && s->x >= e->x)
						{
							shot[j].y = 0;
							vspr_hide(j + 1);
							e->type = ET_EXPLOSION;
							e->phase = 0;
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
		vspr_set(j + 16, (bullet[j].x >> 4) - vscreenx, (bullet[j].y >> 4), 69, VCOL_YELLOW);
	}
}

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
			e->y = 100 + (sintab[(e->phase + 32) & 0x7f] >> 1);
			e->phase++;
			vspr_move(i + 8, e->x - vscreenx, e->y);
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
				if (dy > 16)
				{
					signed char dir = 0;

					if (2 * dx < - dy)
						dir = -1;
					else if (2 * dx > dy)
						dir = 1;

					vspr_image(i + 8, dir + 71);
					if (!(rand() & 63))
						bullet_add(e->x + 8 + 12 * dir, e->y + 20, 16 * dir, 32);
				}

				vspr_move(i + 8, e->x - vscreenx, e->y);
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
}

