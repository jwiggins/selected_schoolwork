/*
	renderslave.h
	$Id: renderslave.h,v 1.7 2003/05/08 01:34:20 prok Exp $

	$Log: renderslave.h,v $
	Revision 1.7  2003/05/08 01:34:20  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
	Revision 1.6  2003/05/08 00:45:58  prok
	Experimental change to render on all nodes. This means that the disk cache
	process runs concurrently with the render process. I'll find out very soon
	if MPICH can handle this kind of abuse. (multiple threads calling MPI_* funcs)
	If this works, then the next step is to write a master/worker style work
	allocation to get past the current load-(im)balancing issues.
	
	Revision 1.5  2003/05/07 16:13:47  prok
	fixed timing code. added more timers.
	
	Revision 1.4  2003/05/07 04:53:44  prok
	Added timing code.
	
	Revision 1.3  2003/05/02 23:55:15  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
*/

#ifndef RENDER_SLAVE_H
#define RENDER_SLAVE_H

#include <list>
#include <stdlib.h>
#include <mpi.h>
#include "myrandom.h"
#include "mpilib.h"
#include "ray.h"
#include "camera.h"
#include "transferfunction.h"
#include "pvolume.h"
#include "timer.h"

using namespace std;

class RenderSlave {
public:
													RenderSlave(int r_node_strt, char *cachedir, int sizelim);
													~RenderSlave();

	void										LoadDataset(char *file);
	void										LoadTransferFunc(char *file);

	void										SetOutputSize(int w, int h);
	void										SetPatchSize(int s);
	void										SetViewParams(const Vector4 &eye, const Vector4 &lookat,
																				const Vector4 &up);
	
	void										Run();
	void										WriteImageFile(char *file);

private:
	void										RenderPatch(int x1, int y1, int x2, int y2);

	PVolume									*mVolume;
	Camera									*mCamera;
	Timer										*mRenderTime, *mMergeTime; 

	unsigned char						*mFrameBuf;
	int											mWidth, mHeight;

	int											mRendNodeStart;
	int											mPatchDim;

};

#endif

