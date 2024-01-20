#ifndef GAMEMUSIC_H
#define GAMEMUSIC_H

enum Tune
{
	TUNE_MAIN_1,
	TUNE_MAIN_2,
	TUNE_MAIN_3,
	TUNE_MAIN_4,

	TUNE_HIGHSCORE,
	TUNE_GAMEOVER,

	TUNE_INTERLUDE,
};


// 3:44
// 2:25
// 3:16
// 4:05

// 1 | 3:52
//		0:06
//		0:16
//		0:30
//		0:38
//		0:45
//		1:00
//		1:16
//		1:32
//		1:48
//		2:04
//		2:12
//		2:28
//		2:58
//		3:13
//		3:26
//		

// 2 | 2:17
//		0:23
//		0:39
//		0:54
//		1:10
//		1:25
//		1:40
//		1:44
// 3 | 3:24
//		0:14
//		0:29
//		0:44
//		1:11
//		1:27
//		1:41
//		1:55
//		2:09
//		2:25
//		2:37
//		2:53
//		

// 4 | 4:05
//		0:07
//		0:22
//		0:30
//		0:38
//		0:54
//		1:10
//		1:23
//		1:40
//		1:54
//		2:10
//		2:26
//		2:40
//		2:55
//		3:12
//		3:27
//		3:42
//		

extern bool	music_off;

void music_init(Tune tune);

void music_volume(char volume);

void music_play(void);

void music_silence(void);

void music_toggle(void);

#pragma compile("gamemusic.c")

#endif
