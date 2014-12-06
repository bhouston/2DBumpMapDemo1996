/*
 *		BestBump
 *			- A Third Attemp At Bump-Mapping
 *
 *		Copyright Ben Houston [SLaPH | AzurE]
 *		Started: Wensday, August 14, 1996
 *		Updated: n/a
 */


#pragma	inline
asm	.386p

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>

typedef	unsigned int	boolean;
typedef	unsigned char	byte;
typedef	unsigned int	word;
typedef	unsigned long	dword;
typedef	float				real;
enum	boolean_values { false = 0, true = 1 };

#define	MAX_ANGLE		256
#define	FACE_RED			40
#define	FACE_GREEN		20
#define	FACE_BLUE		17
#define	LIGHT				40
#define	REFLECT			150
#define	AMBIENT			0
#define	DEG2RAD			0.01745329

char			bumpmapfile[13]	= "bumpmap.dat";
word			vscreen, bumpmap, bumpmap2, phongmap;
int			coslookup[MAX_ANGLE], sinlookup[MAX_ANGLE];
byte			palette[768];

void	fatalerror ( char *msg );

void	init ( void );
void	initphongmap ( void );
void	initbumpmap ( void );
void	initgraphics ( void );
int	check386processor ( void );
void	createbumpmap ( word source, word offset, word dest );

void	getexacttime ( dword *timeptr );
//void	fadepalette ( byte *pal, byte fade );
void	drawsurface ( word source, int xlight, int ylight, word dest );
void	vretrace ( void );
//void	copyscreen ( word source, word dest );
void	copypartialscreen ( word source, word dest );

void	close ( void );

void	main ( void )
{
	byte	xrot, yrot;
	word	x, y;
	dword	starttime, endtime, frames;

	printf ( "BestBump - A Third Attemp At Bump Mapping\n" );
	printf ( "Coded by PLaSH [AzurE] on August 14, 1996\n\n" );

	init ();

	xrot = 0;
	yrot = 32;
	frames = 0;
	getexacttime ( &starttime );
	do {
		x = ( coslookup[xrot += 2] * 0.40 ) + 160;
		y = ( sinlookup[yrot ++] * 0.2 ) + 100;

		drawsurface ( bumpmap, x, y, vscreen );
		vretrace ();
		copypartialscreen ( vscreen, 0xa000 );
		frames++;
		} while ( ! kbhit () );
	getch ();
	getexacttime ( &endtime );

	close ();
	printf ( "average frames per second: %f", ( frames * 18.2 ) / ( endtime-starttime ) );
	getch ();
}


void	getexacttime ( dword *timeptr )
{
	// Returns the number of times the timer interrupt handler has been
	//  called.  Therefore divide this timer by 18.2 to get seconds.
	//  The most accurate way to time without reprogramming the PIC.

	asm	mov	si, [timeptr]
	asm	mov	ax, 0x0040
	asm	mov	es, ax
	asm	mov	di, 0x006C
	asm	mov	eax, dword ptr es:[di]
	asm	mov	dword ptr ds:[si], eax
}

