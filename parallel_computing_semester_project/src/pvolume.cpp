/*
	pvolume.cpp
	$Id: pvolume.cpp,v 1.16 2003/05/09 16:07:11 prok Exp $

	$Log: pvolume.cpp,v $
	Revision 1.16  2003/05/09 16:07:11  prok
	bug hunting continues... currently tracking down a difficult bug in data_dist
	that is indirectly causing volren_par to fail on the SRS00002 dataset.
	
	Revision 1.15  2003/05/08 23:55:53  prok
	more tweaking in the inner kernel. trying to make the cache miss detection as
	fast as possible.
	
	Revision 1.14  2003/05/08 23:25:24  prok
	yet another performance tweak. lets see if it works.
	
	Revision 1.13  2003/05/08 23:07:10  prok
	Small performance tweaks going on in the inner rendering kernel. 50% efficiency
	sucks!
	
	Revision 1.12  2003/05/08 23:04:20  prok
	miniscule changes... everything seems to be working except rendering for the
	SRS00002 dataset. (not sure why) This commit is just to sync changes between
	prism and eyes.
	
	Revision 1.11  2003/05/08 18:34:32  prok
	gobs of bug hunting code added (but commented out). I found the segfault!
	
	Revision 1.10  2003/05/08 16:59:10  prok
	small (hopefull) performance optimization in inner kernel and some extra
	debugging printf()'s to help track down the short/float misbehaviour.
	
	Revision 1.9  2003/05/08 01:34:19  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
	Revision 1.8  2003/05/08 00:45:57  prok
	Experimental change to render on all nodes. This means that the disk cache
	process runs concurrently with the render process. I'll find out very soon
	if MPICH can handle this kind of abuse. (multiple threads calling MPI_* funcs)
	If this works, then the next step is to write a master/worker style work
	allocation to get past the current load-(im)balancing issues.
	
	Revision 1.7  2003/05/07 23:43:41  prok
	commented out some debugging specific timing code, added a render timer to the
	sequential code.
	
	Revision 1.6  2003/05/07 18:39:27  prok
	optimized the inner rendering loop to reduce calls to LRUMemCache::FetchBlock()
	
	Revision 1.5  2003/05/05 17:15:42  prok
	more debugging.
	
	Revision 1.4  2003/05/04 23:18:05  prok
	Cleanup day! Several preemptive changes made before testing begins:
	- interfe.cpp got some changes related to outputting view params so that output
	from the renderer more closely matches OpenGL output
	- cachmanager.cpp lrumemcache.* and volren_par.cpp got some code to assist in
	exiting cleanly at the end of the render
	- extra attention was paid to asynchronous I/O voodoo in lrumemcache.*.
	Specifically, I/O now happens in two threads, requests get stuffed into a queue
	before being handled, requests stay in a list until they are received. All told,
	I think this code will actually work with the way it is being called now.(lots
	of calls to FetchBlock() were a vector for ugliness)
	- fixed a tiny bug in pvolume.cpp where a ray's state could change even if it
	didn't actually complete a sample.
	- moved sequential rendering out of interfe.cpp and into volren_seq.cpp.
	
	Revision 1.3  2003/05/02 23:55:15  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
	Revision 1.2  2003/05/01 18:55:05  prok
	Added some quick changes related to the fetching of individual voxels.
		
*/

#include "pvolume.h"

