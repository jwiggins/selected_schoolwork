/*
	volume.cpp
	$Id: volume.cpp,v 1.5 2003/05/10 19:41:16 prok Exp $

	$Log: volume.cpp,v $
	Revision 1.5  2003/05/10 19:41:16  prok
	This is the final commit. The project is done. This is the tree that will be
	submitted to Prof. Browne.
	
	Revision 1.4  2003/04/25 21:43:48  prok
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

#include "volume.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

union f_swap {
	float value;
	char bytes[4];
};

union ui_swap {
	unsigned int value;
	char bytes[4];
};


void parse_rawiv_header(char *buffer,float *min, float *max, unsigned int *numverts,
					unsigned int *numcells, unsigned int *dim, float *origin, float *span);

// assumes little endian is the native order!!!
void swap_big_to_little_uint(unsigned int &val, char *buffer);
void swap_little_to_big_uint(unsigned int &val, char *buffer);
void swap_big_to_little_float(float &val, char *buffer);
void swap_little_to_big_float(float &val, char *buffer);
void swap_buffer(char *buffer, int size, int type);

Volume::Volume()
{
	mDataset = NULL;
	mDatatype = 0;
	mDim[0] = mDim[1] = mDim[2] = 0;

	mMin.Set(0.0,0.0,0.0 ,1.0);
	mMax.Set(2.0,2.0,2.0 ,1.0);
	mFaceNorms[0].Set(1.0,0.0,0.0 ,0.0); mFaceOffs[0] = -2.0;
	mFaceNorms[1].Set(0.0,1.0,0.0 ,0.0); mFaceOffs[1] = -2.0;
	mFaceNorms[2].Set(0.0,0.0,1.0 ,0.0); mFaceOffs[2] = -2.0;
	mFaceNorms[3].Set(-1.0,0.0,0.0 ,0.0); mFaceOffs[3] = 0.0;
	mFaceNorms[4].Set(0.0,-1.0,0.0 ,0.0); mFaceOffs[4] = 0.0;
	mFaceNorms[5].Set(0.0,0.0,-1.0 ,0.0); mFaceOffs[5] = 0.0;
}

Volume::~Volume()
{
	if (mDataset != NULL)
		free(mDataset);
}

void Volume::LoadDataset(char *filename)
{
	char header[68];
	float min[3], max[3], origin[3];
	unsigned int numverts, numcells;
	int xb,yb,zb;
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("could not open file %s\n", filename);
		exit(-1);
	}

	// seek to the end of the file
	fseek(fp, 0L, SEEK_END);
	// get the size of the data (filesize less 68 byte header)
	mDatasize = ftell(fp) - 68;
	// seek back to the beginning
	fseek(fp, 0L, SEEK_SET);

	// read the rawiv header
	fread(header, sizeof(header), 1, fp);
	// parse the header
	parse_rawiv_header(header, min,max,&numverts,&numcells,mDim,origin,mSpan);
	// determine the datatype of the voxels
	mDatatype = (mDatasize / numverts);

	// recompute the volume bounding box
	ComputeBoundingBox(min,max);

	// allocate space for the dataset (HUGE POINT OF FAILURE)
	mDataset = (char *)malloc(mDatasize);
	// read the dataset into memory
	fread(mDataset, mDatasize, 1, fp);
	// swap the data big -> little
	swap_buffer(mDataset, mDatasize, mDatatype);
	// find min/max values
	DetermineMinMaxDensity(mDatasize);

	switch(mDatatype)
	{
		case 1:
			trans.SetDensityRange((float)mcMinDens, (float)mcMaxDens);
			printf("min/max density: (%d,%d)\n", mcMinDens, mcMaxDens);
			break;
		case 2:
			trans.SetDensityRange((float)msMinDens, (float)msMaxDens);
			printf("min/max density: (%d,%d)\n", msMinDens, msMaxDens);
			break;
		case 4:
			trans.SetDensityRange(mfMinDens, mfMaxDens);
			printf("min/max density: (%2.2f,%2.2f)\n", mfMinDens, mfMaxDens);
			break;
		default:
			break;
	}

	printf("datasize = %d bytes (%fMB)\n", mDatasize, mDatasize/(1024.0*1024.0));
	printf("data type = %s\n", (mDatatype==1)?"char":((mDatatype==2)?"short":"float"));
	printf("min (%f,%f,%f)\n", min[0], min[1], min[2]);
	printf("max (%f,%f,%f)\n", max[0], max[1], max[2]);
	printf("numverts = %d\n", numverts);
	printf("numcells = %d\n", numcells);
	printf("dim (%d,%d,%d)\n", mDim[0], mDim[1], mDim[2]);
	printf("origin (%f,%f,%f)\n", origin[0], origin[1], origin[2]);
	printf("span (%f,%f,%f)\n", mSpan[0], mSpan[1], mSpan[2]);
}

void Volume::GetBoundingBoxDimensions(float dims[3])
{
	dims[0] = mMax.X();
	dims[1] = mMax.Y();
	dims[2] = mMax.Z();
}

/*
	Precondition:
	P is a point within the volume's bounding box, therefore it
	lies within a cell of the volume. (ie it is surrounded by 8
	voxels.
*/
bool Volume::GetDensityAndGradient(const Vector4 &P, float &density,
													Vector4 &grad) const
{
	Vector4 cellOrig;//,grads[8];
	int c[8][3], cell[3];
	// an assortment of pointers to the dataset
	unsigned char *cds = (unsigned char *)mDataset;
	unsigned short *sds = (unsigned short *)mDataset;
	float *fds = (float *)mDataset;	

	// bail out immediately if the point is not within the volume bounds
	if (!PointWithinBoundingBox(P))
		return false;

	// compute the cell origin
	cellOrig.Set(P.X()/mSpan[0]-fmod(P.X(),mSpan[0]),
							P.Y()/mSpan[1]-fmod(P.Y(),mSpan[1]),
							P.Z()/mSpan[2]-fmod(P.Z(),mSpan[2]), 1.0);
	cell[0] =(int)cellOrig.X();
	cell[1] =(int)cellOrig.Y();
	cell[2] =(int)cellOrig.Z();
	
	// first, determine the voxel coords surrounding point P
	c[0][0] = cell[0]; c[0][1] = cell[1]; c[0][2] = cell[2];
	c[1][0] = cell[0]+1; c[1][1] = cell[1]; c[1][2] = cell[2];
	c[2][0] = cell[0]; c[2][1] = cell[1]+1; c[2][2] = cell[2];
	c[3][0] = cell[0]+1; c[3][1] = cell[1]+1; c[3][2] = cell[2];
	c[4][0] = cell[0]; c[4][1] = cell[1]; c[4][2] = cell[2]+1;
	c[5][0] = cell[0]+1; c[5][1] = cell[1]; c[5][2] = cell[2]+1;
	c[6][0] = cell[0]; c[6][1] = cell[1]+1; c[6][2] = cell[2]+1;
	c[7][0] = cell[0]+1; c[7][1] = cell[1]+1; c[7][2] = cell[2]+1;

	// compute the gradients at each corner of the cell
	/*for (int i=0; i<8; i++)
	{
		double ddx=mSpan[0]*2.0,ddy=mSpan[1]*2.0,ddz=mSpan[2]*2.0;
		int idx1,idx2,idy1,idy2,idz1,idz2;
		float dx1,dx2,dy1,dy2,dz1,dz2;

		// find out what data to load
		// x direction
		if (c[i][0] != 0)
			idx1 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0] - 1;
		else
		{
			ddx = mSpan[0];
			idx1 = (c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0]);
		}
		if (c[i][0] < mDim[0])
			idx2 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0] + 1;
		else
		{
			ddx = mSpan[0];
			idx2 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		}
		
		// y direction
		if (c[i][1] != 0)
			idy1 = c[i][2]*mDim[0]*mDim[1] + (c[i][1] - 1)*mDim[0] + c[i][0];
		else
		{
			ddy = mSpan[1];
			idy1 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		}
		if (c[i][1] < mDim[1])
			idy2 = c[i][2]*mDim[0]*mDim[1] + (c[i][1] + 1)*mDim[0] + c[i][0];
		else
		{
			ddy = mSpan[1];
			idy2 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		}
		
		// z direction
		if (c[i][2] != 0)
			idz1 = (c[i][2] - 1)*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		else
		{
			ddz = mSpan[2];
			idz1 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		}
		if (c[i][2] < mDim[2])
			idz2 = (c[i][2] + 1)*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		else
		{
			ddz = mSpan[2];
			idz2 = c[i][2]*mDim[0]*mDim[1] + c[i][1]*mDim[0] + c[i][0];
		}


		// load the surrounding voxels
		switch(mDatatype)
		{
			case 1:
			{
				dx1 = (float)cds[idx1];///255.0;
				dx2 = (float)cds[idx2];///255.0;
				dy1 = (float)cds[idy1];///255.0;
				dy2 = (float)cds[idy2];///255.0;
				dz1 = (float)cds[idz1];///255.0;
				dz2 = (float)cds[idz2];///255.0;
				break;
			}
			case 2:
			{
				dx1 = (float)sds[idx1];///65535.0;
				dx2 = (float)sds[idx2];///65535.0;
				dy1 = (float)sds[idy1];///65535.0;
				dy2 = (float)sds[idy2];///65535.0;
				dz1 = (float)sds[idz1];///65535.0;
				dz2 = (float)sds[idz2];///65535.0;
				break;
			}
			case 4:
			{
				dx1 = fds[idx1];
				dx2 = fds[idx2];
				dy1 = fds[idy1];
				dy2 = fds[idy2];
				dz1 = fds[idz1];
				dz2 = fds[idz2];
				break;
			}
			default:
				break;
		}

		// compute the gradient for this voxel
		grads[i].Set((dx1-dx2)/ddx, (dy1-dy2)/ddy, (dz1-dz2)/ddz, 0.0);
		//grads[i].Normalize();
	}*/

	// now LERP the density and gradient
	float s,t,u,
				c0,c1,c2,c3,c4,c5,c6,c7,
				gx,gy,gz;

	// LERP parameters
	s = fmod(P.X(),mSpan[0]) / mSpan[0];
	t = fmod(P.Y(),mSpan[1]) / mSpan[1];
	u = fmod(P.Z(),mSpan[2]) / mSpan[2];

	// load voxels surrounding the sample point
	switch(mDatatype)
	{
		case 1:
		{
			c0 = (float)cds[(c[0][2]*mDim[0]*mDim[1] + c[0][1]*mDim[0] + c[0][0])];
			c1 = (float)cds[(c[1][2]*mDim[0]*mDim[1] + c[1][1]*mDim[0] + c[1][0])];
			c2 = (float)cds[(c[2][2]*mDim[0]*mDim[1] + c[2][1]*mDim[0] + c[2][0])];
			c3 = (float)cds[(c[3][2]*mDim[0]*mDim[1] + c[3][1]*mDim[0] + c[3][0])];
			c4 = (float)cds[(c[4][2]*mDim[0]*mDim[1] + c[4][1]*mDim[0] + c[4][0])];
			c5 = (float)cds[(c[5][2]*mDim[0]*mDim[1] + c[5][1]*mDim[0] + c[5][0])];
			c6 = (float)cds[(c[6][2]*mDim[0]*mDim[1] + c[6][1]*mDim[0] + c[6][0])];
			c7 = (float)cds[(c[7][2]*mDim[0]*mDim[1] + c[7][1]*mDim[0] + c[7][0])];
			break;
		}
		case 2:
		{
			c0 = (float)sds[(c[0][2]*mDim[0]*mDim[1] + c[0][1]*mDim[0] + c[0][0])];
			c1 = (float)sds[(c[1][2]*mDim[0]*mDim[1] + c[1][1]*mDim[0] + c[1][0])];
			c2 = (float)sds[(c[2][2]*mDim[0]*mDim[1] + c[2][1]*mDim[0] + c[2][0])];
			c3 = (float)sds[(c[3][2]*mDim[0]*mDim[1] + c[3][1]*mDim[0] + c[3][0])];
			c4 = (float)sds[(c[4][2]*mDim[0]*mDim[1] + c[4][1]*mDim[0] + c[4][0])];
			c5 = (float)sds[(c[5][2]*mDim[0]*mDim[1] + c[5][1]*mDim[0] + c[5][0])];
			c6 = (float)sds[(c[6][2]*mDim[0]*mDim[1] + c[6][1]*mDim[0] + c[6][0])];
			c7 = (float)sds[(c[7][2]*mDim[0]*mDim[1] + c[7][1]*mDim[0] + c[7][0])];
			break;
		}
		case 4:
		{
			c0 = fds[(c[0][2]*mDim[0]*mDim[1] + c[0][1]*mDim[0] + c[0][0])];
			c1 = fds[(c[1][2]*mDim[0]*mDim[1] + c[1][1]*mDim[0] + c[1][0])];
			c2 = fds[(c[2][2]*mDim[0]*mDim[1] + c[2][1]*mDim[0] + c[2][0])];
			c3 = fds[(c[3][2]*mDim[0]*mDim[1] + c[3][1]*mDim[0] + c[3][0])];
			c4 = fds[(c[4][2]*mDim[0]*mDim[1] + c[4][1]*mDim[0] + c[4][0])];
			c5 = fds[(c[5][2]*mDim[0]*mDim[1] + c[5][1]*mDim[0] + c[5][0])];
			c6 = fds[(c[6][2]*mDim[0]*mDim[1] + c[6][1]*mDim[0] + c[6][0])];
			c7 = fds[(c[7][2]*mDim[0]*mDim[1] + c[7][1]*mDim[0] + c[7][0])];
			break;
		}
		default:
			c0=c1=c2=c3=c4=c5=c6=c7=0.0; // catch errors
			break;
	}

	// density interpolation
	density = ((c0*(1.0-s) + c1*s)*(1.0-t) + (c2*(1.0-s) + c3*s)*t)*(1.0-u)
					+ ((c4*(1.0-s) + c5*s)*(1.0-t) + (c6*(1.0-s) + c7*s)*t)*u;

	// gradient interpolation, one dimension at a time
	/*gx = ((grads[0].X()*(1.0-s) + grads[1].X()*s)*(1.0-t)
				+ (grads[2].X()*(1.0-s) + grads[3].X()*s)*t)*(1.0-u)
				+ ((grads[4].X()*(1.0-s) + grads[5].X()*s)*(1.0-t)
				+ (grads[6].X()*(1.0-s) + grads[7].X()*s)*t)*u;
	gy = ((grads[0].Y()*(1.0-s) + grads[1].Y()*s)*(1.0-t)
				+ (grads[2].Y()*(1.0-s) + grads[3].Y()*s)*t)*(1.0-u)
				+ ((grads[4].Y()*(1.0-s) + grads[5].Y()*s)*(1.0-t)
				+ (grads[6].Y()*(1.0-s) + grads[7].Y()*s)*t)*u;
	gz = ((grads[0].Z()*(1.0-s) + grads[1].Z()*s)*(1.0-t)
				+ (grads[2].Z()*(1.0-s) + grads[3].Z()*s)*t)*(1.0-u)
				+ ((grads[4].Z()*(1.0-s) + grads[5].Z()*s)*(1.0-t)
				+ (grads[6].Z()*(1.0-s) + grads[7].Z()*s)*t)*u;
	// then copy to grad
	grad.Set(gx,gy,gz,0.0);*/

	// we don't use this value right now, so it doesn't matter
	grad.Set(0.0,0.0,0.0,0.0);

	return true;
}