void	drawsurface ( word source, int xlight, int ylight, word dest )
{
	int	xtemp, ytemp;

	ylight = -ylight;
	xlight = -xlight;

	asm	xor	ebx, ebx
	asm	mov	di, 320 * 4
	asm	mov	si, 640 * 4
	asm	mov	dx, word ptr [ylight]
	asm	add	dx, 4
	asm	mov	word ptr [ytemp], dx
	asm	push	ds
	asm	mov	fs, word ptr [phongmap]
	asm	mov	es, word ptr [dest]
	asm	mov	ds, word ptr [source]
	asm	mov	cx, 96

	line_loop:
		asm	mov	dx, word ptr [xlight]
		asm	mov	word ptr [xtemp], dx
		asm	shl	ecx, 16
		asm	mov	cx, 160

		pixel_loop:
			asm	lodsd
			asm	mov	edx, eax
			asm	xor	bh, bh
			asm	mov	bl, dh
			asm	inc	word ptr [xtemp]
			asm	xor	dh, dh
			asm	add	bx, word ptr [ytemp]
			asm	add	dx, word ptr [xtemp]
			asm	add	dh, bh
			asm	jz		a_lighted_pixel

		a_unlighted_pixel:
			asm	xor	al, al
			asm	jmp	do_pixel_b

		a_lighted_pixel:
			asm	mov	bh, dl
			asm	mov	al, fs:[ebx]

		do_pixel_b:
			asm	shr	edx, 16
			asm	xor	bh, bh
			asm	mov	bl, dh
			asm	inc	word ptr [xtemp]
			asm	xor	dh, dh
			asm	add	bx, word ptr [ytemp]
			asm	add	dx, word ptr [xtemp]
			asm	add	dh, bh
			asm	jnz	b_unlighted_pixel

		b_lighted_pixel:
			asm	mov	bh, dl
			asm	mov	ah, fs:[ebx]
			asm	dec	cx
			asm	stosw
			asm	jnz	pixel_loop
			asm	jmp	pixel_loop_end

		b_unlighted_pixel:
			asm	xor	ah, ah
			asm	dec	cx
			asm	stosw
			asm	jnz	pixel_loop

		pixel_loop_end:

		asm	shr	ecx, 16
		asm	inc	word ptr [ytemp]
		asm	dec	cx
		asm	jnz	line_loop

	asm	pop	ds
	asm	push	ds
	asm	mov	ds, [bumpmap2]
	asm	mov	si, 0
	asm	mov	cx, 96
	line_loop2:
		asm	mov	dx, word ptr [xlight]
		asm	mov	word ptr [xtemp], dx
		asm	shl	ecx, 16
		asm	mov	cx, 160

		pixel_loop2:
			asm	lodsd
			asm	mov	edx, eax
			asm	xor	bh, bh
			asm	mov	bl, dh
			asm	inc	word ptr [xtemp]
			asm	xor	dh, dh
			asm	add	bx, word ptr [ytemp]
			asm	add	dx, word ptr [xtemp]
			asm	add	dh, bh
			asm	jz		a_lighted_pixel2

		a_unlighted_pixel2:
			asm	xor	al, al
			asm	jmp	do_pixel_b2

		a_lighted_pixel2:
			asm	mov	bh, dl
			asm	mov	al, fs:[ebx]

		do_pixel_b2:
			asm	shr	edx, 16
			asm	xor	bh, bh
			asm	mov	bl, dh
			asm	inc	word ptr [xtemp]
			asm	xor	dh, dh
			asm	add	bx, word ptr [ytemp]
			asm	add	dx, word ptr [xtemp]
			asm	add	dh, bh
			asm	jnz	b_unlighted_pixel2

		b_lighted_pixel2:
			asm	mov	bh, dl
			asm	mov	ah, fs:[ebx]
			asm	dec	cx
			asm	stosw
			asm	jnz	pixel_loop2
			asm	jmp	pixel_loop_end2

		b_unlighted_pixel2:
			asm	xor	ah, ah
			asm	dec	cx
			asm	stosw
			asm	jnz	pixel_loop2

		pixel_loop_end2:

		asm	shr	ecx, 16
		asm	inc	word ptr [ytemp]
		asm	dec	cx
		asm	jnz	line_loop2

	asm	pop	ds
}

void	createbumpmap ( word source, word sourceoffset, word dest )
{
	asm	mov	es, word ptr [source]
	asm	mov	fs, word ptr [dest]
	asm	mov	di, word ptr [sourceoffset]
	asm	xor	si, si
	asm	mov	cx, 32000

	pixel_loop:
		asm	xor	ax, ax
		asm	xor	bx, bx
		asm	mov	bl, es:[di+1]
		asm	mov	al, es:[di-1]
		asm	sub	ax, bx
		asm   shr	ax, 1
		asm	add	ax, 127
		asm	mov	dl, al

		asm	xor 	ax, ax
		asm	xor	bx, bx
		asm	mov 	bl, es:[di+320]
		asm	mov	al, es:[di-320]
		asm	sub 	ax, bx
		asm   shr	ax, 1
		asm	add	ax, 127
		asm	mov 	dh, al

		asm	mov	fs:[si], dx
		asm	add	si, 2
		asm	inc	di
		asm	dec	cx
		asm	jnz	pixel_loop

}

void	init ( void )
{
	printf ( "initializing please wait" );
	delay ( 500 );

	if ( ! check386processor () )
		fatalerror ( "   *** sorry, you need at least a 386 processor" );

	putch ( '.' );
	if ( allocmem ( 4000, &vscreen ) != -1 )
		fatalerror ( "   *** sorry, out of memory (allocation #1)" );
	if ( allocmem ( 4000, &bumpmap ) != -1 )
		fatalerror ( "   *** sorry, out of memory (allocation #2)" );
	if ( allocmem ( 4000, &bumpmap2 ) != -1 )
		fatalerror ( "   *** sorry, out of memory (allocation #3)" );
	if ( allocmem ( 4096, &phongmap ) != -1 )
		fatalerror ( "   *** sorry, out of memory (allocation #4)" );

	// make rotation lookups
	for ( int loop = 0; loop < MAX_ANGLE; loop ++ ) {
		sinlookup[loop] = sin ( (float)loop / MAX_ANGLE * M_PI * 2 ) * 256;
		coslookup[loop] = cos ( (float)loop / MAX_ANGLE * M_PI * 2 ) * 256;
		}

	putch ( '.' );
	initbumpmap ();
	putch ( '.' );
	initphongmap ();
	putch ( '.' );
	initgraphics ();
}

