/*
	This expects a 128x128 CT23 texture to be placed at vram location 0x2300
	The vram address is not divided by 64 by gifscript, you must do that before
	In other words, the actual vram address is 0x2300 * 64

	UV (and XY coords) are automatically shifted, there is no way to do half
	pixel offsets at this moment
*/

textured_tri {
	rgbaq 0x80,0,0,127;
	prim tri textured;

	// By default DECAL is used
	
	// This draws half of the texture as-is
	tex0 0x2300 2 ct24 7,7;
	uv 0,0;
	xyz2 100,100,0;
	uv 0,128;
	xyz2 100,228,0;
	uv 128,128;
	xyz2 228,228,0;

	// This draws the other half of the texture, modulated by vertex colours
	// with gouraud shading to make it a bit more interesting
	prim tri textured gouraud;
	tex0 0x2300 2 ct24 7,7 modulate;
	rgbaq 0x00,0x80,0x00,127;
	uv 0,0;
	xyz2 100,100,0;
	rgbaq 0x00,0x00,0x80,127;
	uv 128,0;
	xyz2 228,100,0;
	rgbaq 0x80,0x00,0x00,127;
	uv 128,128;
	xyz2 228,228,0;
}
