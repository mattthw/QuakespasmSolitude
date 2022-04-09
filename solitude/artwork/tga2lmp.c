/*
TGA 2 LMP SOURCECODE

The MIT License (MIT)

Copyright (c) 2016-2019 Marco "eukara" Hladik <marco at icculus.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#define TGAHEADER 18
#define QPALSIZE 768
#define VERSION "1.1"

typedef unsigned char byte;

typedef union {
	int rgba:32; 
	struct {
		byte b:8;
		byte g:8;
		byte r:8;
		byte a:8;
	} u;
} pixel_t;

byte cust_pal[QPALSIZE]; /* custom 256 color palette */

/* fallback palette from Quake, put into the Public Domain by John Carmack */
pixel_t pal_fallb[256] = {
	0x000000,0x0f0f0f,0x1f1f1f,0x2f2f2f,0x3f3f3f,0x4b4b4b,0x5b5b5b,0x6b6b6b,
	0x7b7b7b,0x8b8b8b,0x9b9b9b,0xababab,0xbbb,0xcbcbcb,0xdbdbdb,0xebebeb,
	0x0f0b07,0x170f0b,0x1f170b,0x271b0f,0x2f2313,0x372b17,0x3f2f17,0x4b371b,
	0x533b1b,0x5b431f,0x634b1f,0x6b531f,0x73571f,0x7b5f23,0x836723,0x8f6f23,
	0x0b0b0f,0x13131b,0x1b1b27,0x272733,0x2f2f3f,0x37374b,0x3f3f57,0x474767,
	0x4f4f73,0x5b5b7f,0x63638b,0x6b6b97,0x7373a3,0x7b7baf,0x8383b,0x8b8bcb,
	0x000000,0x070700,0x0b0b00,0x131300,0x1b1b00,0x232300,0x2b2b07,0x2f2f07,
	0x373707,0x3f3f07,0x474707,0x4b4b0b,0x53530b,0x5b5b0b,0x63630b,0x6b6b0f,
	0x070000,0x0f0000,0x170000,0x1f0000,0x270000,0x2f0000,0x370000,0x3f0000,
	0x470000,0x4f0000,0x570000,0x5f0000,0x670000,0x6f0000,0x770000,0x7f0000,
	0x131300,0x1b1b00,0x232300,0x2f2b00,0x372f00,0x433700,0x4b3b07,0x574307,
	0x5f4707,0x6b4b0b,0x77530f,0x835713,0x8b5b13,0x975f1b,0xa3631f,0xaf6723,
	0x231307,0x2f170b,0x3b1f0f,0x4b2313,0x572b17,0x632f1f,0x733723,0x7f3b2b,
	0x8f4333,0x9f4f33,0xaf632f,0xbf772f,0xcf8f2b,0xdfab27,0xefcb1f,0xfff31b,
	0x0b0700,0x1b1300,0x2b230f,0x372b13,0x47331b,0x533723,0x633f2b,0x6f4733,
	0x7f533f,0x8b5f47,0x9b6b53,0xa77b5f,0xb7876b,0xc3937b,0xd3a38b,0xe3b397,
	0xab8ba3,0x9f7f97,0x937387,0x8b677b,0x7f5b6f,0x775363,0x6b4b57,0x5f3f4b,
	0x573743,0x4b2f37,0x43272f,0x371f23,0x2b171b,0x231313,0x170b0b,0x0f0707,
	0xb739f,0xaf6b8f,0xa35f83,0x975777,0x8b4f6b,0x7f4b5f,0x734353,0x6b3b4b,
	0x5f333f,0x532b37,0x47232b,0x3b1f23,0x2f171b,0x231313,0x170b0b,0x0f0707,
	0xdbc3b,0xcb3a7,0xbfa39b,0xaf978b,0xa3877b,0x977b6f,0x876f5f,0x7b6353,
	0x6b5747,0x5f4b3b,0x533f33,0x433327,0x372b1f,0x271f17,0x1b130f,0x0f0b07,
	0x6f837b,0x677b6f,0x5f7367,0x576b5f,0x4f6357,0x475b4f,0x3f5347,0x374b3f,
	0x2f4337,0x2b3b2f,0x233327,0x1f2b1f,0x172317,0x0f1b13,0x0b130b,0x070b07,
	0xfff31b,0xefdf17,0xdbcb13,0xcb70f,0xba70f,0xab970b,0x9b8307,0x8b7307,
	0x7b6307,0x6b5300,0x5b4700,0x4b3700,0x3b2b00,0x2b1f00,0x1b0f00,0x0b0700,
	0x0000ff,0x0b0bef,0x1313df,0x1b1bcf,0x2323bf,0x2b2baf,0x2f2f9f,0x2f2f8f,
	0x2f2f7f,0x2f2f6f,0x2f2f5f,0x2b2b4f,0x23233f,0x1b1b2f,0x13131f,0x0b0b0f,
	0x2b0000,0x3b0000,0x4b0700,0x5f0700,0x6f0f00,0x7f1707,0x931f07,0xa3270b,
	0xb7330f,0xc34b1b,0xcf632b,0xdb7f3b,0xe3974f,0xe7ab5f,0xefbf77,0xf7d38b,
	0xa77b3b,0xb79b37,0xc7c337,0xe7e357,0x7fbfff,0xabe7ff,0xd7ffff,0x670000,
	0x8b0000,0xb30000,0xd70000,0xff0000,0xfff393,0xfff7c7,0xffffff,0x9f5b53
};

