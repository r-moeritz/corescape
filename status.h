#pragma once


extern char score[6], highscore[6];

extern char num_ships;

extern const char StatusFont[];

void score_init(void);

inline void score_inc(unsigned amount);

void score_update(void);

bool score_check(void);

void ships_inc(void);

void ships_dec(void);

void text_sprimage(char sp, char ch);

#pragma compile("status.cpp")
