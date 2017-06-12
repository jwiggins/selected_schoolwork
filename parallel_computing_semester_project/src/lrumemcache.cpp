#include "lrumemcache.h"

LRUMemCache::LRUMemCache(char *cachedir, int sizelimit)
{
	// remember where our block id files are
	mCacheDir = strdup(cachedir);
	
	// note the size limit and datatype
	mSizeLim = sizelimit *1024*1024; // given in MB
	mDatatype = 0;

	// get our rank
	MPI_Comm_rank(MPI_COMM_WORLD, &mRank);
	
	// init the heap
	mHeap = new LRUHeap();

	// init the timers
	mCommTime = new Timer("memory cache comm timer");
	mFetchTime = new Timer("memory cache fetch timer");

	// init the block location buffer
	mBlockLocs = NULL;
	
	// init all the threading stuff
	mExitFlag = false;
	pthread_cond_init(&mBlockReq, NULL);
	pthread_mutex_init(&mCacheLock, NULL);
	pthread_mutex_init(&mReqLock, NULL);

	// spawn the IO request thread
	//pthread_create(&mIOReqThid, NULL, IOReqThreadFunc, (void *)this);
	// spawn the IO receive thread
	//pthread_create(&mIORecvThid, NULL, IORecvThreadFunc, (void *)this);
}

LRUMemCache::~LRUMemCache()
{
	// signal the IO request thread
	mExitFlag = true;
	//pthread_mutex_lock(&mReqLock);
	//pthread_cond_signal(&mBlockReq);
	//pthread_mutex_unlock(&mReqLock);
	// wait for the IO request thread to exit
	//pthread_join(mIOReqThid, NULL);
	// detach the IO receive thread (it needs an MPI message to quit it)
	//pthread_detach(mIORecvThid);

	// clean up all the pthread related spoo
	pthread_mutex_destroy(&mCacheLock);
	pthread_mutex_destroy(&mReqLock);
	pthread_cond_destroy(&mBlockReq);

	// print the amount of time spent communicating and fetching
	mCommTime->Print();
	//mFetchTime->Print();
	fflush(stdout);

	// free memory
	delete mHeap;
	delete mCommTime;
	delete mFetchTime;
	if (mBlockLocs)
		free(mBlockLocs);
}

void LRUMemCache::Reset(char *file, int datatype)
{
	char fname[255];
	FILE *fp;
	int fsize;
	
	// cache the new type
	mDatatype = datatype;

	// clear the heap, giving it a new max # of blocks
	mHeap->Reset(mSizeLim / (mDatatype*1000));

	// clear the pending fetches set (it should be empty already)
	//mPendingFetches.clear();
	// the same goes for the request queue
	//mReqQueue.clear();

	// load the block ids for the new dataset
	sprintf(fname, "%s/%s.ids", mCacheDir, file);
	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		printf("could not open block id file %s\n", fname);
		exit(-1);
	}
	// find out how big the buffer needs to be
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	// resize the buffer
	mBlockLocs = (unsigned char *)realloc(mBlockLocs, fsize);
	// fill the buffer
	fread(mBlockLocs, fsize, 1, fp);
	// close the file
	fclose(fp);
}

void *LRUMemCache::FetchBlock(int id)
{
#ifdef _DEBUG
	//printf("{");
	//fflush(stdout);
#endif

	void *ret;
	bool needFetch = false;

	// fetch the block from the local cache.
	// ret will be NULL if the block is not resident
	//pthread_mutex_lock(&mCacheLock);
	//mFetchTime->Start();
	ret = mHeap->FetchBlock(id);
	//mFetchTime->Stop();
	// if the block wasn't there, add it to the pending fetch list if it's not
	// on that list already
	//if (ret == NULL && mPendingFetches.count(id) == 0)
	//{
	//	mPendingFetches.insert(id);
	//	needFetch = true;
	//}
	//pthread_mutex_unlock(&mCacheLock);

	if (ret == NULL)
	{
		int recvr, sender, req[2];
		MPI_Status status;
		char *rcvBuf=NULL, *bptr;
		int bid=0, blksize = mDatatype*1000;
		void *block = malloc(blksize); // allocate mem for a block

		// request blocks
#ifdef _DEBUG
		//printf("%d ", id);
		//fflush(stdout);
#endif
		// find out who to ask
		recvr = (int)mBlockLocs[id];

#ifdef _DEBUG
		//printf("0");
		//fflush(stdout);
#endif
		// start the communication timer
		mCommTime->Start();

		// do the request
		// construct
		req[0] = id; // the block id
		req[1] = mRank; // who's asking
		// send
#ifdef _DEBUG
		//printf("1");
		//fflush(stdout);
#endif
		MPI_Send(req, 2, MPI_INT, recvr, PVOL_BLK_REQ1_TAG, MPI_COMM_WORLD);
#ifdef _DEBUG
		//printf("2");
		//fflush(stdout);
#endif
		//MPI_Send(&id, 1, MPI_INT, recvr, PVOL_BLK_REQ2_TAG, MPI_COMM_WORLD);

		// get the reply
		// Part I
#ifdef _DEBUG
		//printf("3");
		//fflush(stdout);
#endif
		//MPI_Recv(req, 2, MPI_INT, MPI_ANY_SOURCE, PVOL_BLK_RPLY1_TAG,
		//								MPI_COMM_WORLD, &status);

		// resize the receive buffer
		//bptr = rcvBuf = (char *)realloc(rcvBuf, req[0]);
		// set the sender
		sender = recvr; //req[1];

		// Part II
#ifdef _DEBUG
		//printf("4");
		//fflush(stdout);
#endif
		//MPI_Recv(rcvBuf, req[0], MPI_CHAR, sender, PVOL_BLK_RPLY2_TAG,
		//								MPI_COMM_WORLD, &status);
		MPI_Recv(block, blksize, MPI_CHAR, sender, PVOL_BLK_RPLY2_TAG,
										MPI_COMM_WORLD, &status);

#ifdef _DEBUG
		//printf("5");
		//fflush(stdout);
#endif

		// stop the communication timer
		mCommTime->Stop();

#ifdef _DEBUG
		//if (mRank == 9)
		//{
		// printf("reply: req[0] = %d, blksize = %d\n", req[0], blksize);
		// fflush(stdout);
		//}
#endif
		
		// copy received data into the block cache
		// copy the block id from the buffer
		//memcpy(&bid, bptr, sizeof(int));
		//bptr += sizeof(int);

		// copy the block data into the newly allocated block
		//memcpy(block, bptr, blksize);
		//bptr += blksize;

		// lock the block cache
		//pthread_mutex_lock(&mCacheLock);
		// add the new block
		mHeap->AddBlock(id, block);
		// unlock the block cache
		//pthread_mutex_unlock(&mCacheLock);

		//realloc(rcvBuf, 0);
		ret = block;
	}
#ifdef _DEBUG
		//printf("6");
		//fflush(stdout);
#endif


	/*if (ret == NULL && needFetch)
	{

#ifdef _DEBUG
		printf("node %d requesting block %d, Q size %d\n", mRank, id,
		mHeap->Size());
		fflush(stdout);
#endif
		// we need to add this block to the cache

		// lock the request mutex
		pthread_mutex_lock(&mReqLock);
		// give insert id into the request queue
		mReqQueue.push_back(id);
		// signal the IO Req thread
		pthread_cond_signal(&mBlockReq);
		// unlock the request mutex
		pthread_mutex_unlock(&mReqLock);
	}*/
	
#ifdef _DEBUG
	//printf("}");
	//fflush(stdout);
#endif

	return ret;
}