byte c_last; /* last palette index we chose */
pixel_t last_px; /* last RGB value we chose */

/* quickly translate 24 bit RGB value to our palette */
byte pal24to8(byte r, byte g, byte b)
{
	pixel_t		px;
	byte		c_red, c_green, c_blue, c_best, l;
	int			dist, best;

	/* compare the last with the current pixel color for speed */
	if ((last_px.u.r == r) && (last_px.u.g == g) && (last_px.u.b == b)) {
		return c_last;
	}

	px.u.r = last_px.u.r = r;
	px.u.g = last_px.u.g = g;
	px.u.b = last_px.u.b = b;

	best = 255 + 255 + 255;
	c_last = c_best = c_red = c_green = c_blue = 255;
	l = 0;

	while (1) {
		if ((cust_pal[l * 3 + 0] == r) && (cust_pal[l * 3 + 0] == g) 
			&& (cust_pal[l * 3 + 0] == b))
		{
			last_px.u.r = cust_pal[l * 3 + 0];
			last_px.u.g = cust_pal[l * 3 + 1];
			last_px.u.b = cust_pal[l * 3 + 2];
			c_last = l;
			return l;
		}

		c_red = abs(cust_pal[l * 3 + 0] - px.u.r);
		c_green = abs(cust_pal[l * 3 + 1] - px.u.g);
		c_blue = abs(cust_pal[l * 3 + 2] - px.u.b);
		dist = (c_red + c_green + c_blue);

		/* is it better than the last? */
		if (dist < best) {
			best = dist;
			c_best = l;
		}

		if (l != 255) {
			l++;
		} else {
			break;
		}
	}

	c_last = c_best;
	return c_best;
}

