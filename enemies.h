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
	ET_PINGPONG,
	ET_LEFTGUARD,
	ET_RIGHTGUARD,
	ET_SPARKSPHERE,

	ET_ALIEN_1,
	ET_ALIEN_2,
	ET_ALIEN_3,
	ET_ALIEN_4,

	ET_POPCORN,
	ET_BOMBER_LEFT,
	ET_BOMBER_RIGHT,

	ET_SHIP_1,
	ET_SHIP_2,
	ET_SHIP_3,
	ET_SHIP_4,

	NUM_ENEMY_TYPES
};

enum EnemyWave
{
	WAVE_NONE,
	WAVE_UFO_1,
	WAVE_UFO_2,
	WAVE_UFO_3,
	WAVE_UFO_4,
	WAVE_ALIEN_1,
	WAVE_ALIEN_2,
	WAVE_ALIEN_3,
	WAVE_ALIEN_4,
	WAVE_POPCORN_1,
	WAVE_POPCORN_2,
	WAVE_POPCORN_3,
	WAVE_POPCORN_4,
	WAVE_POPCORN_5,
	WAVE_BOMBER_L1,
	WAVE_BOMBER_L2,
	WAVE_BOMBER_L3,
	WAVE_BOMBER_L4,
	WAVE_BOMBER_L5,
	WAVE_BOMBER_R1,
	WAVE_BOMBER_R2,
	WAVE_BOMBER_R3,
	WAVE_BOMBER_R4,
	WAVE_BOMBER_R5,
	WAVE_SHIP_1,
	WAVE_SHIP_2,
	WAVE_SHIP_3,
	WAVE_SHIP_4,
};

extern __striped struct Enemy
{
	EnemyType	type;
	int 		x, cx, p0, p1;
	char		y, cy;
	signed char	vx,vy;
	char		phase, cnt;
}	enemies[8];

void enemies_init(void);

void enemies_move(void);

void enemies_check(void);

bool enemies_collide(char hx, char y);

void enemies_add(int x, int y, EnemyType type, int p0, int p1);

void wave_start(EnemyWave wave);

void wave_loop(void);

#pragma compile("enemies.cpp")