double Volume::NearestIntersection(const Ray &r) const
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

void Volume::ComputeBoundingBox(float min[3], float max[3])
{
	// fill in min and max vectors
	mMin.Set(min[0],min[1],min[2],1.0);
	mMax.Set(max[0],max[1],max[2],1.0);

	for (int i=0; i<3; i++)
		mFaceOffs[i] = -max[i];
}

bool Volume::PointWithinBoundingBox(const Vector4 &P) const
{
	bool result=false;
	double x=P.X(),y=P.Y(),z=P.Z(),eps=0.0;

	if (x > mMin.X()-eps && x < mMax.X()+eps
			&& y > mMin.Y()-eps && y < mMax.Y()+eps
			&& z > mMin.Z()-eps && z < mMax.Z()+eps)
		result = true;

	return result;
}

void Volume::DetermineMinMaxDensity(int size)
{
	switch(mDatatype)
	{
		case 1:
		{
			unsigned char *ds = (unsigned char *)mDataset;
			mcMinDens = 255;
			mcMaxDens = 0;

			for (int i=0; i < size; i++)
			{
				if (ds[i] < mcMinDens)
					mcMinDens = ds[i];
				if (ds[i] > mcMaxDens)
					mcMaxDens = ds[i];
			}
			break;
		}
		case 2:
		{
			unsigned short *ds = (unsigned short *)mDataset;
			msMinDens = 65535;
			msMaxDens = 0;

			for (int i=0; i < size/2; i++)
			{
				if (ds[i] < msMinDens)
					msMinDens = ds[i];
				if (ds[i] > msMaxDens)
					msMaxDens = ds[i];
			}
			break;
		}
		case 4:
		{
			float *ds = (float *)mDataset;
			mfMinDens = 1000000.0;
			mfMaxDens = -1000000.0;

			for (int i=0; i < size/4; i++)
			{
				if (ds[i] < mfMinDens)
					mfMinDens = ds[i];
				if (ds[i] > mfMaxDens)
					mfMaxDens = ds[i];
			}
			break;
		}
		default:
			break;
	}
}