void process_tga2lmp(char *filename)
{
	FILE *fLMP;					/* file Ident of the LUMP */
	byte *lmp_buff;				/* buffer of the LUMP */
	FILE *fTGA;					/* file Ident of the TARGA */
	byte tga_header[TGAHEADER];	/* TARGA Header (18 bytes usually) */
	byte *tga_buff;				/* 24bit gR input buffer + TGA header */
	int col, row, done;			/* used for the sorting loop */
	int img_w, img_h;			/* dimensions */

	/* load the TARGA */
	fTGA = fopen(filename, "rb");

	/* check whether the file exists or not */
	if (!fTGA) {
		fprintf(stderr, "couldn't find %s\n", filename);
		return;
	}

	/* put the TARGA header into the buffer for validation */
	fread(tga_header, 1, TGAHEADER, fTGA);

	/* only allow uncompressed, 24bit TARGAs */
	if (tga_header[2] != 2) {
		fprintf(stderr, "%s should be an uncompressed, RGB image\n", filename);
		return;
	}
	if (tga_header[16] != 24) {
		fprintf(stderr, "%s is not 24 bit in depth\n", filename);
		return;
	}

	/* read the resolution into an int (TGA uses shorts for the dimensions) */
	img_w = (tga_header[12]) | (tga_header[13] << 8);
	img_h = (tga_header[14]) | (tga_header[15] << 8);
	tga_buff = malloc(img_w * img_h * 3);

	if (tga_buff == NULL) {
		fprintf(stderr, "mem alloc failed at %d bytes\n", (img_w * img_h * 3));
		return;
	}

	/* skip to after the TARGA HEADER... and then read the buffer */
	fseek(fTGA, TGAHEADER, SEEK_SET);
	fread(tga_buff, 1, img_w * img_h * 3, fTGA);
	fclose(fTGA);

	/* start generating the lump data */
	lmp_buff = malloc(img_w * img_h + 8);

	if (lmp_buff == NULL) {
		fprintf(stderr, "mem alloc failed at %d bytes\n", (img_w * img_h + 8));
		return;
	}

	/* split the integer dimensions into 4 bytes */
	lmp_buff[3] = (img_w >> 24) & 0xFF;
	lmp_buff[2] = (img_w >> 16) & 0xFF;
	lmp_buff[1] = (img_w >> 8) & 0xFF;
	lmp_buff[0] = img_w & 0xFF;
	lmp_buff[7] = (img_h >> 24) & 0xFF;
	lmp_buff[6] = (img_h >> 16) & 0xFF;
	lmp_buff[5] = (img_h >> 8) & 0xFF;
	lmp_buff[4] = img_h & 0xFF;

	/* translate the rgb values into indexed entries and flip */
	done = 0;
	for (row = img_h - 1; row >= 0; row--) {
		for (col = 0; col < img_w; col++) {
			lmp_buff[8 + done] =
				pal24to8(tga_buff[((row * (img_w * 3)) + (col * 3 + 2))],
						tga_buff[((row * (img_w * 3)) + (col * 3 + 1))],
						tga_buff[((row * (img_w * 3)) + (col * 3 + 0))]);
			done++;
		}
	}

	/* FIXME: We assume too much!
	 * Save the output to FILENAME.tga
	 * This is ugly when the input name has no .lmp extension.
	 * But who's going to try that. ...right? */
	filename[strlen(filename)-3] = 'l';
	filename[strlen(filename)-2] = 'm';
	filename[strlen(filename)-1] = 'p';
	fprintf(stdout, "writing %s\n", filename);
	fLMP = fopen(filename, "w+b");
	fwrite(lmp_buff, 1, (img_w * img_h) + 8, fLMP);
	fclose(fLMP);
}

int main(int argc, char *argv[])
{
	int c;
	short p;
	FILE *fPAL;

	if (argc <= 1) {
		fprintf(stderr, "usage: tga2lmp [file.tga ...]\n");
		return 1;
	}

	fPAL = fopen("palette.lmp", "rb");

	if (!fPAL) {
		fprintf(stdout, "no palette.lmp found, using builtin palette.\n");
		for (p = 0; p < 256; p++) {
			cust_pal[p * 3 + 0] = pal_fallb[p].u.r;
			cust_pal[p * 3 + 1] = pal_fallb[p].u.g;
			cust_pal[p * 3 + 2] = pal_fallb[p].u.b;
		}
	} else {
		fprintf(stdout, "custom palette.lmp found\n");
		fread(cust_pal, 1, QPALSIZE, fPAL);
		fclose(fPAL);
	}

	for (c = 1; c < argc; c++)
		process_tga2lmp(argv[c]);

	return 0;
}
