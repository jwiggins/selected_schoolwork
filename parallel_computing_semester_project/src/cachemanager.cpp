/*
	cachemanager.cpp
	$Id: cachemanager.cpp,v 1.9 2003/05/09 16:07:11 prok Exp $

	$Log: cachemanager.cpp,v $
	Revision 1.9  2003/05/09 16:07:11  prok
	bug hunting continues... currently tracking down a difficult bug in data_dist
	that is indirectly causing volren_par to fail on the SRS00002 dataset.
	
	Revision 1.8  2003/05/08 23:04:20  prok
	miniscule changes... everything seems to be working except rendering for the
	SRS00002 dataset. (not sure why) This commit is just to sync changes between
	prism and eyes.
	
	Revision 1.7  2003/05/08 01:34:19  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
*/

#include "cachemanager.h"

WageSlave::WageSlave(char *cachedir)
{
	mCache = new LRUDiskCache(cachedir);

	// determine who you are
	MPI_Comm_rank(MPI_COMM_WORLD, &mRank);
}

WageSlave::~WageSlave()
{
	delete mCache;
}

void WageSlave::Run()
{
	int req[2], blk, recvr, bsize;
	char *sndBuf = NULL, *bptr;
	MPI_Status status;

#ifdef _DEBUG
	//printf("WageSlave running on node %d\n", mRank);
	//fflush(stdout);
#endif

	while (true)
	{
		// wait for a request to come over the wire
		MPI_Recv(req, 2, MPI_INT, MPI_ANY_SOURCE, PVOL_BLK_REQ1_TAG,
							MPI_COMM_WORLD, &status);

		// req[0] is the block id being requested
		// req[1] is the rank of the requesting process
		blk = req[0];
		recvr = req[1];

		// if the receiver field is negative, it is a quit request
		if (recvr < 0)
		{
			// clean up memory
			//realloc(blks, 0);
			realloc(sndBuf, 0);
			// exit
			break;
		}

		// start by allocating space for the request
		//blks = (int *)realloc(blks, req[0]*sizeof(int));
		// then finish receiving the request
		//MPI_Recv(blks, req[0], MPI_INT, recvr, PVOL_BLK_REQ2_TAG,
		//					MPI_COMM_WORLD, &status);

		// process the request
		// first, allocate the send buffer
		bsize = mCache->BlockSize();
		bptr = sndBuf = (char *)realloc(sndBuf, bsize);
		// then, fill the buffer
		//for (int i=0; i < req[0]; i++)
		//{
			// copy the block id into the buffer
			//memcpy(bptr, &blks[i], sizeof(int));
			//memcpy(bptr, &blk, sizeof(int));
			//bptr += sizeof(int);
			// copy the block data into the buffer
			//memcpy(bptr, mCache->FetchBlock(blks[i]), mCache->BlockSize());
			memcpy(bptr, mCache->FetchBlock(blk), mCache->BlockSize());
			//bptr += mCache->BlockSize();
		//}

		// reply
		// req[0] is the size of the data being sent (set to bsize)
		// req[1] is the rank of the sending process (set to mRank)
		//req[0] = bsize;
		//req[1] = mRank;

		// part I
		//MPI_Send(req, 2, MPI_INT, recvr, PVOL_BLK_RPLY1_TAG, MPI_COMM_WORLD);
		// part II
		MPI_Send(sndBuf, bsize, MPI_CHAR, recvr, PVOL_BLK_RPLY2_TAG, MPI_COMM_WORLD);

		// loop and do it all over again!
	}
}

