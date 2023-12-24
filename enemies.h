#pragma once

extern char bulls, bulle, bulld;

extern __striped struct Bullet
{
	int 	x, y, dx, dy;
	char	hx, hy;
}	bullet[8];

enum EnemyType
{
	ET_FREE,
	ET_EXPLOSION,
	ET_UFO,
	ET_GUN,
	ET_EVDOOR,
	ET_PINGPONG
};

extern __striped struct Enemy
{
	EnemyType	type;
	int 		x, p0, p1;
	char		y, cy;
	signed char	vx,vy;
	char		phase, cnt;
}	enemies[8];

void enemies_init(void);

void enemies_move(void);

void enemies_check(void);

bool enemies_collide(char hx, char y);

void enemies_add(int x, int y, EnemyType type, int p0, int p1);

#pragma compile("enemies.cpp")
