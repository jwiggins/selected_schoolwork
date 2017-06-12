/*
	mpilib.h
	$Id: mpilib.h,v 1.1 2003/05/02 23:55:14 prok Exp $

	$Log: mpilib.h,v $
	Revision 1.1  2003/05/02 23:55:14  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
*/

#ifndef MY_MPILIB_H
#define MY_MPILIB_H

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

void logNMergeImage(unsigned char *buffer, int bufsize, int root, int start, int end,
								MPI_Comm comm);
void my_bcast(void *buf, int count, MPI_Datatype dt, int root, int start, int end,
								MPI_Comm comm);

#endif

