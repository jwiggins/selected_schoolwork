/*
	light.h
	$Id: light.h,v 1.1.1.1 2003/04/19 05:18:21 prok Exp $
	
	$Log: light.h,v $
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#ifndef LIGHT_H
#define LIGHT_H

#include "veclib.h"
#include "color.h"

class Light {
public:
										Light();
										Light(float x, float y, float z, float r, float g, float b);
										Light(const Light &l);
										~Light() {}
										
	Vector4						Position() const { return position; }
	color_t						Intensity() const { return intensity; }

private:

	Vector4						position;
	color_t						intensity;
};

inline Light::Light()
: position(0.0,0.0,0.0,1.0),
intensity(0.0,0.0,0.0)
{}

inline Light::Light(float x, float y, float z, float r, float g, float b)
: position(x,y,z,1.0),
intensity(r,g,b)
{}

inline Light::Light(const Light &l)
: position(l.position),
intensity(l.intensity)
{}

#endif
