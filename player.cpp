#include <c64/joystick.h>
#include <c64/sprites.h>
#include "player.h"
#include "display.h"
#include "status.h"
#include "enemies.h"

int shipx, shipy;
char shots, shotd;

__striped struct Shot	shot[4];

void player_init(void)
{
	shipx = 272;
	shipy = 200;

	playerState = PLST_ENTERING;
	playerStateCount = 100;

	for(char i=0; i<4; i++)
	{
		shot[i].y = 0;
		vspr_hide(i + 1);
	}

	vspr_set(0, shipx, shipy, 65, VCOL_ORANGE);
}

static inline bool char_blocks_player(char c)
{
	return c >= 0x90 && c < 0xc0;
}

void player_move(void)
{
	if (playerState == PLST_DESTROYED)
		return;

	joy_poll(0);

	char	ssy = 39 + (phase & 7);
	signed char sdx = joyx[0];

	if (playerState == PLST_EXPLODING)
	{
		playerStateCount--;
		vspr_image(0, SPIMAGE_EXPLOSION + 15 - (playerStateCount >> 2));
		if (playerStateCount == 0)
		{
			playerState = PLST_DESTROYED;
			vspr_hide(0);
		}
	}
	else
	{
		shipx += 2 * sdx;
		if (joyy[0] < 0)
			shipy -= 3;
		else if (joyy[0] > 0)
			shipy += 3;

		if (shipx < 48)
		{
			shipx = 48;
			sdx = 0;
		}
		else if (shipx > 500)
		{
			shipx = 500;
			sdx = 0;
		}

		if (playerState == PLST_ACTIVE)
		{
			char	scx0 = (shipx - 24) >> 3;
			char	scx1 = (shipx - 24) >> 3;

			char	scy = (shipy - ssy) >> 3;

			char * scl0 = tilerows[(screeny + scy + 0) & 31];
			char * scl1 = tilerows[(screeny + scy + 1) & 31];
			char * scl2 = tilerows[(screeny + scy + 2) & 31];

			if (char_blocks_player(LevelAttr[scl0[scx0 + 1]]) || char_blocks_player(LevelAttr[scl0[scx1 + 1]]))
			{
				shipy = scy * 8 + (phase & 7) + 46;
				if (shipy > 232)
				{
					playerState = PLST_EXPLODING;
					playerStateCount = 64;
					shipy = 228;
				}
				else if (shipy > 224)
				{
					if (!char_blocks_player(LevelAttr[scl0[scx0]]))
						shipx -= 4;
					else if (!char_blocks_player(LevelAttr[scl0[scx1 + 2]]))
						shipx += 4;					
				}

				scl0 = scl1;
				scl1 = scl2;
			}
			else if (shipy > 232)
				shipy = 232;
			else if (shipy < 60)
				shipy = 60;
			else if (char_blocks_player(LevelAttr[scl2[scx0 + 1]]) || char_blocks_player(LevelAttr[scl2[scx1 + 1]]))
				shipy -= 3;

			if (char_blocks_player(LevelAttr[scl0[scx0]]) || char_blocks_player(LevelAttr[scl1[scx1]]))
			{
				shipx += 2;
				sdx = 0;
			}

			if (char_blocks_player(LevelAttr[scl0[scx1 + 2]]) || char_blocks_player(LevelAttr[scl1[scx1 + 2]]))
			{
				shipx -= 2;
				sdx = 0;
			}
		}
		else if (shipy > 232)
			shipy = 232;

		vspr_move(0, shipx - vscreenx, shipy);	
		vspr_image(0, (SPIMAGE_PLAYER + 1) + joyx[0]);

		if (playerState == PLST_ENTERING)
		{
			playerStateCount--;		
			if (playerStateCount == 0)
				playerState = PLST_ACTIVE;
			else
			{
				if (!(playerStateCount & 1))
					vspr_hide(0);
			}
		}
	}

	if (playerState == PLST_ACTIVE)
	{
		char hx = (shipx >> 1) + 2;
		for(char i=bulls; i!=bulle; i++)
		{
			char j = i & 7;
			if (bullet[j].y != 0 && (char)(bullet[j].hx - hx) < 8 && (char)(bullet[j].y - shipy) < 16)
			{
				playerState = PLST_EXPLODING;
				playerStateCount = 64;
				bullet[j].y = 0;
				vspr_hide(j + 16);
			}
		}

		if (playerState == PLST_ACTIVE)
		{
			char ei = enemies_collide(hx, shipy);
			if (ei != 0xff)
			{
				switch (enemies[ei].type)
				{
				case ET_STAR:
					ships_inc();
					enemies[ei].type = ET_FREE;
					vspr_hide(ei + 8);
					break;
				case ET_COIN:
					score_inc(200);
					enemies[ei].type = ET_FREE;
					vspr_hide(ei + 8);
					break;
				default:
					playerState = PLST_EXPLODING;
					playerStateCount = 64;
					break;
				}
			}
		}
	}

	for(char i=0; i<4; i++)
	{
		char shy = shot[i].y;
		if (shy != 0)
		{
			if (shy >= ssy)
			{
				char	sy = (char)(shy - ssy) >> 3;
				char	sx = (char)((char)(shot[i].x >> 1) - 10) >> 2;

				char * scl0 = tilerows[(screeny + sy + 0) & 31];
				char la = LevelAttr[scl0[sx]];
				if (la >= 0xa0)
				{
					if (la == 0xb0 && sy > 1)
						tile_collide(sx, sy);

					shy = 0;
					vspr_hide(i + 1);
				}
			}

			if (shy != 0)
			{
				shy -= 4;

				if (shy < 40)
				{
					shy = 0;
					vspr_hide(i + 1);
				}
				else
					vspr_move(i + 1, shot[i].x - vscreenx, shot[i].y);
			}

			shot[i].y = shy;
			shot[i].x += shot[i].dx;
		}
	}

	if (playerState == PLST_ACTIVE || playerState == PLST_ENTERING)
	{
		if (shotd > 0)
			shotd--;
		else if (shot[shots].y == 0 && joyb[0])
		{
			shot[shots].y = shipy - 8;
			shot[shots].x = shipx + 1;
			shot[shots].dx = sdx;

			vspr_set(shots + 1, shot[shots].x - vscreenx, shot[shots].y, SPIMAGE_SHOT, VCOL_YELLOW);
			shots = (shots + 1) & 3;
			if (joyy[0] < 0)
				shotd = 16;
			else
				shotd = 8;
		}
	}
}
