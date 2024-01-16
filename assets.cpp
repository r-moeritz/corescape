#include "assets.h"

#pragma section( music, 0)
#pragma region( music, 0xa000, 0xd000, , , {music} )

#pragma region( zeropage, 0x80, 0xfc, , , {} )

#pragma region( main, 0x0880, 0xa000, , , {code, data, bss, heap} )

#pragma region( stack, 0x0200, 0x0400, , , {stack} )

#pragma stacksize(512)
#pragma heapsize(0)

#pragma data( music )

__export const char GameMusic[] = {
	#embed 0x3000 0x88 "AcidShmupVA.sid" 
};

#pragma data(data)
