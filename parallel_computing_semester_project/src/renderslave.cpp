/*
	renderslave.cpp
	$Id: renderslave.cpp,v 1.14 2003/05/08 23:25:24 prok Exp $

	$Log: renderslave.cpp,v $
	Revision 1.14  2003/05/08 23:25:24  prok
	yet another performance tweak. lets see if it works.
	
	Revision 1.13  2003/05/08 23:04:20  prok
	miniscule changes... everything seems to be working except rendering for the
	SRS00002 dataset. (not sure why) This commit is just to sync changes between
	prism and eyes.
	
	Revision 1.12  2003/05/08 18:34:32  prok
	gobs of bug hunting code added (but commented out). I found the segfault!
	
	Revision 1.11  2003/05/08 02:44:50  prok
	Revamped load balancing. Made one of the render nodes a dedicated load balancer.
	Now instead of render nodes picking the patches they will render and then
	rendering them, they will contact the load balancer for a work unit of 5 or
	fewer patches. This _should_ resolve the massive load imbalance in the previous
	version.
	
	Revision 1.10  2003/05/08 01:34:20  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
	Revision 1.9  2003/05/08 00:45:58  prok
	Experimental change to render on all nodes. This means that the disk cache
	process runs concurrently with the render process. I'll find out very soon
	if MPICH can handle this kind of abuse. (multiple threads calling MPI_* funcs)
	If this works, then the next step is to write a master/worker style work
	allocation to get past the current load-(im)balancing issues.
	
	Revision 1.8  2003/05/07 04:53:44  prok
	Added timing code.
	
	Revision 1.7  2003/05/05 17:39:24  prok
	Black output bug found and hopefully squished. (now for new bugs! :)
	
	Revision 1.6  2003/05/05 17:27:08  prok
	added a histogram computation to renderslave for debugging purposes. Made a
	small fix in lrumemcache.h to use the std namespace.
	
	Revision 1.5  2003/05/05 17:15:42  prok
	more debugging.
	
	Revision 1.4  2003/05/05 03:38:55  prok
	debugging printf()'s added.
	
	Revision 1.3  2003/05/04 23:51:35  prok
	Small debugging related changes.
	
	Revision 1.2  2003/05/02 23:55:15  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
*/

#include "renderslave.h"

RenderSlave::RenderSlave(int r_node_strt, char *cachedir, int sizelimit)
{
	// init the volume
	mVolume = new PVolume(cachedir, sizelimit);

	// init the camera
	mCamera = NULL;
	
	// init the frame buffer
	mFrameBuf = NULL;
	mHeight = 0;
	mWidth = 0;

	// some parallel related values
	mRendNodeStart = r_node_strt; // the rank of the first render slave
	mPatchDim = 10;

	// timers
	mRenderTime = new Timer("node render time");
	mMergeTime = new Timer("node merge time");
}

RenderSlave::~RenderSlave()
{
	mRenderTime->Print();
	mMergeTime->Print();
	fflush(stdout);

	delete mRenderTime;
	delete mMergeTime;
	
	delete mVolume;
	if (mCamera)
		delete mCamera;
	if (mFrameBuf)
		free(mFrameBuf);
}

void RenderSlave::LoadDataset(char *file)
{
	// pass it on
	mVolume->LoadDataset(file);
}

void RenderSlave::LoadTransferFunc(char *file)
{
	TransferFunction tf;

	// load the transfer function
	tf.LoadVinayFile(file);

	// set the transfer function on the volume
	mVolume->SetTransferFunction(tf);
}

void RenderSlave::SetOutputSize(int w, int h)
{
	// set the new width and height
	mWidth = w;
	mHeight = h;

	// allocate a new frame buffer
	mFrameBuf = (unsigned char*)realloc(mFrameBuf, mWidth*mHeight*3);
}

void RenderSlave::SetPatchSize(int s)
{
	mPatchDim = s;
}

