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
#include "levelseq4.h"
#include "status.h"
#include "intro.h"

static char LevelText[] = S"STAGE 1";
static char GameOverText[] = S"GAMEOVER";

int main(void)
{
	assets_init();

	display_init();

	for(char i=0; i<6; i++)
		highscore[i] = 0;

	bool	trainer_mode = false;

	for(;;)
	{
		music_init(TUNE_HIGHSCORE);
		music_volume(15);
		intro_play();			

		score_init();

		for(char level = 0; level < 4; level++)
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
			case 3:
				level_init(LevelSequence4, LevelWaves4, sizeof(LevelSequence4));
				music_init(TUNE_MAIN_4);
				break;
			}

			background_init(level);

			music_volume(15);

			player_init();
			enemies_init();

			display_fade_in();

			LevelText[6] = S'1' + level;
			for(char i=0; i<7; i++)
			{
				if (LevelText[i] != ' ')
				{
					text_sprimage(SPIMAGE_TEXT_1 + i, LevelText[i]);
					enemies_add(350 + 20 * i, 100, ET_LEVEL, i, 0);
				}
			}

			for(;;)
			{
				keyb_poll();
				if (keyb_key == (KSCAN_STOP | KSCAN_QUAL_DOWN))
					break;
				else if (keyb_key == (KSCAN_T | KSCAN_QUAL_DOWN))
					trainer_mode = !trainer_mode;

				player_move();

				enemies_move();
				enemies_check();

				display_loop();

				if (playerState == PLST_DESTROYED)
				{					
					if (!trainer_mode)
					{
						if (num_ships == 0)
							break;
						ships_dec();
					}

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
					if (playerState != PLST_DESTROYED && shipy > 20)
					{
						shipy -= i;
						vspr_move(0, shipx - vscreenx, shipy);
					}

					score_update();
					enemies_move();
					vspr_sort();

					rirq_wait();
					vspr_update();
					rirq_sort();

					music_play();
					vic_waitBelow(100);					
					music_play();

				}
			}

			music_volume(0);

			if (playerState == PLST_DESTROYED && num_ships == 0)
			{
				enemies_init();

				unsigned x = 350;
				for(char i=0; i<8; i++)
				{
					text_sprimage(SPIMAGE_TEXT_1 + i, GameOverText[i]);
					enemies_add(x, 100, ET_LEVEL, i, 0);
					x += 20;
					if (i == 3)
						x += 20;
				}

				music_init(TUNE_GAMEOVER);
				music_volume(15);


				for(int i=0; i<15; i++)
				{
//					music_volume(15 - i);
					for(int j=0; j<20; j++)
					{
						score_update();
						enemies_move();
						vspr_sort();

						rirq_wait();
						vspr_update();
						rirq_sort();

						music_play();
						vic_waitBelow(100);					
						music_play();
					}
				}
				display_fade_out();

				break;			
			}

			display_fade_out();

		}

		score_check();
	}

	return 0;
}