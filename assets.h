#pragma once

#pragma region( rbss, 0x0400, 0x0800, , , { bss } )

#pragma section( xbss, 0, , , bss )
#pragma region( xbss, 0x9000, 0xa000, , , { xbss } )

#pragma section( assets, 0)
#pragma region( assets, 0x9a00, 0xd000, , , {assets} )


void assets_init(void);


#pragma compile("assets.cpp")