void RenderSlave::SetViewParams(const Vector4 &eye, const Vector4 &lookat,
																const Vector4 &up)
{
	// out with the old
	if (mCamera)
		delete mCamera;
	
	// in with the new
	mCamera = new Camera(eye,lookat,up,60.0,mWidth/(double)mHeight,
									(double)mWidth,(double)mHeight);
}

void RenderSlave::Run()
{
	int rank, size;
	int xPatches, yPatches, rseed;
	//list<int> patches;

	// get some MPI info to start
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

#ifdef _DEBUG
	//printf("RenderSlave running on node %d\n", rank);
	//fflush(stdout);
#endif
	
	// what are the dimensions of the output in patches?
	xPatches = (mWidth / mPatchDim) + ((mWidth%mPatchDim)?1:0); 
	yPatches = (mHeight / mPatchDim) + ((mHeight%mPatchDim)?1:0); 

	//
	// collectively determine which patches to render
	
#if 0	
	// seed the rng
	if (rank == mRendNodeStart)
		rseed = time(NULL);
	// limited broadcast
	my_bcast(&rseed, 1, MPI_INT, mRendNodeStart, mRendNodeStart, size-1,
									MPI_COMM_WORLD);
	// seed on all nodes
	mysrandom(rseed);
	
	// dole out the patches
	for (int i=0; i < xPatches*yPatches; i++)
	{
		int p = myrandom(mRendNodeStart, size-1);

		// if p equals our rank, we must render this patch
		if (rank == p)
			patches.push_back(i);
	}
#endif
	
#ifdef _DEBUG
	//printf("RenderSlave %d starting to render patches\n", rank);
	//fflush(stdout);
#endif

	//
	// render the patches

	// start the render timer	
	mRenderTime->Start();

	while (true)
	{
		int patches[5], num=0;
		MPI_Status status;

		// get some patches from the load balancer
		MPI_Send(&rank, 1, MPI_INT, mRendNodeStart, PVOL_PTCH_REQ_TAG,
						MPI_COMM_WORLD);
		MPI_Recv(&num, 1, MPI_INT, mRendNodeStart, PVOL_PTCH_RPLY1_TAG,
						MPI_COMM_WORLD, &status);

		// no more work to do!
		if (num == -1)
			break;
		
		MPI_Recv(patches, num, MPI_INT, mRendNodeStart, PVOL_PTCH_RPLY2_TAG,
						MPI_COMM_WORLD, &status);

		for (int i=0; i < num; i++)
		{
			int x1,y1,x2,y2;

			// compute the corners of the patch
			x1 = (patches[i] % xPatches) * mPatchDim;
			y1 = (patches[i] / xPatches) * mPatchDim;
			x2 = x1+mPatchDim; x2 -= (x2>mWidth)?x2-mWidth:0;
			y2 = y1+mPatchDim; y2 -= (y2>mHeight)?y2-mHeight:0;
		
			// render the patch
			RenderPatch(x1,y1, x2,y2);
		}
	}

#if 0
	// loop until all the patches are gone
	while (!patches.empty())
	{
		int p, x1,y1,x2,y2;
	 	
		// grab the first patch in the list
		p	= patches.front();
		patches.pop_front();

		// compute the corners of the patch
		x1 = (p % xPatches) * mPatchDim;
		y1 = (p / xPatches) * mPatchDim;
		x2 = x1+mPatchDim; x2 -= (x2>mWidth)?x2-mWidth:0;
		y2 = y1+mPatchDim; y2 -= (y2>mHeight)?y2-mHeight:0;
		
		// render the patch
		RenderPatch(x1,y1, x2, y2);
#ifdef _DEBUG
		//printf("node %d rendered (%d,%d)->(%d,%d)\n", mRank, x1,y1,x2,y2);
		//fflush(stdout);
#endif
	}
#endif

	// stop the render timer
	mRenderTime->Stop();

#ifdef _DEBUG
	//printf("RenderSlave %d done rendering!\n", rank);
	//fflush(stdout);
#endif

	//int hist[256];
	//memset(hist, 0, sizeof(int)*256);
	//for (int i=0; i<mWidth*mHeight; i++)
	//{
	//	int avg=0;
	//	avg += mFrameBuf[i+0];
	//	avg += mFrameBuf[i+1];
	//	avg += mFrameBuf[i+2];
	//	avg /= 255;
	//	hist[avg]++;
	//}
	//for (int i=0; i<256; i++)
	//{
	//	printf("%d ", hist[i]);
	//	if ((i % 64) == 0)
	//		printf("\n");
	//}
	//fflush(stdout);

	//MPI_Barrier(MPI_COMM_WORLD);

	// start the merge timer
	mMergeTime->Start();

	//
	// merge outputs
	//logNMergeImage(mFrameBuf, mWidth*mHeight*3, mRendNodeStart, mRendNodeStart,
	//							size-1, MPI_COMM_WORLD);
	logNMergeImage(mFrameBuf, mWidth*mHeight*3, mRendNodeStart+1, mRendNodeStart+1,
								size-1, MPI_COMM_WORLD);

	// stop the merge timer
	mMergeTime->Stop();
}

