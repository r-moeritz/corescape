#include <c64/vic.h>
#include <c64/memmap.h>
#include <c64/joystick.h>
#include <c64/sprites.h>
#include <c64/rasterirq.h>
#include <c64/keyboard.h>
#include <oscar.h>
#include <string.h>
#include <c64/cia.h>
#include "gamemusic.h"
#include "display.h"
#include "assets.h"
#include "player.h"
#include "enemies.h"
#include "levelseq0.h"
#include "levelseq1.h"


int main(void)
{
	display_init();

	music_init(TUNE_MAIN_2);

	for(;;)
	{
		level_init(LevelSequence0, LevelWaves0, sizeof(LevelSequence0));
		player_init();
		enemies_init();
		music_init(TUNE_MAIN_1);

//		while (playerState != PLST_DESTROYED)
		for(;;)
		{
			keyb_poll();
			if (keyb_key == (KSCAN_STOP | KSCAN_QUAL_DOWN))
				break;

			player_move();

			enemies_move();
			enemies_check();

			display_loop();

			if (playerState == PLST_DESTROYED)
			{
				player_init();
			}
			else if (levely < 4)
			{
				break;
			}
		}
	}
}