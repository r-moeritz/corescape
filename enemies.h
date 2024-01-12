#pragma once

#define TIME_DEBUG	0

extern char bulls, bulle, bulld;

extern __striped struct Bullet
{
	int 	x, dx;
	char	hx, hy, y, dy;
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

	ET_RETRO,
	ET_CORVETTE,

	ET_STAR,

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
	WAVE_RETRO_1,
	WAVE_RETRO_2,
	WAVE_RETRO_3,
	WAVE_RETRO_4,
	WAVE_CORVETTE_1,
	WAVE_CORVETTE_2,
	WAVE_CORVETTE_3,
	WAVE_CORVETTE_4,

	WAVE_STAR_1,
	WAVE_STAR_2,
	WAVE_STAR_3,
	WAVE_STAR_4,
	WAVE_STAR_5,
	WAVE_STAR_6,
	WAVE_STAR_7,
	WAVE_STAR_8,
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

char enemies_collide(char hx, char y);

void enemies_add(int x, int y, EnemyType type, int p0, int p1);

void wave_start(EnemyWave wave);

void wave_loop(void);

#pragma compile("enemies.cpp")
