
# Experiments in 2D Phong Bump-Mapping

<image src="./bestbump_screencap.jpg"></image>

Ben Houston / PLaSH of AzurE
E-Mail: bhouston@geocities.com

Friday, August 30, 1996 (Ottawa Release)
Friday, November 22, 1996 (Internet Release)

## Overview

Sometime back in July I downloaded a source and doc written by a
member of the french demo group Flower Corp showing an example of
bump-mapping.  The algorithm for the 2d bump-mapping was interesting
but fairly slow on my 486 computer.  I attempted a few improvements
and the sources 'slowbump.cpp', 'fastbump.cpp' and 'bestbump.cpp' are
the result.  I'm not going to explain directly the different
techniques of how to bump-map 2D surfaces but you should be able to
decipher by sources.  If you want a document albeit in french on the
technique used in 'slowbump.cpp' you can download Flower Corp's
archive called 'flcbump.zip'.

The bump-mapping inner loop used in the first example source
'slowbump.cpp' is almost a exact rip from Flower Corps example source. 
I don't mean offence to Flower Corp by the source's name it's just
what I called it many months ago.  The only changes I made to the look
of the bump-map was the addition of a phong palette (the Flower Corp
demo use a linear palette).  The slowness of this example is causes by
the need for a multiply per lighted pixel.

The first improvement in the actual algorithm is seen in
'fastbump.cpp'.  Here I replace the simple multiply per lighted pixel
algorithm with a technique of wrapping a bitmap on the bumpy surface. 
I calculate the bitmap to contains a circular (although not in the
least correct) highlight.

The second major improvement changes the format of the run-time
surface data from pixel heights to height deltas between pixels.  This
causes the surface to double in memory usage but reduces the run-time
calculations tremendously.  The major drawback of this implementation
is that the animation that is used in the second part of
'fastbump.cpp' is not longer so easy.

The frame rate that these demos achieved on my computer was 5 fps
for 'slowbump.cpp', 8 fps (6 fps with animation) for 'fastbump.cpp',
and 16 fps for 'bestbump.cpp'.  I hope you can benefit in some way
from the three sources or at least enjoy the bump-mapping examples. 
Just remember not to rip and give credit.  I like to thank first of
all Flower Corp for releasing there bump-mapping information, Rex
Deathstar for his wonderful examples of inline assembly coding, and
everyone else who releases their old code for others to learn from. 
Feel free to e-mail me for any reason at all.  Greets to by buds
Mundane and Asch of Azure, Omega and the rest of Intra, Grimace of
Miracle, Cyclone and the rest from NAID'96.

## Contents

* SLOWBUMP.CPP, source of a bump-mapping algorithm
* FASTBUMP.CPP, source of a bump-mapping algorithm
* BESTBUMP.CPP, source of a bump-mapping algorithm
* BUMPMAP.DAT, a common data file used by all executables
