/*
	pvolume.h
	$Id: pvolume.h,v 1.7 2003/05/08 23:25:24 prok Exp $

	$Log: pvolume.h,v $
	Revision 1.7  2003/05/08 23:25:24  prok
	yet another performance tweak. lets see if it works.
	
	Revision 1.6  2003/05/08 01:34:19  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
	Revision 1.5  2003/05/08 00:45:57  prok
	Experimental change to render on all nodes. This means that the disk cache
	process runs concurrently with the render process. I'll find out very soon
	if MPICH can handle this kind of abuse. (multiple threads calling MPI_* funcs)
	If this works, then the next step is to write a master/worker style work
	allocation to get past the current load-(im)balancing issues.
	
	Revision 1.4  2003/05/07 23:43:41  prok
	commented out some debugging specific timing code, added a render timer to the
	sequential code.
	
	Revision 1.3  2003/05/02 23:55:15  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
*/

#ifndef PVOLUME_H
#define PVOLUME_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "veclib.h"
#include "ray.h"
#include "transferfunction.h"
#include "lrumemcache.h"
#include "timer.h"

struct ray_state {
	double	t, e;
	float o;
	color_t	c;
};

class PVolume {
public:
														PVolume(char *cachedir, int sizelimit);
														~PVolume();

	void											LoadDataset(char *filename);
	void											SetTransferFunction(const TransferFunction &tf);

	bool											CastRay(const Ray &r, ray_state *s);

private:
	
	double										nearest_intersection(const Ray &r) const;
	double										nearest_exit(const Ray &r) const;
	bool											point_within_bounding_box(const Vector4 &P) const;
	void											set_bounding_box(float min[3], float max[3]);

	TransferFunction					mTfunc;
	LRUMemCache								*mVolCache;
	Timer											*mExtraTime;

	char											*mCacheDir;

	int												mDatatype;
	unsigned int							mDim[3], mBdim[3], mCubeDim;
	Vector4										mFaceNorms[6],mMin,mMax,mSpan;
	double										mFaceOffs[6];
};

#endif

