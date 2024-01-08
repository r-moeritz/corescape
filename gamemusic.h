#ifndef GAMEMUSIC_H
#define GAMEMUSIC_H

enum Tune
{
	TUNE_MAIN_1,
	TUNE_MAIN_2,
	TUNE_MAIN_3,
	TUNE_MAIN_4
};

// 1 | 3:52
// 2 | 2:17
//		0:23
//		0:39
//		0:54
//		1:10
//		1:25
//		1:40
//		1:44
// 3 | 3:24
// 4 | 3:13

extern bool	music_off;

void music_init(Tune tune);

void music_play(void);

void music_silence(void);

void music_toggle(void);

#pragma compile("gamemusic.c")

#endif