void *LRUMemCache::IOReqThreadFunc(void *arg)
{
	// this function's purpose is to give the IO Req thread a
	// valid this pointer.
	
	// call our real thread function
	((LRUMemCache *)arg)->IOReqThread();
	// go away
	pthread_exit(NULL);
}

void *LRUMemCache::IORecvThreadFunc(void *arg)
{
	// this function's purpose is to give the IO Recv thread a
	// valid this pointer.
	
	// call our real thread function
	((LRUMemCache *)arg)->IORecvThread();
	// go away
	pthread_exit(NULL);
}

void LRUMemCache::IOReqThread()
{
	int id,num=1, req[2], recvr;
	
	while (true)
	{
		// wait until there's work to do

		// lock the request mutex
		pthread_mutex_lock(&mReqLock);
		// don't wait if the request queue has work in it
		if (mReqQueue.empty())
		{
			// wait for the signal
			pthread_cond_wait(&mBlockReq, &mReqLock);
		}
		// get the block id to fetch
		id = mReqQueue.front();
		mReqQueue.pop_front();
		// unlock the request mutex
		pthread_mutex_unlock(&mReqLock);

		// bail if the exit flag is true
		if (mExitFlag) break;
		
		// request blocks
		
		// find out who to ask
		recvr = (int)mBlockLocs[id];
		
		// do the request
		// construct
		req[0] = num; // number of blocks
		req[1] = mRank; // who's asking
		// send
		MPI_Send(req, 2, MPI_INT, recvr, PVOL_BLK_REQ1_TAG, MPI_COMM_WORLD);
		MPI_Send(&id, num, MPI_INT, recvr, PVOL_BLK_REQ2_TAG, MPI_COMM_WORLD);
	}
}

void LRUMemCache::IORecvThread()
{
	int req[2], sender;
	char *rcvBuf = NULL, *bptr;
	MPI_Status status;
	
	while (true)
	{
		// wait for block to arrive

		// get the reply
		// Part I
		MPI_Recv(req, 2, MPI_INT, MPI_ANY_SOURCE, PVOL_BLK_RPLY1_TAG,
										MPI_COMM_WORLD, &status);

#ifdef _DEBUG
		//printf("received a reply: {%d,%d}\n", req[0], req[1]);
		//fflush(stdout);
#endif

		// resize the receive buffer
		bptr = rcvBuf = (char *)realloc(rcvBuf, req[0]);
		// set the sender
		sender = req[1];

		// if sender is negative, we're being asked to quit
		if (sender < 0) break;

		// Part II
		MPI_Recv(rcvBuf, req[0], MPI_CHAR, sender, PVOL_BLK_RPLY2_TAG,
										MPI_COMM_WORLD, &status);

		
		// copy received data into the block cache
		for (int i=0; i < req[0]; i++)
		{
			int bid=0, blksize = mDatatype*1000;
			void *block = malloc(blksize); // allocate mem for a block

			// copy the block id from the buffer
			memcpy(&bid, bptr, sizeof(int));
			bptr += sizeof(int);

			// copy the block data into the newly allocated block
			memcpy(block, bptr, blksize);
			bptr += blksize;

			// lock the block cache
			pthread_mutex_lock(&mCacheLock);
			// add the new block
			mHeap->AddBlock(bid, block);
			// remove the block id from the pending fetch set
			mPendingFetches.erase(bid);
			// unlock the block cache
			pthread_mutex_unlock(&mCacheLock);
		}
	}
#ifdef _DEBUG
	printf("IO Recv thread exiting\n");
	fflush(stdout);
#endif
}

