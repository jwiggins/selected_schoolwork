#ifndef LRU_MEM_CACHE_H
#define LRU_MEM_CACHE_H

#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <deque>
#include "lruheap.h"
#include "messagetags.h"
#include "timer.h"

using namespace std;

class LRUMemCache {
public:
											LRUMemCache(char *cachedir, int sizelimit);
											~LRUMemCache();

	void								Reset(char *file, int datatype);
	
	void	*							FetchBlock(int id);

private:

	static void *				IOReqThreadFunc(void *arg);
	static void *				IORecvThreadFunc(void *arg);
	void								IOReqThread();
	void								IORecvThread();

	LRUHeap							*mHeap;
	Timer								*mCommTime, *mFetchTime;
	int									mSizeLim;
	int									mDatatype;
	char								*mCacheDir;
	unsigned char				*mBlockLocs;

	set<int>						mPendingFetches;
	deque<int>					mReqQueue;
	int									mRank;

	pthread_t						mIOReqThid, mIORecvThid;
	pthread_mutex_t			mCacheLock, mReqLock;
	pthread_cond_t			mBlockReq;
	bool								mExitFlag;
	
};

#endif
