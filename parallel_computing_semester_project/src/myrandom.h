/*
	myrandom.h
	$Id: myrandom.h,v 1.2 2003/05/02 23:55:15 prok Exp $

	$Log: myrandom.h,v $
	Revision 1.2  2003/05/02 23:55:15  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
	Revision 1.1  2003/04/25 21:49:35  prok
	This is a simple pseudo-random number generator for use by the random
	distribution code.
		
*/

#ifndef MY_RANDOM_H
#define MY_RANDOM_H

void mysrandom(int seed);
int myrandom(int from, int to);

#endif

