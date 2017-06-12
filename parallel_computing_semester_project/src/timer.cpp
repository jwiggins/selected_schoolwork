#include "timer.h"

Timer::Timer(const char *name)
{
	mName = (char *)malloc(strlen(name)+1);
	strcpy(mName, name);

	mElapsedTime = mLastStart = 0.0;
	mStopped = true;

	MPI_Comm_rank(MPI_COMM_WORLD, &mRank);
}

Timer::~Timer()
{
	free(mName);
}

void Timer::Start()
{
	// ignore if timer already started
	if (mStopped)
	{
		mStopped = false;
		mLastStart = MPI_Wtime();
	}
}

void Timer::Stop()
{
	// ignore if already stopped
	if (!mStopped)
	{
		double currentTime = MPI_Wtime();
		mStopped = true;
		mElapsedTime += currentTime - mLastStart;
	}
}

void Timer::Reset()
{
	// only stopped timers can be reset
	if (mStopped)
		mElapsedTime = mLastStart = 0.0;
}

void Timer::Print()
{
	if (mStopped)
		printf("[%d] Timer \"%s\": time elapsed %fs\n", mRank, mName, mElapsedTime);
	else
		printf("[%d] Timer \"%s\": time elapsed %fs\n", mRank, mName, Read());
}

double Timer::Read()
{
	if (mStopped)
		return mElapsedTime;

	double currentTime = MPI_Wtime();
	return mElapsedTime + (currentTime-mLastStart);
}
