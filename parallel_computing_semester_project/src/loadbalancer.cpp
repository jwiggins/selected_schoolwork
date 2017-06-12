/*
	loadbalancer.cpp
	$Id: loadbalancer.cpp,v 1.2 2003/05/08 23:04:20 prok Exp $

	$Log: loadbalancer.cpp,v $
	Revision 1.2  2003/05/08 23:04:20  prok
	miniscule changes... everything seems to be working except rendering for the
	SRS00002 dataset. (not sure why) This commit is just to sync changes between
	prism and eyes.
	
	Revision 1.1  2003/05/08 02:44:49  prok
	Revamped load balancing. Made one of the render nodes a dedicated load balancer.
	Now instead of render nodes picking the patches they will render and then
	rendering them, they will contact the load balancer for a work unit of 5 or
	fewer patches. This _should_ resolve the massive load imbalance in the previous
	version.
	
*/

#include "loadbalancer.h"

LoadBalancer::LoadBalancer(int w, int h, int p)
{
	int xDim, yDim;
	
	// what are the dimensions of the output in patches?
	xDim = (w / p) + ((w%p)?1:0); 
	yDim = (h / p) + ((h%p)?1:0);
	
	mNumPatches = xDim*yDim;
	mPatchesRemaining = mNumPatches;
	
	// allocate enough space for the patch work buffer
	mPatches = (bool *)malloc(mNumPatches*sizeof(bool));
	memset(mPatches, false, mNumPatches*sizeof(bool));
	
	// seed the rng
	mysrandom(time(NULL));
}

LoadBalancer::~LoadBalancer()
{
	// clean up
	free(mPatches);
}

void LoadBalancer::Run()
{
	MPI_Status status;
	int requester, patch, allocated=0, msg[5];

	while(true)
	{
		// get a work request
		MPI_Recv(&requester, 1, MPI_INT, MPI_ANY_SOURCE, PVOL_PTCH_REQ_TAG,
								MPI_COMM_WORLD, &status);

		// a quit message
		if (requester == -1)
			break;

		if (mPatchesRemaining != 0)
		{
			// get some patches
			allocated = 0;
			while (allocated < 3 && mPatchesRemaining != 0)
			{
				// get a patch
				do
				{
					patch = myrandom(0, mNumPatches-1);
				} while (mPatches[patch]);
				
				// mark it
				mPatches[patch] = true;
	
				// add it to the msg buffer
				msg[allocated] = patch;
				
				// twiddle counters
				allocated++;
				mPatchesRemaining--;
			}
	
			// send the reply
			MPI_Send(&allocated, 1, MPI_INT, requester, PVOL_PTCH_RPLY1_TAG,
							MPI_COMM_WORLD);
			MPI_Send(msg, allocated, MPI_INT, requester, PVOL_PTCH_RPLY2_TAG,
							MPI_COMM_WORLD);
		}
		else
		{
			// ain't no patches left
			allocated = -1;
			MPI_Send(&allocated, 1, MPI_INT, requester, PVOL_PTCH_RPLY1_TAG,
							MPI_COMM_WORLD);
		}
	}
}