PVolume::PVolume(char *cachedir, int sizelimit)
: mTfunc()
{
	// init the volume data cache
	mVolCache = new LRUMemCache(cachedir, sizelimit);

	// init our "extra work" timer
	mExtraTime = new Timer("inner loop extra work");

	// save a copy of cachedir
	mCacheDir = strdup(cachedir);

	// various dataset dependent variables
	mDatatype = 0;
	mDim[0] = mDim[1] = mDim[2] = 0;
	mBdim[0] = mBdim[1] = mBdim[2] = 0;
	mCubeDim = 10;

	mMin.Set(0.0,0.0,0.0 ,1.0);
	mMax.Set(2.0,2.0,2.0 ,1.0);
	mSpan.Set(1.0,1.0,1.0,0.0);
	mFaceNorms[0].Set(1.0,0.0,0.0 ,0.0); mFaceOffs[0] = -2.0;
	mFaceNorms[1].Set(0.0,1.0,0.0 ,0.0); mFaceOffs[1] = -2.0;
	mFaceNorms[2].Set(0.0,0.0,1.0 ,0.0); mFaceOffs[2] = -2.0;
	mFaceNorms[3].Set(-1.0,0.0,0.0 ,0.0); mFaceOffs[3] = 0.0;
	mFaceNorms[4].Set(0.0,-1.0,0.0 ,0.0); mFaceOffs[4] = 0.0;
	mFaceNorms[5].Set(0.0,0.0,-1.0 ,0.0); mFaceOffs[5] = 0.0;
}

PVolume::~PVolume()
{
	// how much time was spent doing data lookup (for the most part)
	//mExtraTime->Print();
	//fflush(stdout);
	delete mExtraTime;
	
	delete mVolCache;
	free(mCacheDir);
}

void PVolume::LoadDataset(char *filename)
{
	char fname[255];
	FILE *fp;

	unsigned int dim[3], cubedim, type, xb,yb,zb;
	float min[3],max[3],span[3],minD,maxD;

	// read the info file for the dataset
	sprintf(fname, "%s/%s.info", mCacheDir, filename);
	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		printf("unable to open dataset info file %s\n", fname);
		exit(-1);
	}

	// read the info file
	// data dimensions
	fscanf(fp, "%d %d %d\n", &dim[0], &dim[1], &dim[2]);
	// block dimensions
	fscanf(fp, "%d %d %d\n", &xb, &yb, &zb);
	// data type
	fscanf(fp, "%d\n", &type);
	// minimum coordinate (origin)
	fscanf(fp, "%f %f %f\n", &min[0], &min[1], &min[2]);
	// maximum coordinate
	fscanf(fp, "%f %f %f\n", &max[0], &max[1], &max[2]);
	// cell span
	fscanf(fp, "%f %f %f\n", &span[0], &span[1], &span[2]);
	// subvolume cube dimension
	fscanf(fp, "%d\n", &cubedim);
	// minimum "density"
	fscanf(fp, "%f\n", &minD);
	// maximum "density"
	fscanf(fp, "%f\n", &maxD);
	
	// close the info file
	fclose(fp);

#ifdef _DEBUG
	//printf("dataset dimensions: (%d,%d,%d)\n", dim[0], dim[1], dim[2]);
	//printf("block dimensions: (%d,%d,%d)\n", xb,yb,zb);
	//printf("dataset type: %d\n", type);
	//printf("min: (%f,%f,%f)\n", min[0], min[1], min[2]);
	//printf("max: (%f,%f,%f)\n", max[0], max[1], max[2]);
	//printf("span: (%f,%f,%f)\n", span[0], span[1], span[2]);
	//printf("minD: %f, maxD: %f\n", minD, maxD);
#endif

	// set all the variables dependent on data in the info file
	
	// reset the volume data cache
	mVolCache->Reset(filename, type);
	
	// min/max density for transfer function
	mTfunc.SetDensityRange(minD,maxD);
	// the volume bounding box
	set_bounding_box(min,max);
	// a voxel cell span
	mSpan.Set(span[0], span[1], span[2], 0.0);
	// the dataset's dimensions
	mDim[0] = dim[0]; mDim[1] = dim[1]; mDim[2] = dim[2];
	// dataset dimensions in blocks
	mBdim[0] = xb; mBdim[1] = yb; mBdim[2] = zb;
	// the dataset's datatype
	mDatatype = type;
	// the dimensions of a block
	mCubeDim = cubedim;
}

void PVolume::SetTransferFunction(const TransferFunction &tf)
{
	mTfunc = tf;
}

