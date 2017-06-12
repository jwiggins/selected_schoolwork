/*
	trace.h
	$Id: trace.h,v 1.3 2003/04/25 21:43:35 prok Exp $
	
	$Log: trace.h,v $
	Revision 1.3  2003/04/25 21:43:35  prok
	Volume rendering is finally correct. Shading is turned off, because it's broken.
	Two nasty bugs were squashed in transferfunction.h. Data distribution code in
	read_data.cpp was modified to do a random distribution, rather than a modulo
	distribution.
	
	Revision 1.2  2003/04/20 08:14:49  prok
	
	This is the initial working version. Output is mostly correct...
	I Need to look at more transfer functions and datasets.
	
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#ifndef TRACE_H
#define TRACE_H

#include <vector>
#include <math.h>
#include <stdio.h>
#include "ray.h"
#include "veclib.h"
#include "color.h"
#include "light.h"
#include "volume.h"

using namespace std;

class Trace {
public:
											Trace(Volume *v);
											Trace(const Trace &t);
											~Trace();
											
	void								AddLightToScene(Light *light);
	
	color_t							RayCast(const Ray &r) const;

private:
	color_t							compute_direct_illumination(color_t c, const Vector4 &P,
																	const Vector4 &V, const Vector4 &N) const;
	color_t							local_illumination(color_t c, const Vector4 &P, const Vector4 &N,
																										const Vector4 &L, const Vector4 &H) const;

	vector<Light *>			light_list;
	Volume							*vol;
};

#endif
