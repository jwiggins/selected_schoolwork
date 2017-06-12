/*
	volume.h
	$Id: volume.h,v 1.5 2003/05/10 19:41:16 prok Exp $

	$Log: volume.h,v $
	Revision 1.5  2003/05/10 19:41:16  prok
	This is the final commit. The project is done. This is the tree that will be
	submitted to Prof. Browne.
	
	Revision 1.4  2003/04/25 21:43:51  prok
	Volume rendering is finally correct. Shading is turned off, because it's broken.
	Two nasty bugs were squashed in transferfunction.h. Data distribution code in
	read_data.cpp was modified to do a random distribution, rather than a modulo
	distribution.
	
	Revision 1.3  2003/04/21 04:24:15  prok
	improved rendering. works with float and char datasets so far. (should work
	just fine on short data)
	
	Revision 1.2  2003/04/20 08:14:49  prok
	
	This is the initial working version. Output is mostly correct...
	I Need to look at more transfer functions and datasets.
	
*/

#ifndef VOLUME_H
#define VOLUME_H

#include "veclib.h"
#include "ray.h"
#include "transferfunction.h"

class Volume {
public:
											Volume();
											~Volume();

	void								LoadDataset(char *filename);
	void								LoadTransferFunction(char *filename)
											{ trans.LoadVinayFile(filename); }

	void								GetBoundingBoxDimensions(float dim[3]);
	void								GetCellSpan(Vector4 &span) const 
											{ span.Set(mSpan[0],mSpan[1],mSpan[2],0.0); }
	TransferFunction	&	GetTransferFunction() { return trans; }

	bool								GetDensityAndGradient(const Vector4 &P, float &density,
															Vector4 &grad) const;
	double							NearestIntersection(const Ray &r) const;

private:
	void								ComputeBoundingBox(float min[3], float max[3]);
	bool								PointWithinBoundingBox(const Vector4 &P) const;
	void								DetermineMinMaxDensity(int size);

	char								*mDataset;
	int									mDatatype, mDatasize;
	unsigned int				mDim[3];
	Vector4							mFaceNorms[6],mMin,mMax;
	double							mFaceOffs[6];
	float								mSpan[3];
	TransferFunction 		trans;
	union {
		float mfMinDens;
		unsigned short msMinDens;
		unsigned char mcMinDens;
	};
	union {
		float mfMaxDens;
		unsigned short msMaxDens;
		unsigned char mcMaxDens;
	};
};

#endif