void RenderSlave::WriteImageFile(char *file)
{
	FILE *fp;

	fp = fopen(file, "w");
	if (fp == NULL)
		printf("could not write image to file %s\n", file);
	else
	{
		// write the frame buffer as a ppm file
		fprintf(fp, "P6\n%d %d\n255\n", mWidth, mHeight);
		fwrite(mFrameBuf, mWidth*mHeight*3, 1, fp);
		// close the file
		fclose(fp);
	}
}

void RenderSlave::RenderPatch(int x1, int y1, int x2, int y2)
{
	struct ray_bundle {
		Ray				r;
		ray_state s;
		int				x,y;
	};
	
	list<void *> rays;
	//int rank;
	//bool hasColor=false;

	//MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// generate the list of rays
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			ray_bundle *b = new ray_bundle;
		 
			// generate the ray
			b->r	= mCamera->RayForPixel(x,y);

			// zero the ray state
			b->s.t = 0.0;
			b->s.e = 0.0;
			b->s.o = 0.0;
			b->s.c.r = b->s.c.g = b->s.c.b = 0.0;

			// give it some coords
			b->x = x;
			b->y = y;
			
			// add it to the list
			rays.push_back(b);
		}
	}

	// loop until there are no more rays
	while (!rays.empty())
	{
#ifdef _DEBUG
		//printf("[");
		//fflush(stdout);
#endif
		bool ray_finished;
		ray_bundle *b;

		// pull a ray off the front of the list
		b	= (ray_bundle *)rays.front();
		rays.pop_front();

		// cast the ray
		ray_finished = mVolume->CastRay(b->r, &(b->s));

		// if the ray finished, write its color to the frame buffer
		if (ray_finished)
		{
			// write the color
			mFrameBuf[(b->y*mWidth+b->x)*3+0] = (unsigned char)(b->s.c.r*255.0);
			mFrameBuf[(b->y*mWidth+b->x)*3+1] = (unsigned char)(b->s.c.g*255.0);
			mFrameBuf[(b->y*mWidth+b->x)*3+2] = (unsigned char)(b->s.c.b*255.0);

#ifdef _DEBUG
			//if(rank == 8)
			//{
			//	printf("%02x%02x%02x ", mFrameBuf[(b->y*mWidth+b->x)*3+0],
			//	mFrameBuf[(b->y*mWidth+b->x)*3+1],
			//	mFrameBuf[(b->y*mWidth+b->x)*3+2]);
			//	fflush(stdout);
			//}
#endif
			
			//if (!hasColor && 
			//	(b->s.c.r > 0.0 || b->s.c.g > 0.0 || b->s.c.b > 0.0))
			//	hasColor = true;

			// free the bundle
			delete b;
		}
		else // otherwise, put it back in the list
			rays.push_back(b);

#ifdef _DEBUG
		//printf("]");
		//fflush(stdout);
#endif
	}

#ifdef _DEBUG
	//if (hasColor)
	//{
	//	printf("(%d,%d)->(%d,%d) ", x1,y1,x2,y2);
	//	fflush(stdout);
	//}
#endif
}

