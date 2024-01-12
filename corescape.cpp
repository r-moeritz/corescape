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
#include "levelseq2.h"
#include "levelseq3.h"
#include "status.h"

int main(void)
{
	display_init();

	music_init(TUNE_MAIN_1);

	score_init();

	for(;;)
	{
		for(char level = 0; level < 3; level++)
		{
			switch (level)
			{
			case 0:
				level_init(LevelSequence1, LevelWaves1, sizeof(LevelSequence1));
				music_init(TUNE_MAIN_1);
				break;
			case 1:
				level_init(LevelSequence2, LevelWaves2, sizeof(LevelSequence2));
				music_init(TUNE_MAIN_2);
				break;
			case 2:
				level_init(LevelSequence3, LevelWaves3, sizeof(LevelSequence3));
				music_init(TUNE_MAIN_3);
				break;
			}

			music_volume(15);

			player_init();
			enemies_init();

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
					ships_dec();
					player_init();
				}
				else if (levely < 4)
				{
					break;
				}
			}

			for(int i=0; i<15; i++)
			{
				music_volume(15 - i);
				for(int j=0; j<10; j++)
				{
					vic_waitBottom();
					music_play();
					vic_waitLine(100);					
					music_play();
				}
			}

			music_volume(0);
		}
	}

	return 0;
}