/*
	transferfunction.h
	$Id: transferfunction.h,v 1.5 2003/05/04 23:51:35 prok Exp $

	$Log: transferfunction.h,v $
	Revision 1.5  2003/05/04 23:51:35  prok
	Small debugging related changes.
	
	Revision 1.4  2003/04/25 21:43:41  prok
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

#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include <list>
#include "color.h"

using namespace std;

struct opac {
	float pos;
	float alpha;
};

struct color {
	float pos;
	color_t col;
};

class TransferFunction {
public:
									TransferFunction();
									TransferFunction(const TransferFunction &t);
									~TransferFunction();

	void						LoadVinayFile(char *filename);
	void						AddOpacity(float position, float opacity);
	void						AddColor(float position, float r, float g, float b);

	float						GetOpacity(float density) const;
	color_t					GetColor(float density) const;

	void						SetDensityRange(float lower, float upper);
	void						Print() const;

private:
	
	float						mRStart, mREnd;
	list<opac>			mOpacities;
	list<color>			mColors;
};

inline TransferFunction::TransferFunction()
: mOpacities(), mColors()
{
	mRStart = 0.0;
	mREnd = 1.0;
}

inline TransferFunction::TransferFunction(const TransferFunction &t)
:mRStart(t.mRStart),mREnd(t.mREnd),
mOpacities(t.mOpacities), mColors(t.mColors)
{
}

inline TransferFunction::~TransferFunction()
{
	// nothin!
}

inline void TransferFunction::LoadVinayFile(char *filename)
{
	FILE *fp;
	int alphanodes, colornodes, isonodes;

	
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("could not open file \"%s\"\n", filename);
		exit(-1);
	}

	fscanf(fp, "Anthony and Vinay are Great.\n");
	fscanf(fp, "Alphamap\n");
	fscanf(fp, "Number of nodes\n%d\n", &alphanodes);
	fscanf(fp, "Position and opacity\n");
	for (int i=0; i < alphanodes; i++)
	{
		float pos, opacity;
		fscanf(fp, "%f %f\n", &pos, &opacity);
		//printf("alpha node %d: position = %f, opacity = %f\n",i,pos,opacity);
		AddOpacity(pos,opacity);
	}
	fscanf(fp, "ColorMap\n");
	fscanf(fp, "Number of nodes\n%d\n", &colornodes);
	fscanf(fp, "Position and RGB\n");
	for (int i=0; i < colornodes; i++)
	{
		float pos, R,G,B;
		fscanf(fp, "%f %f %f %f\n", &pos, &R, &G, &B);
		//printf("color node %d: position = %f, RGB = (%f,%f,%f)\n",i,pos,R,G,B);
		AddColor(pos,R,G,B);
	}
	fscanf(fp, "IsocontourMap\n");
	fscanf(fp, "Number of nodes\n%d\n", &isonodes);
	fscanf(fp, "Position\n");
	for (int i=0; i < isonodes; i++)
	{
		float pos;
		fscanf(fp, "%f\n", &pos);
		//printf("isocontour node %d: position = %f\n",i,pos);
	}

	//Print();

	fclose(fp);
}

inline void TransferFunction::AddOpacity(float position, float opacity)
{
	opac o;
	bool ins=false;
	
	// fill it in
	o.pos = position;
	o.alpha = opacity;
	// add it to the opacities vector
	//if (mOpacities.empty())
	//	mOpacities.push_back(o);
	//else
		for (list<opac>::iterator i=mOpacities.begin();!ins && i != mOpacities.end(); ++i)
		{
			if (position <= i->pos)
			{
				mOpacities.insert(i, o);
				ins = true;
			}
		}
	if (!ins)
		mOpacities.push_back(o);
	//mOpacities.push_back(o);
}

inline void TransferFunction::AddColor(float position, float r, float g, float b)
{
	color c;
	color_t rgb(r,g,b);
	bool ins=false;

	// fill it in
	c.pos = position;
	c.col = rgb;
	// add it to the colors vector
	//if (mColors.size() == 0)
	//	mColors.push_back(c);
	//else
		for (list<color>::iterator i=mColors.begin();!ins && i != mColors.end(); ++i)
		{
			if (position <= i->pos)
			{
				mColors.insert(i, c);
				ins = true;
			}
		}
	if (!ins)
		mColors.push_back(c);
	//mColors.push_back(c);
}

inline float TransferFunction::GetOpacity(float density) const
{
	float dScaled, result;
	list<opac>::const_iterator i = mOpacities.begin();
	opac b, a;
	
	// what is density when scaled to the range [0,1] ?
	dScaled = (density-mRStart) / (mREnd-mRStart);

	while (i != mOpacities.end() && dScaled >= i->pos)
		++i;

	b = *i;
	a = *(--i);
	
	result = a.alpha + (dScaled-a.pos)*((b.alpha-a.alpha)/(b.pos-a.pos));

	/*for (; i != mOpacities.end(); ++i)
	{
		if (dScaled < i->pos)
		{
			opac b = *i, a = *(--i);
			result = a.alpha + (dScaled-a.pos)*((b.alpha-a.alpha)/(b.pos-a.pos));
			break;
		}
	}*/

	//if (result > 1.0)
	//	printf("warning! transfer function returning opacity > 1.0\n");
	//else if (result < 0.0)
	//{
	//	printf("what the fuck? transfer function returning opacity < 0.0\n");
	//	printf("density = %f, dScaled = %f\n", density, dScaled);
	//}

	return result;
}

inline color_t TransferFunction::GetColor(float density) const
{
	color_t result;
	float dScaled;
	list<color>::const_iterator i = mColors.begin();

	// what is density when scaled to the range [0,1] ?
	dScaled = (density-mRStart) / (mREnd-mRStart);
	
	/*for (;i != mColors.end(); ++i)
	{
		if (dScaled < i->pos)
		{
			color b = *i, a = *(--i);
			float xpos=dScaled-a.pos;

			result.r = a.col.r+xpos*((b.col.r-a.col.r)/(b.pos-a.pos));
			result.g = a.col.g+xpos*((b.col.g-a.col.g)/(b.pos-a.pos));
			result.b = a.col.b+xpos*((b.col.b-a.col.b)/(b.pos-a.pos));
			break;
		}
	}*/

	while (i != mColors.end() && dScaled >= i->pos)
		i++;

	color b = *i, a = *(--i);
	float xpos=dScaled-a.pos;

	result.r = a.col.r+xpos*((b.col.r-a.col.r)/(b.pos-a.pos));
	result.g = a.col.g+xpos*((b.col.g-a.col.g)/(b.pos-a.pos));
	result.b = a.col.b+xpos*((b.col.b-a.col.b)/(b.pos-a.pos));
	

	//if (result.r > 1.0 || result.g > 1.0 || result.b > 1.0)
	//	printf("warning! transfer function returning r, g, or b > 1.0\n");

	return result;
}

inline void TransferFunction::SetDensityRange(float lower, float upper)
{
	mRStart = lower;
	mREnd = upper;
}

inline void TransferFunction::Print() const
{
	for (list<opac>::const_iterator i=mOpacities.begin(); i != mOpacities.end(); ++i)
		printf("opacity: position = %f(%f), alpha = %f\n",
					i->pos, (i->pos-mRStart)*(mREnd-mRStart), i->alpha);

	for (list<color>::const_iterator i=mColors.begin(); i != mColors.end(); ++i)
		printf("color: position = %f(%f), RGB = (%f,%f,%f)\n",
					i->pos, (i->pos-mRStart)*(mREnd-mRStart), i->col.r,i->col.g,i->col.b);
}

#endif