bool PVolume::CastRay(const Ray &r, ray_state *st)
{
#ifdef _DEBUG
	//printf("(");
	//fflush(stdout);
#endif
	Vector4 P;
	Vector4 cellOrig;
	int c[8][3], cell[3];
	
	int bids[8], lBids[8];
	void *blocks[8], *lBlocks[8];
	bool hasData = true;
	
	float s,t,u, c0,c1,c2,c3,c4,c5,c6,c7, tInc;
	
	float density, opacity;
	color_t color;


	// if this is a new ray, find out where to start
	if (st->t == 0.0)
	{
		// nearest point on ray where it intersects the bounding box
		st->t = nearest_intersection(r);

		// bail if the ray doesn't intersect the volume
		if (st->t < 0.0)
			return hasData; // the ray completed successfully
		else
			st->e = nearest_exit(r); // point on ray where it leaves the bounding box

#ifdef _DEBUG
		if (st->e < 0.0 || st->e < st->t)
		{
			printf("st->e looks buggered. t = %3.3f, e = %3.3f\n", st->t,st->e);
			fflush(stdout);
		}
#endif
	}

#define MIN3(x,y,z)\
			((x <= y && x <= z) ? x : (y <= x && y <= z) ? y : z)
	tInc = MIN3(mSpan.X(),mSpan.Y(),mSpan.Z()) / 3.0;
	
	// zero block related lists
	memset(blocks, 0, 8*sizeof(void *));
	memset(bids, -1, 8*sizeof(int));
	memset(lBlocks, 0, 8*sizeof(void *));
	memset(lBids, -1, 8*sizeof(int));

	// loop while we have data to work with and the opacity is
	// less than its cutoff
	while (hasData && st->o < 0.95)
	{
		// increment t
		st->t += tInc;
		
		// get the next sample point
		P = r.PointAt(st->t);
	
		// bail out immediately if the point is not within the volume bounds
		//if (!point_within_bounding_box(P))
		//{
			//printf("t = %4.2f,", st->t);
			//fflush(stdout);
			//if (st->c.r > 0.0)
			//{
			//	printf("(%2.2f,%2.2f,%2.2f) ", st->c.r,st->c.g,st->c.b);
			//	fflush(stdout);
			//}
			//return hasData;
		//}
		// a faster bail out
		if (st->t > st->e)
			return hasData;
	
		// compute the cell origin
		//cellOrig.Set(P.X()/mSpan.X()-fmod(P.X(),mSpan.X()),
		//						P.Y()/mSpan.Y()-fmod(P.Y(),mSpan.Y()),
		//						P.Z()/mSpan.Z()-fmod(P.Z(),mSpan.Z()), 1.0);
		cellOrig.Set((P.X()-fmod(P.X(),mSpan.X()))/mSpan.X(),
								(P.Y()-fmod(P.Y(),mSpan.Y()))/mSpan.Y(),
								(P.Z()-fmod(P.Z(),mSpan.Z()))/mSpan.Z(), 1.0);
		cell[0] =(int)cellOrig.X();
		cell[1] =(int)cellOrig.Y();
		cell[2] =(int)cellOrig.Z();
#ifdef _DEBUG
		//printf("P = "); P.Print();
		//printf("span = "); mSpan.Print();
		//printf("orig = "); cellOrig.Print();
		//fflush(stdout);
#endif
		
		// start the extra work timer
		//mExtraTime->Start();
		
		// first, determine the voxel coords surrounding point P
		c[0][0] = cell[0]; c[0][1] = cell[1]; c[0][2] = cell[2];
		c[1][0] = cell[0]+1; c[1][1] = cell[1]; c[1][2] = cell[2];
		c[2][0] = cell[0]; c[2][1] = cell[1]+1; c[2][2] = cell[2];
		c[3][0] = cell[0]+1; c[3][1] = cell[1]+1; c[3][2] = cell[2];
		c[4][0] = cell[0]; c[4][1] = cell[1]; c[4][2] = cell[2]+1;
		c[5][0] = cell[0]+1; c[5][1] = cell[1]; c[5][2] = cell[2]+1;
		c[6][0] = cell[0]; c[6][1] = cell[1]+1; c[6][2] = cell[2]+1;
		c[7][0] = cell[0]+1; c[7][1] = cell[1]+1; c[7][2] = cell[2]+1;
	
		// beginning of block pointer loading
		// 
		// determine which blocks the coords belong to
		for (int i=0; i < 8; i++)
		{
			int bx = c[i][0] / mCubeDim,
					by = c[i][1] / mCubeDim,
					bz = c[i][2] / mCubeDim,
					bID, bIndx, lbIndx;

			//(i*xb*yb)+(j*xb)+n
			// the block id
			bID = (bz*mBdim[0]*mBdim[1])+(by*mBdim[0])+bx;
#if 0
			// fast check
			if (bID != bids[i])
			{
				lIndx = -1;
				// look at the other block ids
				for (int n=0; n<8; n++)
					if (bID == bids[n])
					{
						lIndx = n;
						break;
					}

				if (lIndx != -1)
				{
					// we already have a pointer to this block
					bids[i] = bids[lIndx];
					blocks[i] = blocks[lIndx];
				}
				else // we have to fetch the block
				{
					// assign the (new) block id
					bids[i] = bID;
					// fetch the block
					blocks[i] = mVolCache->FetchBlock(bids[i]);
				}
			}
#else
			// fast check
			if (bID == lBids[i])
			{
				bids[i] = lBids[i];
				blocks[i] = lBlocks[i];
			}
			else // slow check
			{
				// find out if this block is already loaded
				bIndx = lbIndx = -1;
				// search all the previously loaded blocks
				for (int n=7; n>=0; n--)
				{
					if (bID == lBids[n])
					{
						lbIndx = n;
						break;
					}
					if (bID == bids[n])
					{
						bIndx = n;
						break;
					}
				}
				// load a pointer to the block
				if (lbIndx != bIndx)
				{
					// the block we need is already loaded
					if (bIndx != -1)
					{
						blocks[i] = blocks[bIndx];
						bids[i] = bids[bIndx];
					}
					else
					{
						blocks[i] = lBlocks[lbIndx];
						bids[i] = lBids[lbIndx];
					}
				}
				else // the block is not in a pointer with local scope
				{
					// pause the extra work timer
					//mExtraTime->Stop();
					
					// assign the (new) block id
					bids[i] = bID;
					// fetch the block
					blocks[i] = mVolCache->FetchBlock(bID);
	
					// unpause the timer
					//mExtraTime->Start();
				}
				
				// check the return value of FetchBlock
				//if (blocks[i] == NULL)
				//	hasData = false;
			} // end of slow check
#endif
		}
		// end of block pointer loading

		// stop the extra work timer
		//mExtraTime->Stop();

		// begin the actual sample calculation

		// only continue if all the needed blocks were resident in memory
		if (hasData)
		{
			// while it's still fresh, cache the block ptrs and ids
			memcpy(lBids, bids, 8*sizeof(int));
			memcpy(lBlocks, blocks, 8*sizeof(void *));

#ifdef _DEBUG
			//printf("(");
			//fflush(stdout);
#endif
			// load voxels surrounding the sample point
			switch(mDatatype)
			{
				case 1:
				{
					unsigned char **cblks = (unsigned char **)blocks;
					c0 = (float)cblks[0][(c[0][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[0][1]%mCubeDim)*mCubeDim
															+ (c[0][0]%mCubeDim)];
					c1 = (float)cblks[1][(c[1][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[1][1]%mCubeDim)*mCubeDim
															+ (c[1][0]%mCubeDim)];
					c2 = (float)cblks[2][(c[2][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[2][1]%mCubeDim)*mCubeDim
															+ (c[2][0]%mCubeDim)];
					c3 = (float)cblks[3][(c[3][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[3][1]%mCubeDim)*mCubeDim
															+ (c[3][0]%mCubeDim)];
					c4 = (float)cblks[4][(c[4][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[4][1]%mCubeDim)*mCubeDim
															+ (c[4][0]%mCubeDim)];
					c5 = (float)cblks[5][(c[5][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[5][1]%mCubeDim)*mCubeDim
															+ (c[5][0]%mCubeDim)];
					c6 = (float)cblks[6][(c[6][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[6][1]%mCubeDim)*mCubeDim
															+ (c[6][0]%mCubeDim)];
					c7 = (float)cblks[7][(c[7][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[7][1]%mCubeDim)*mCubeDim
															+ (c[7][0]%mCubeDim)];

					break;
				}
				case 2:
				{
					unsigned short **sblks = (unsigned short **)blocks;
					c0 = (float)sblks[0][(c[0][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[0][1]%mCubeDim)*mCubeDim
															+ (c[0][0]%mCubeDim)];
					c1 = (float)sblks[1][(c[1][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[1][1]%mCubeDim)*mCubeDim
															+ (c[1][0]%mCubeDim)];
					c2 = (float)sblks[2][(c[2][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[2][1]%mCubeDim)*mCubeDim
															+ (c[2][0]%mCubeDim)];
					c3 = (float)sblks[3][(c[3][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[3][1]%mCubeDim)*mCubeDim
															+ (c[3][0]%mCubeDim)];
					c4 = (float)sblks[4][(c[4][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[4][1]%mCubeDim)*mCubeDim
															+ (c[4][0]%mCubeDim)];
					c5 = (float)sblks[5][(c[5][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[5][1]%mCubeDim)*mCubeDim
															+ (c[5][0]%mCubeDim)];
					c6 = (float)sblks[6][(c[6][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[6][1]%mCubeDim)*mCubeDim
															+ (c[6][0]%mCubeDim)];
					c7 = (float)sblks[7][(c[7][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[7][1]%mCubeDim)*mCubeDim
															+ (c[7][0]%mCubeDim)];

					break;
				}
				case 4:
				{
					float **fblks = (float **)blocks;
					c0 = fblks[0][(c[0][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[0][1]%mCubeDim)*mCubeDim
															+ (c[0][0]%mCubeDim)];
					c1 = fblks[1][(c[1][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[1][1]%mCubeDim)*mCubeDim
															+ (c[1][0]%mCubeDim)];
					c2 = fblks[2][(c[2][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[2][1]%mCubeDim)*mCubeDim
															+ (c[2][0]%mCubeDim)];
					c3 = fblks[3][(c[3][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[3][1]%mCubeDim)*mCubeDim
															+ (c[3][0]%mCubeDim)];
					c4 = fblks[4][(c[4][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[4][1]%mCubeDim)*mCubeDim
															+ (c[4][0]%mCubeDim)];
					c5 = fblks[5][(c[5][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[5][1]%mCubeDim)*mCubeDim
															+ (c[5][0]%mCubeDim)];
					c6 = fblks[6][(c[6][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[6][1]%mCubeDim)*mCubeDim
															+ (c[6][0]%mCubeDim)];
					c7 = fblks[7][(c[7][2]%mCubeDim)*mCubeDim*mCubeDim
															+ (c[7][1]%mCubeDim)*mCubeDim
															+ (c[7][0]%mCubeDim)];

					break;
				}
				default:
					c0=c1=c2=c3=c4=c5=c6=c7=0.0; // catch errors
					break;
			}

#ifdef _DEBUG
			//printf(")");
			//fflush(stdout);
#endif
	
			// now LERP the density
	
			// LERP parameters
			s = fmod(P.X(),mSpan.X()) / mSpan.X();
			t = fmod(P.Y(),mSpan.Y()) / mSpan.Y();
			u = fmod(P.Z(),mSpan.Z()) / mSpan.Z();
	
			// density interpolation
			density = ((c0*(1.0-s) + c1*s)*(1.0-t) + (c2*(1.0-s) + c3*s)*t)*(1.0-u)
							+ ((c4*(1.0-s) + c5*s)*(1.0-t) + (c6*(1.0-s) + c7*s)*t)*u;
	
			//printf("%3.3f ", density);
			//fflush(stdout);
	
			// do the transfer function dance
			opacity = mTfunc.GetOpacity(density);
			color = mTfunc.GetColor(density);
	
			//if (opacity > 0.0)
			//{
			//	printf("%2.2f ", opacity);
			//	fflush(stdout);
			//}

			// blend color into result
			st->c = st->c + (1.0-(st->o))*opacity*color;
			// accumulate opacity
			st->o = st->o + (1.0-(st->o))*opacity;
		}
		else
		{
			//the data wasn't there... make sure st->t gets un-incremented
			st->t -= tInc;
		} // end of sample calculation
	}

#ifdef _DEBUG
	//printf("%c", hasData ? '.' : ',');
	//printf(")");
	//fflush(stdout);
#endif

	return hasData;
}

double PVolume::nearest_intersection(const Ray &r) const
{
	int currPlane=0;
	double tvals[3], result=-1.0, eps=0.001;

	// find all the points on this ray where it could conceivably
	// intersect the bounding cube of the volume
	for (int i=0; i < 6; i++)
	{
		double num, denom;
	
		// does the ray intersect our plane?
		denom = mFaceNorms[i]*r.Direction();
		// denom < 0 => ray is pointing towards plane
		if (denom < 0.0)
		{
			// find the value of t where the ray intersects this plane
			num = mFaceOffs[i] + (mFaceNorms[i]*r.Origin());
			tvals[currPlane] = -num / denom;
			currPlane++;
		}
	}

	for (int i=0; i < currPlane; i++)
	{
		Vector4 P = r.PointAt(tvals[i]);
		double x=P.X(),y=P.Y(),z=P.Z();

		if (x > mMin.X()-eps && x < mMax.X()+eps
			&& y > mMin.Y()-eps && y < mMax.Y()+eps
			&& z > mMin.Z()-eps && z < mMax.Z()+eps)
		{
			result = tvals[i];
			//printf("hit!\n");
			break;
		}
	}

	return result;
}

double PVolume::nearest_exit(const Ray &r) const
{
	int currPlane=0;
	double tvals[3], result=-1.0, eps=0.001;

	// find all the points on this ray where it could conceivably
	// intersect the bounding cube of the volume
	for (int i=0; i < 6; i++)
	{
		double num, denom;
	
		// does the ray intersect our plane?
		denom = mFaceNorms[i]*r.Direction();
		// denom > 0 => ray is pointing towards back of plane
		if (denom > 0.0)
		{
			// find the value of t where the ray intersects this plane
			num = mFaceOffs[i] + (mFaceNorms[i]*r.Origin());
			tvals[currPlane] = -num / denom;
			currPlane++;
		}
	}

	for (int i=0; i < currPlane; i++)
	{
		Vector4 P = r.PointAt(tvals[i]);
		double x=P.X(),y=P.Y(),z=P.Z();

		if (x > mMin.X()-eps && x < mMax.X()+eps
			&& y > mMin.Y()-eps && y < mMax.Y()+eps
			&& z > mMin.Z()-eps && z < mMax.Z()+eps)
		{
			result = tvals[i];
			//printf("hit!\n");
			break;
		}
	}

	return result;
}

void PVolume::set_bounding_box(float min[3], float max[3])
{
	// fill in min and max vectors
	mMin.Set(min[0],min[1],min[2],1.0);
	mMax.Set(max[0],max[1],max[2],1.0);

	for (int i=0; i<3; i++)
		mFaceOffs[i] = -max[i];

	// min should be (0.0,0.0,0.0), but just in case....
	for (int i=3; i<6; i++)
		mFaceOffs[i] = -min[i-3];
}

bool PVolume::point_within_bounding_box(const Vector4 &P) const
{
	bool result=false;
	double x=P.X(),y=P.Y(),z=P.Z(),eps=0.0;

	if (x > mMin.X()-eps && x < mMax.X()+eps
			&& y > mMin.Y()-eps && y < mMax.Y()+eps
			&& z > mMin.Z()-eps && z < mMax.Z()+eps)
		result = true;

	return result;
}

