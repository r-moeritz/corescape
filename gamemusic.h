#ifndef GAMEMUSIC_H
#define GAMEMUSIC_H

enum Tune
{
	TUNE_MAIN_1,
};

extern bool music_voice3;
extern bool	music_off;

void music_init(Tune tune);

void music_queue(Tune tune);

void music_play(void);

void music_silence(void);

void music_toggle(void);

void music_patch_voice3(bool enable);

#pragma compile("gamemusic.c")

#endif
