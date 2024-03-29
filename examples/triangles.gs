macro shaded_triangle {
	rgbaq 125,0,125;
	xyz2 0,25,0;
	rgbaq 255,255,255;
	xyz2 25,12,0;
	rgbaq 255,0,0;
	xyz2 50,25,0;
}

test {
	prim triangle;
	macro shaded_triangle 200,100;
	macro shaded_triangle 100,100;

	prim triangle gouraud;
	macro shaded_triangle 100,200;
	macro shaded_triangle 200,200;
	macro shaded_triangle 150,150;
	macro shaded_triangle 150,225;

	prim linestrip gouraud;
	rgbaq 255,255,255;
	xyz2 0,0,0;
	rgbaq 0,255,0;
	xyz2 50,50,0;
	rgbaq 0,0,255;
	xyz2 150,0,0;
	rgbaq 255,0,0;
	xyz2 200,50,0;
	rgbaq 255,255,255;
	xyz2 250,0,0;
}

