#include "gamemusic.h"
#include <c64/sid.h>

bool	music_off;

void music_init(Tune tune)
{
	__asm
	{
		lda		tune
		jsr		$9400
	}
}

void music_queue(Tune tune)
{
}

void music_play(void)
{
	if (!music_off)
	{
		__asm
		{	
			jsr		$9403
		}
	}
}

void music_silence(void)
{
	sid.voices[0].ctrl = 0;
	sid.voices[0].susrel = 0;
	sid.voices[1].ctrl = 0;
	sid.voices[1].susrel = 0;
	sid.voices[2].ctrl = 0;
	sid.voices[2].susrel = 0;
}

void music_toggle(void)
{
	if (music_off)
		music_off = false;
	else
	{
		music_off = true;
		music_silence();
	}
}

void music_volume(char volume)
{
	*(volatile char *)0x955d = volume;
}
