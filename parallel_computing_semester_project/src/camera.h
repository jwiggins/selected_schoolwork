/*
	camera.h
	$Id: camera.h,v 1.1.1.1 2003/04/19 05:18:21 prok Exp $
	
	$Log: camera.h,v $
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <math.h>
#include "veclib.h"
#include "ray.h"

class Camera {
public:
									Camera();
									Camera(const Vector4 &cop, const Vector4 &dir, const Vector4 &up,
													double fov, double a_ratio, double width, double height);
									Camera(const Camera &c);
									~Camera();
									
	Ray							RayForPixel(int x, int y) const;
private:

	Vector4					COP;
	Vector4					v1,v2,v3,v4;
	Vector4					xDir,yDir;
	double					xStep,yStep;
};

#endif