/***************************************************************************/


void parse_rawiv_header(char *buffer,float *min, float *max, unsigned int *numverts,
					unsigned int *numcells, unsigned int *dim, float *origin, float *span)
{
	// read minX
	swap_big_to_little_float(min[0], buffer);
	buffer += 4;
	// read minY
	swap_big_to_little_float(min[1], buffer);
	buffer += 4;
	// read minZ
	swap_big_to_little_float(min[2], buffer);
	buffer += 4;

	// read maxX
	swap_big_to_little_float(max[0], buffer);
	buffer += 4;
	// read maxY
	swap_big_to_little_float(max[1], buffer);
	buffer += 4;
	// read maxZ
	swap_big_to_little_float(max[2], buffer);
	buffer += 4;

	// read numverts 
	swap_big_to_little_uint(*numverts, buffer);
	buffer += 4;

	// read numcells 
	swap_big_to_little_uint(*numcells, buffer);
	buffer += 4;

	// read dimX
	swap_big_to_little_uint(dim[0], buffer);
	buffer += 4;
	// read dimY
	swap_big_to_little_uint(dim[1], buffer);
	buffer += 4;
	// read dimZ
	swap_big_to_little_uint(dim[2], buffer);
	buffer += 4;

	// read originX
	swap_big_to_little_float(origin[0], buffer);
	buffer += 4;
	// read originY
	swap_big_to_little_float(origin[1], buffer);
	buffer += 4;
	// read originZ
	swap_big_to_little_float(origin[2], buffer);
	buffer += 4;

	// read spanX
	swap_big_to_little_float(span[0], buffer);
	buffer += 4;
	// read spanY
	swap_big_to_little_float(span[1], buffer);
	buffer += 4;
	// read spanZ 
	swap_big_to_little_float(span[2], buffer);
}

