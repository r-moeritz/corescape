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


int main(void)
{
	display_init();

	music_init(TUNE_MAIN_1);

	for(;;)
	{
		level_init();
		player_init();
		enemies_init();

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
		}
	}
}