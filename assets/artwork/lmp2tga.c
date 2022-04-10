/*
LMP 2 TGA SOURCECODE

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
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#define TGAHEADER 18
#define QPALSIZE 768

typedef union
{
	int rgba:32; 
	struct {
		unsigned char b:8;
		unsigned char g:8;
		unsigned char r:8;
		unsigned char a:8;
	} u;
} palette_t;

/* Fallback palette from Quake, put into the Public Domain by John Carmack */
palette_t pal_fallb[256] = {
	0x000000,0x0f0f0f,0x1f1f1f,0x2f2f2f,0x3f3f3f,0x4b4b4b,0x5b5b5b,0x6b6b6b,
	0x7b7b7b,0x8b8b8b,0x9b9b9b,0xababab,0xbbbbbb,0xcbcbcb,0xdbdbdb,0xebebeb,
	0x0f0b07,0x170f0b,0x1f170b,0x271b0f,0x2f2313,0x372b17,0x3f2f17,0x4b371b,
	0x533b1b,0x5b431f,0x634b1f,0x6b531f,0x73571f,0x7b5f23,0x836723,0x8f6f23,
	0x0b0b0f,0x13131b,0x1b1b27,0x272733,0x2f2f3f,0x37374b,0x3f3f57,0x474767,
	0x4f4f73,0x5b5b7f,0x63638b,0x6b6b97,0x7373a3,0x7b7baf,0x8383bb,0x8b8bcb,
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
	0xbb739f,0xaf6b8f,0xa35f83,0x975777,0x8b4f6b,0x7f4b5f,0x734353,0x6b3b4b,
	0x5f333f,0x532b37,0x47232b,0x3b1f23,0x2f171b,0x231313,0x170b0b,0x0f0707,
	0xdbc3bb,0xcbb3a7,0xbfa39b,0xaf978b,0xa3877b,0x977b6f,0x876f5f,0x7b6353,
	0x6b5747,0x5f4b3b,0x533f33,0x433327,0x372b1f,0x271f17,0x1b130f,0x0f0b07,
	0x6f837b,0x677b6f,0x5f7367,0x576b5f,0x4f6357,0x475b4f,0x3f5347,0x374b3f,
	0x2f4337,0x2b3b2f,0x233327,0x1f2b1f,0x172317,0x0f1b13,0x0b130b,0x070b07,
	0xfff31b,0xefdf17,0xdbcb13,0xcbb70f,0xbba70f,0xab970b,0x9b8307,0x8b7307,
	0x7b6307,0x6b5300,0x5b4700,0x4b3700,0x3b2b00,0x2b1f00,0x1b0f00,0x0b0700,
	0x0000ff,0x0b0bef,0x1313df,0x1b1bcf,0x2323bf,0x2b2baf,0x2f2f9f,0x2f2f8f,
	0x2f2f7f,0x2f2f6f,0x2f2f5f,0x2b2b4f,0x23233f,0x1b1b2f,0x13131f,0x0b0b0f,
	0x2b0000,0x3b0000,0x4b0700,0x5f0700,0x6f0f00,0x7f1707,0x931f07,0xa3270b,
	0xb7330f,0xc34b1b,0xcf632b,0xdb7f3b,0xe3974f,0xe7ab5f,0xefbf77,0xf7d38b,
	0xa77b3b,0xb79b37,0xc7c337,0xe7e357,0x7fbfff,0xabe7ff,0xd7ffff,0x670000,
	0x8b0000,0xb30000,0xd70000,0xff0000,0xfff393,0xfff7c7,0xffffff,0x9f5b53
};

unsigned char cust_pal[QPALSIZE]; /* Custom 256 color palette */

void process_lmp2tga(char *filename)
{
	FILE *fLMP;
	int lmp_header[2];			/* Resolution of loaded LUMP file */
	unsigned char *lmp_buff;	/* Buffer of the LUMP */
	struct stat lmp_st;
	FILE *fTGA;
	unsigned char *tga_buff;	/* 24bit BGA output buffer + TGA header */
	int col, row, done; 		/* used for the sorting loop */

	fLMP = fopen(filename, "rb");

	if (!fLMP) {
		fprintf(stderr, "couldn't find %s\n", filename);
		return;
	}

	stat(filename, &lmp_st);

	if (lmp_st.st_size <= 8) {
		fprintf(stderr, "couldn't read %s\n", filename);
		return;
	}

	fread(lmp_header, sizeof(int) * 2, 1, fLMP);

	/* Other types of lumps (e.g. palette.lmp) don't have a header, skip them */
	if (lmp_st.st_size != (lmp_header[0] * lmp_header[1] + 8)) {
		fprintf(stderr, "incomplete lump %s, skipping\n", filename);
		return;
	}

	lmp_buff = malloc(lmp_header[0] * lmp_header[1]);
	fseek(fLMP, sizeof(int) * 2, SEEK_SET);
	fread(lmp_buff, 1, lmp_header[0] * lmp_header[1], fLMP);
	fclose(fLMP);

	/* Allocate enough memory for the header + buffer of the TARGA */
	tga_buff = malloc((lmp_header[0] * lmp_header[1] * 3) + TGAHEADER);

	memset (tga_buff, 0, 18);
	tga_buff[2] = 2;						/* Uncompressed TARGA */
	tga_buff[12] = lmp_header[0] & 0xFF;	/* Width */
	tga_buff[13] = lmp_header[0] >> 8;
	tga_buff[14] = lmp_header[1] & 0xFF;	/* Height */
	tga_buff[15] = lmp_header[1] >> 8;
	tga_buff[16] = 24;						/* Color depth */

	/* TARGAs are flipped in a messy way, 
	 * so we gotta do some sorting magic (vertical flip) */
	done = 0; /* readability */

	for (row = lmp_header[1] - 1; row >= 0; row--) {
		for (col = 0; col < lmp_header[0]; col++) {
			tga_buff[18 + ((row * (lmp_header[0] * 3)) + (col * 3 + 0))] = 
				cust_pal[lmp_buff[done] * 3 + 2]; 

			tga_buff[18 + ((row * (lmp_header[0] * 3)) + (col * 3 + 1))] = 
				cust_pal[lmp_buff[done] * 3 + 1];

			tga_buff[18 + ((row * (lmp_header[0] * 3)) + (col * 3 + 2))] = 
				cust_pal[lmp_buff[done] * 3 + 0];
			done++;
		}
	}

	/* FIXME: We assume too much!
	 * Save the output to FILENAME.tga
	 * This is ugly when the input name has no .lmp extension.
	 * But who's going to try that. ...right? */
	filename[strlen(filename)-3] = 't';
	filename[strlen(filename)-2] = 'g';
	filename[strlen(filename)-1] = 'a';
	fprintf(stdout, "writing %s\n", filename);
	fTGA = fopen(filename, "w+b");
	fwrite(tga_buff, 1, (lmp_header[0] * lmp_header[1] * 3) + TGAHEADER, fTGA);
	fclose(fTGA);
}

int main(int argc, char *argv[])
{
	int c;
	short p;
	FILE *fPAL;

	if (argc <= 1) {
		fprintf(stderr, "usage: lmp2tga [file ...]\n");
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
		process_lmp2tga(argv[c]);

	return 0;
}
