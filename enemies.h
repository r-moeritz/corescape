#pragma once

extern char bulls, bulle, bulld;

extern __striped struct Bullet
{
	int 	x, y, dx, dy;
}	bullet[8];

enum EnemyType
{
	ET_FREE,
	ET_EXPLOSION,
	ET_UFO,
	ET_GUN
};

extern __striped struct Enemy
{
	EnemyType	type;
	int 		x;
	char		y;
	char		phase, cnt;
}	enemies[8];

void enemies_init(void);

void enemies_move(void);

void enemies_check(void);

void enemies_add(int x, int y, EnemyType type);

#pragma compile("enemies.cpp")
