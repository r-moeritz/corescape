#pragma once


extern char score[6];

extern char num_ships;

void score_init(void);

inline void score_inc(unsigned amount);

void score_update(void);

void ships_inc(void);

void ships_dec(void);

void text_sprimage(char sp, char ch);

#pragma compile("status.cpp")
