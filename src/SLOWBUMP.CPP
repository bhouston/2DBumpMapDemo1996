/*
 *		SlowBump
 *			- A First Attemp At Bump-Mapping
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
#define	REFLECT			30
#define	AMBIENT			30
#define	DEG2RAD			0.01745329

char			bumpmapfile[13]	= "bumpmap.dat";
word			vscreen, bumpmap;
int			coslookup[MAX_ANGLE], sinlookup[MAX_ANGLE];
byte			palette[768];

void	fatalerror ( char *msg );

void	init ( void );
void	initlookup ( void );
void	initbumpmap ( void );
void	initgraphics ( void );
int	check386processor ( void );

void	drawsurface ( word x, word y );
void	vretrace ( void );
void	copyscreen ( word source, word dest );
void	getexacttime ( dword *time );

void	close ( void );

void	main ( void )
{
	byte	rot;
	word	x, y;
	dword	frames, starttime, endtime;

	printf ( "SlowBump - A First Attemp At Bump Mapping\n" );
	printf ( "Coded by PLaSH [AzurE] on August 12, 1996\n\n" );

	init ();

	rot = 0;
	frames = 0;
	getexacttime ( &starttime );
	do {

		x = ( coslookup[rot ++] >> 1 ) + 160;
		y = ( sinlookup[rot ++] >> 2 ) + 100;

		drawsurface ( x, y );
		vretrace ();
		copyscreen ( vscreen, 0xa000 );
		frames ++;
		} while ( ! kbhit () );
	getexacttime ( &endtime );
	getch ();
	close ();
	printf ( "average frames per second: %f\n", ( frames * 18.2 ) / ( endtime-starttime ) );
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


void	drawsurface ( word posx, word posy )
{
	word	x_one, y_one, x, y, j, i;

	asm	mov	di, 0

	asm	mov	ax, 1
	asm	sub	ax, word ptr [posy]
	asm	mov	word ptr [y_one], ax

	asm	mov	word ptr [i], 200


	boucle_Y:
		asm	mov	ax, 1
		asm	sub	ax, word ptr [posx]
		asm	mov	word ptr [x_one], ax
		asm	mov	word ptr [j], 318

		boucle_X:
			asm	mov	es, word ptr [bumpmap]

			asm	xor	bx, bx

			asm	xor	ax,ax
			asm	mov	al, es:[di+1]
			asm	mov	bl, es:[di-1]
			asm	sub	ax, bx
			asm	mov	word ptr [x], ax

			asm	xor 	ax, ax
			asm	mov 	al, es:[di+320]
			asm	mov	bl, es:[di-320]
			asm	sub 	ax, bx
			asm	mov 	[y], ax

			asm	inc	word ptr [x_one]

			asm	mov 	ax, [x]
			asm	sub 	ax, [x_one]
			asm	cwd
			asm	xor 	ax, dx
			asm	sub 	ax, dx
			asm	mov 	bx, 128
			asm	sub 	bx, ax
			asm	cmp 	bx, 0
			asm	jg  	lapin
			asm	mov	bx, 0
		lapin:
			asm	mov	ax, [y]
			asm	sub	ax, [y_one]
			asm	cwd
			asm	xor	ax, dx
			asm	sub	ax, dx
			asm	mov 	cx, 128
			asm	sub	cx, ax
			asm	cmp	cx, 0
			asm	jg		elephant
			asm	mov	cx, 0
		elephant:
			asm	mov	ax, bx
			asm	mul	cx
			asm	mov	bx, 64
			asm	div	bx
			asm	cmp	ax, 255
			asm	jbe	girafe
			asm	mov	ax, 255
		girafe:
			asm	mov	es, word ptr [vscreen]
			asm	stosb
			//asm	inc	di

			asm	dec	word ptr [j]
			asm	jnz	boucle_X

		asm	inc	word ptr [y_one]

		asm	add	di,2
		asm	dec	word ptr [i]
		asm	jz	skip

		goto	boucle_Y;
		skip:
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

	// make rotation lookups
	for ( int loop = 0; loop < MAX_ANGLE; loop ++ ) {
		sinlookup[loop] = sin ( (float)loop / MAX_ANGLE * M_PI * 2 ) * 256;
		coslookup[loop] = cos ( (float)loop / MAX_ANGLE * M_PI * 2 ) * 256;
		}

	putch ( '.' );
	initbumpmap ();
	putch ( '.' );
	initgraphics ();
}

void	initbumpmap ( void )
{
	int	loop, temp;
	real	intensity;

	// create phong palette
	for ( loop = 0; loop < 256; loop ++ ) {
		intensity = cos ( (248-loop) / 496.0 * M_PI );
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
	asm	mov	cx, 0xffff
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
	asm	mov	si, 320*5
	asm	mov	di, si
	asm	mov	cx, (64000 - 320 * 10 ) / 4
	asm	rep	movsd
	asm	pop	ds
}