void	initphongmap ( void )
{
	int	x, y, loop, temp;
	real	intensity;

	// create phong palette
	for ( loop = 0; loop < 256; loop ++ ) {
		intensity = cos ( (255-loop) / 512.0 * M_PI );
		temp = FACE_RED * AMBIENT / 63.0 + FACE_RED * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
		palette[loop*3] = temp;
		temp = FACE_GREEN * AMBIENT / 63.0 + FACE_GREEN * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
		palette[loop*3+1] = temp;
		temp = FACE_BLUE * AMBIENT / 63.0 + FACE_BLUE * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
		palette[loop*3+2] = temp;
		}

	asm	mov	es, [phongmap]
	asm	xor	di, di
	asm	mov	cx, 0x8000
	asm	xor	eax, eax
	asm	rep	stosd

	for ( x = 0; x < 128; x ++ )
		for ( y = 0; y < 128; y ++ ) {
			intensity = 1 - pow ( x / 127.5 - 1, 2 ) - pow ( y / 127.5 - 1, 2 );
			if ( intensity >= 0 ) {
				temp = pow ( intensity, 3 ) * 255;
				asm	mov	bh, byte ptr [x]
				asm	mov	bl, byte ptr [y]
				asm	mov	es, word ptr [phongmap]
				asm	mov	di, bx
				asm	mov	al, byte ptr [temp]
				asm	mov	es:[di], al
				}
			}
	for ( x = 0; x < 128; x ++ )
		for ( y = 0; y < 128; y ++ ) {
			asm	mov	es, word ptr [phongmap]
			asm	mov	ch, byte ptr [x]
			asm	mov	cl, byte ptr [y]
			asm	mov	dx, cx
			asm	mov	si, cx
			asm	mov	bx, 0xffff
			asm	sub	bx, cx
			asm	mov   al, es:[si]
			asm	mov	di, bx
			asm	mov	es:[di], al
			asm	mov	cl, bl
			asm	mov	di, cx
			asm	mov	es:[di], al
			asm	mov	dh, bh
			asm	mov	di, dx
			asm	mov	es:[di], al
			}
}

void	initbumpmap ( void )
{
	// open the file with read only access (ax=handle on return)
	asm	push	bp
	asm	mov	ax, 0x3d00
	asm	lea	dx, [bumpmapfile]
	asm	int	0x21
	asm	mov	bp, ax
	asm	jnc	file_ok
	asm	pop	bp
	fatalerror ( "   *** error, cannot open texture map resource file" );

	// read the raw data
	file_ok:
	asm	mov	ah, 0x3f
	asm	mov   bx, bp
	asm	mov	cx, 64000
	asm	push	ds
	asm	mov	ds, [vscreen]
	asm	xor	dx, dx
	asm	int	0x21
	asm	pop	ds

	// close the file
	asm	mov	bx, bp
	asm	mov	ah, 0x3e
	asm	int	0x21
	asm	pop	bp

	// post process bumpmap
	createbumpmap ( vscreen, 0, bumpmap );
	createbumpmap ( vscreen, 32000, bumpmap2 );
}

void	initgraphics ( void )
{
	// clear virtual screen
	asm	mov	es, [vscreen]
	asm	xor	di, di
	asm	mov	cx, 16000
	asm	xor	eax, eax
	asm	rep	stosd

	// set mcga video mode
	asm	mov	ax, 0x13
	asm	int	0x10

	// set vga palette
	asm	lea	si, [palette]
	asm	mov	cx, 768
	asm	mov	dx, 0x3c8
	asm	xor	al, al
	asm	out	dx, al
	asm	inc	dx
	asm	rep	outsb
}

void	close ( void )
{
	asm	mov	ax, 0x03	// text mode switch
	asm	int	0x10

	freemem ( vscreen );
	freemem ( bumpmap );
	freemem ( bumpmap2 );
	freemem ( phongmap );

	delay ( 500 );
}

int	check386processor ( void )
{
	int	is386;

	asm	pushf
	asm	xor	ah, ah
	asm	push	ax
	asm	popf
	asm	pushf
	asm	pop	ax
	asm	and	ah, 0xf0
	asm	cmp	ah, 0xf0
	asm	je		no386
	asm	mov	ah, 0x70
	asm	push	ax
	asm	popf
	asm	pushf
	asm	pop	ax
	asm	and	ah, 0x70
	asm	jz		no386
	asm	popf
	asm	mov	word ptr [is386], 1
	asm	jmp	end
	no386:
	asm	mov	word ptr [is386], 0
	end:

	return	is386;
}

void	fatalerror ( char *msg )
{
	printf ( "\n%s", msg );
	printf ( "\ninitialization failed. press a key to quit..." );
	getch ();
	exit ( 0 );
}

void	vretrace ( void )
{
	asm	mov	dx, 0x3da
	retrace_end:
		asm	in		al, dx
		asm	test	al, 0x8
		asm	jnz	retrace_end
	retrace_start:
		asm	in		al, dx
		asm	test	al, 0x8
		asm	jz		retrace_start
}
/*
void	copyscreen ( word source, word dest )
{
	asm	push	ds
	asm	mov	ds, [source]
	asm	mov	es, [dest]
	asm	mov	si, 0
	asm	mov	di, si
	asm	mov	cx, 16000
	asm	rep	movsd
	asm	pop	ds
} */

void	copypartialscreen ( word source, word dest )
{
	asm	push	ds
	asm	mov	ds, [source]
	asm	mov	es, [dest]
	asm	mov	si, 320*5
	asm	mov	di, si
	asm	mov	cx, 320*190/4
	asm	rep	movsd
	asm	pop	ds
}