void swap_big_to_little_uint(unsigned int &val, char *buffer)
{
	ui_swap uis;

	// swap the order (little -> big)
	uis.bytes[0] = buffer[3];
	uis.bytes[1] = buffer[2];
	uis.bytes[2] = buffer[1];
	uis.bytes[3] = buffer[0];
	// assign the value
	val = uis.value;
}

void swap_little_to_big_uint(unsigned int &val, char *buffer)
{
	ui_swap uis;

	// assign the value
	uis.value = val;
	// swap the order (little -> big)
	buffer[3] = uis.bytes[0];
	buffer[2] = uis.bytes[1];
	buffer[1] = uis.bytes[2];
	buffer[0] = uis.bytes[3];
}

void swap_big_to_little_float(float &val, char *buffer)
{
	f_swap fs;

	// swap the order (little -> big)
	fs.bytes[0] = buffer[3];
	fs.bytes[1] = buffer[2];
	fs.bytes[2] = buffer[1];
	fs.bytes[3] = buffer[0];
	// assign the value
	val = fs.value;
}

void swap_little_to_big_float(float &val, char *buffer)
{
	f_swap fs;

	// assign the value
	fs.value = val;
	// swap the order (little -> big)
	buffer[3] = fs.bytes[0];
	buffer[2] = fs.bytes[1];
	buffer[1] = fs.bytes[2];
	buffer[0] = fs.bytes[3];
}

void swap_buffer(char *buffer, int size, int type)
{
	// swapping isn't necessary on single byte data
	if (type == 1)
		return;
	
	char sbuf[4];

	for (int i=0; i < size/type; i++)
	{
		memcpy(sbuf, buffer+(i*type), type);
		
		switch (type)
		{
			case 2:
			{
				buffer[i*type] = sbuf[1];
				buffer[i*type+1] = sbuf[0];
				break;
			}
			case 4:
			{
				buffer[i*type] = sbuf[3];
				buffer[i*type+1] = sbuf[2];
				buffer[i*type+2] = sbuf[1];
				buffer[i*type+3] = sbuf[0];
				break;
			}
			default:
				break;
		}
	}
}

