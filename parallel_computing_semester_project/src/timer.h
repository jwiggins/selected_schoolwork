#ifndef PROK_TIMER_H
#define PROK_TIMER_H

#include <mpi.h> //MPI_Wtime()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

class Timer {
public:
		Timer(const char *name);
		~Timer();

void		Start();
void		Stop();
void		Reset();

void		Print();

double		Read();


private:

double		mElapsedTime;
double		mLastStart;
int		mRank;
char		*mName;
bool		mStopped;
};

#endif
