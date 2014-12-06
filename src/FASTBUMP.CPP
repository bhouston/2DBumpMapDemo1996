/*
 *		FastBump
 *			- A Second Attemp At Bump-Mapping
 *
 *		Copyright Ben Houston [SLaPH | AzurE]
 *		Started: Monday, August 12, 1996
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
#define	REFLECT			100
#define	AMBIENT			20
#define	DEG2RAD			0.01745329

char			bumpmapfile[13]	= "bumpmap.dat";
word			vscreen, bumpmap, phongmap, texture;
int			coslookup[MAX_ANGLE], sinlookup[MAX_ANGLE];
byte			palette[768];

void	fatalerror ( char *msg );

void	init ( void );
void	initphongmap ( void );
void	initbumpmap ( void );
void	initgraphics ( void );
int	check386processor ( void );

void	fadepalette ( byte *pal, byte fade );
void	drawsurface ( word source, int xlight, int ylight, word dest );
void	vretrace ( void );
void	copyscreen ( word source, word dest );
void	copypartialscreen ( word source, word dest );
void 	average ( word source, word dest );
void	fade ( word source, word dest );
void	dot ( word dest, int x, int y, byte color );
void	getexacttime ( dword * time );

void	close ( void );

void	main ( void )
{
	byte	xrot, yrot;
	word	x, y;
	dword	frames, endtime, starttime;

	printf ( "FastBump - A Second Attemp At Bump Mapping\n" );
	printf ( "Coded by PLaSH [AzurE] on August 12, 1996\n\n" );

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
	getexacttime ( &endtime );
	getch ();

	do {
		x = ( coslookup[xrot += 2] * 0.40 ) + 160;
		y = ( sinlookup[yrot ++] * 0.2 ) + 100;
		fade ( bumpmap, vscreen );
		copyscreen ( vscreen, bumpmap );
		drawsurface ( bumpmap, x, y, vscreen );
		vretrace ();
		copypartialscreen ( vscreen, 0xa000 );
		} while ( ! kbhit () );
	getch ();

	do {
		x = ( coslookup[xrot += 2] * 0.40 ) + 160;
		y = ( sinlookup[yrot ++] * 0.2 ) + 100;
		drawsurface ( bumpmap, x, y, vscreen );
		vretrace ();
		copypartialscreen ( vscreen, 0xa000 );
		} while ( ! kbhit () );
	getch ();

	do {
		x = ( coslookup[xrot += 2] * 0.40 ) + 160;
		y = ( sinlookup[yrot ++] * 0.2 ) + 100;
		average ( bumpmap, vscreen );
		copyscreen ( vscreen, bumpmap );
		drawsurface ( bumpmap, x, y, vscreen );
		vretrace ();
		copypartialscreen ( vscreen, 0xa000 );
		} while ( ! kbhit () );
	getch ();

	close ();

	printf ( "average frames per second for part one: %f\n", ( frames * 18.2 ) / ( endtime-starttime ) );
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


void	fadepalette ( byte *pal, byte fade )
{
	int color;

	asm	lea	si, [palette]
	asm	mov	cx, 768
	asm	mov	dx, 0x3c8
	asm	xor	al, al
	asm	out	dx, al
	for ( int i = 0; i < 768; i ++ ) {
		color = pal[i] * fade / 64;
		asm	mov	al, byte ptr [color]
      asm	mov	dx, 0x3c9
		asm	out	dx, al
		}
}

void	dot ( word dest, int x, int y, byte color )
{
	asm	mov	es, [dest]
	asm	mov	ax, [y]
	asm	mov	bx, 320
	asm	mul	bx
	asm	mov	di, ax
	asm	add	di, [x]
	asm	mov	al, [color]
	asm	mov	ah, [color]
	asm	shl	eax, 16
	asm	mov	al, [color]
	asm	mov	ah, [color]
	asm	inc	di
	asm	stosd
	asm	add	di, 315
	asm	stosd
	asm	stosw
	asm	add	di, 314
	asm	stosd
	asm	stosw
	asm	add	di, 314
	asm	stosd
	asm	stosw
	asm	add	di, 315
	asm	stosd
}

void	drawsurface ( word source, int xlight, int ylight, word dest )
{
	int	xdistance, ydistance;
	int	xdelta, ydelta;
	word	j, i;

	xdelta = 0;
	ydelta = 0;

	asm	mov	es, word ptr [source]
	asm	mov	fs, word ptr [phongmap]
	asm	mov	gs, word ptr [dest]
	asm	mov	di, 0

	ydistance = - ylight;
	asm	mov	word ptr [i], 200

	boucle_Y:
		xdistance = - xlight;
		asm	mov	word ptr [j], 320

		boucle_X:
			asm	xor	ax,ax
			asm	xor	bx, bx
			asm	mov	bl, es:[di+1]
			asm	mov	al, es:[di-1]
			asm	sub	ax, bx
			asm   shr	ax, 1
			asm	add	ax, 127
			asm	mov	byte ptr [xdelta], al

			asm	xor 	ax, ax
			asm	xor	bx, bx
			asm	mov 	bl, es:[di+320]
			asm	mov	al, es:[di-320]
			asm	sub 	ax, bx
			asm   shr	ax, 1
			asm	add	ax, 127
			asm	mov 	byte ptr [ydelta], al

			asm	inc	word ptr [xdistance]

			asm	mov 	ax, word ptr [xdistance]
			asm	add 	ax, word ptr [xdelta]
			asm	test	ah, ah
			asm	jnz	end_of_inner

			asm	mov 	bx, word ptr [ydistance]
			asm	add 	bx, word ptr [ydelta]
			asm	test	bh, bh
			asm	jnz	end_of_inner

			asm	mov	bh, al
			asm	mov	si, bx
			asm	mov	al, fs:[si]
			asm	mov	gs:[di], al
			asm	inc	di
			asm	dec	word ptr [j]
			asm	jnz	boucle_X
			asm	jmp	here

		end_of_inner:
			asm	xor	al, al
			asm	mov	gs:[di], al
			asm	inc	di
			asm	dec	word ptr [j]
			asm	jnz	boucle_X
		here:

		asm	inc	word ptr [ydistance]

//		asm	add	di,2
		asm	dec	word ptr [i]
		asm	jz	skip

		goto	boucle_Y;
		skip:
}

void average ( word source, word dest )
{
	asm	push	ds

	asm	mov	ds, [source]
	asm	mov	es, [dest]
	asm	mov	di, 0
	asm	mov	cx, 64000

	pixel_loop:
		asm	xor	bx, bx
		asm	mov	bl, byte ptr ds:[di-320]
		asm	mov   ax, bx
		asm	mov	bl, byte ptr ds:[di-1]
		asm	add	ax, bx
		asm	mov	bl, byte ptr ds:[di+1]
		asm	add   ax, bx
		asm	mov	bl, byte ptr ds:[di+320]
		asm	add	ax, bx
		asm	shr	ax, 2
		asm	jz		minimum
		asm	dec	ax
		minimum:
		asm	stosb
		asm	dec	cx
		asm	jnz	pixel_loop

	asm	pop	ds
}

void fade ( word source, word dest )
{
	asm	push	ds

	asm	mov	ds, [source]
	asm	mov	es, [dest]
	asm	mov	di, 0
	asm	mov	cx, 64000

	pixel_loop:
		asm	mov	al, byte ptr ds:[di]
		asm	sub	al, 5
		asm	jnc	minimum
		asm	xor	al, al
		minimum:
		asm	stosb
		asm	dec	cx
		asm	jnz	pixel_loop

	asm	pop	ds
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
	if ( allocmem ( 4000, &bumpmap ) != -1 )
		fatalerror ( "   *** sorry, out of memory (allocation #3)" );
	if ( allocmem ( 4096, &phongmap ) != -1 )
		fatalerror ( "   *** sorry, out of memory (allocation #3)" );

	// make rotation lookups
	for ( int loop = 0; loop < MAX_ANGLE; loop ++ ) {
		sinlookup[loop] = sin ( (float)loop / MAX_ANGLE * M_PI * 2 ) * 256;
		coslookup[loop] = cos ( (float)loop / MAX_ANGLE * M_PI * 2 ) * 256;
		}

	putch ( '.' );
	initphongmap ();
	putch ( '.' );
	initbumpmap ();
	putch ( '.' );
	initgraphics ();
}

void	initphongmap ( void )
{
	int	x, y, temp;
	real	intensity;

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
	int	loop, temp;
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

	// load raw bump map
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
	asm	mov	ds, [bumpmap]
	asm	xor	dx, dx
	asm	int	0x21
	asm	pop	ds

	// close the file
	asm	mov	bx, bp
	asm	mov	ah, 0x3e
	asm	int	0x21
	asm	pop	bp

	copyscreen ( bumpmap, texture );
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
}

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

