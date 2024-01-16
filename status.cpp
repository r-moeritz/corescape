#include "status.h"
#include "display.h"
#include <c64/sprites.h>

extern char score[6];
unsigned score_acc;

const char ScoreFont[] = {
	#embed ctm_chars "scorefont.ctm"
};

const char StatusFont[] = {
	#embed ctm_chars "statusfont.ctm"
};

char * const ScoreSprite = (char *)(0xc000 + 64 * SPIMAGE_SCORE);

char * const ScoreSpriteChars[] = {
	ScoreSprite + 0,
	ScoreSprite + 1,
	ScoreSprite + 2,
	ScoreSprite + 64,
	ScoreSprite + 65,
	ScoreSprite + 66
};

char * const ShipsSpriteChars[] = {
	ScoreSprite + 3 * 8 + 0,
	ScoreSprite + 3 * 8 + 1, 
	ScoreSprite + 3 * 8 + 2, 

	ScoreSprite + 3 *  8 + 0 + 64,
	ScoreSprite + 3 *  8 + 1 + 64, 
	ScoreSprite + 3 *  8 + 2 + 64, 

	ScoreSprite + 3 * 14 + 0,
	ScoreSprite + 3 * 14 + 1, 
	ScoreSprite + 3 * 14 + 2, 

	ScoreSprite + 3 * 14 + 0 + 64,
	ScoreSprite + 3 * 14 + 1 + 64, 
	ScoreSprite + 3 * 14 + 2 + 64, 
};

void score_draw(char i)
{
	char v = score[i];
	__assume(v < 10);

	char * dp = ScoreSpriteChars[i];
	const char * sp = ScoreFont + 8 * v;

	#pragma unroll(full)
	for(char i=0; i<8; i++)
		dp[3 * i] = sp[i];
}

void score_init(void)
{
	memset(ScoreSprite, 0, 128);

	for(char i=0; i<6; i++)
	{
		score[i] = 0;	
		score_draw(i);
	}

	score_acc = 0;
	num_ships = 0;

	ships_inc();
	ships_inc();
	ships_inc();

	vspr_set(6, 280, 220, SPIMAGE_SCORE + 0, VCOL_YELLOW);
	vspr_set(7, 304, 220, SPIMAGE_SCORE + 1, VCOL_YELLOW);
}

void score_inc(unsigned amount)
{
	score_acc += amount;
}

void ships_inc(void)
{
	if (num_ships < 12)
	{
		char * dp = ShipsSpriteChars[num_ships];
		const char * sp = ScoreFont + 8 * 10;

		#pragma unroll(full)
		for(char i=0; i<6; i++)
			dp[3 * i] = sp[i];

		num_ships++;
	}
}

void ships_dec(void)
{
	if (num_ships > 0)
	{
		num_ships--;

		char * dp = ShipsSpriteChars[num_ships];
		const char * sp = ScoreFont + 8 * 10;

		#pragma unroll(full)
		for(char i=0; i<6; i++)
			dp[3 * i] = 0;
	}
}

void score_update(void)
{
	if (score_acc)
	{
		char by = 1;
		char digit;

		if (score_acc >= 1000)
		{
			digit = 3;
			score_acc -= 1000;
		}
		else if (score_acc >= 100)
		{
			digit = 4;
			score_acc -= 100;
		}
		else if (score_acc >= 10)
		{
			digit = 5;
			score_acc -= 10;
		}
		else
		{
			digit = 6;
			by = score_acc;
			score_acc = 0;
		}

		while (by)
		{
			digit--;
			char d = score[digit] + by;
			by = 0;
			if (d >= 10)
			{
				d -= 10;
				by = 1;
			}
			score[digit] = d;
			score_draw(digit);
		}

		if (digit < 2)
			ships_inc();
	}
}

bool score_check(void)
{
	while (score_acc)
		score_update();

	char i = 0;
	while (i < 6 && score[i] == highscore[i])
		i++;
	if (i < 6 && score[i] > highscore[i])
	{
		do {
			highscore[i] = score[i];
			i++;
		} while (i < 6);

		return true;
	}
	else
		return false;
}

static const char nibble[] = {
	0x00, 0x03, 0x0c, 0x0f,
	0x30, 0x33, 0x3c, 0x3f,
	0xc0, 0xc3, 0xcc, 0xcf,
	0xf0, 0xf3, 0xfc, 0xff
};

void text_sprimage(char sp, char ch)
{
	char 	* 	spr = (char *)(0xc000 + 64 * sp);
	const char * cp = StatusFont + 8 * (ch & 0x3f);

	char c0, c1, c2;
	c0 = 0;
	c1 = 0;

	for(char i=0; i<10; i++)
	{
		c2 = i < 8 ? cp[i] : 0;

		char cm = c0 | c1 | c2;

		unsigned long xm = nibble[cm & 0x0f] | (nibble[cm >> 4] << 8);
		xm |= xm << 2;
		xm |= xm << 2;		
		unsigned long xc = nibble[c1 & 0x0f] | (nibble[c1 >> 4] << 8);
		xc <<= 2;

		unsigned long m = (xm & 0x555555l) ^ xc;

		spr[0] = (m >> 16) & 0xff;
		spr[1] = (m >>  8) & 0xff;
		spr[2] = (m      ) & 0xff;
		spr += 3;

		c0 = c1;
		c1 = c2;
	}
}
