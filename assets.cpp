#include "assets.h"

#pragma section( music, 0)
#pragma region( music, 0xa000, 0xc000, , , {music} )

#pragma region( zeropage, 0x80, 0xfc, , , {} )

#pragma data( music )

__export const char GameMusic[] = {
	#embed 0x2000 0x88 "AcidShmupV5-1.sid" 
};

#pragma data(data)
