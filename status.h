#pragma once

// Score and highscore in one digit per byte
extern char score[6], highscore[6];

// Number of lifes/ships
extern char num_ships;

// The status font for the status sprites
extern const char StatusFont[];

// Init score system
void score_init(void);

// Increment the score by the given amount
inline void score_inc(unsigned amount);

// Update the score display
void score_update(void);

// Check if a new highscore was achieved
bool score_check(void);

// Increment the number of lifes/ships
void ships_inc(void);

// Decrement the number of lifes/ships
void ships_dec(void);

// Convert character data into a sprite
void text_sprimage(char sp, char ch);

#pragma compile("status.cpp")
