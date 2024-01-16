#include "assets.h"
#include "display.h"
#include <oscar.h>
#include <c64/vic.h>
#include <c64/memmap.h>
#include <c64/cia.h>

#pragma section( music, 0)
#pragma region( music, 0xa000, 0xd000, , , {music} )

#pragma region( zeropage, 0x80, 0xfc, , , {} )

#pragma region( main, 0x0880, 0x9000, , , {code, data, heap} )

#pragma region( stack, 0x0200, 0x0400, , , {stack} )

#pragma stacksize(512)
#pragma heapsize(0)

#pragma data( assets )

const char LevelFont[] = {
	#embed ctm_chars lzo "tiles.ctm"
};

// Ship sprite image
char const SpriteImages[] = {
	#embed spd_sprites lzo "sprites.spd"
};

__export const char AssetMusic[] = {
	#embed 0x3000 0x88 lzo "AcidShmupVA.sid" 
};

#pragma data(data)



void assets_init(void)
{
	cia_init();

	mmap_trampoline();

	mmap_set(MMAP_RAM);

	// Expand charset
	oscar_expand_lzo(Sprites, SpriteImages);

	mmap_set(MMAP_NO_ROM);

	// Expand charset
	oscar_expand_lzo(Charset, LevelFont);

	oscar_expand_lzo((char *)0xa000, AssetMusic);
}
