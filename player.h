#pragma once


extern int shipx, shipy;
extern char shots, shotd;

extern __striped struct Shot
{
	int 	x;
	char 	y;
}	shot[4];


void player_init(void);

void player_move(void);


#pragma compile("player.cpp")

