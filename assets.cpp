#include "assets.h"

#pragma section( music, 0)
#pragma region( music, 0x9800, 0xc000, , , {music} )

#pragma region( zeropage, 0x80, 0xfc, , , {} )

#pragma region( main, 0x0880, 0x9800, , , {code, data, bss, heap, stack} )

#pragma stacksize(1024)
#pragma heapsize(1024)

#pragma data( music )

__export const char GameMusic[] = {
	#embed 0x2800 0x88 "AcidShmupV7.sid" 
};

#pragma data(data)
