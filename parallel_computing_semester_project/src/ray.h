/*
	ray.h
	$Id: ray.h,v 1.1.1.1 2003/04/19 05:18:21 prok Exp $
	
	$Log: ray.h,v $
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#ifndef RAY_H
#define RAY_H

#include <stdio.h>
#include "veclib.h"

class Ray {
public:
								Ray();
								Ray(const Vector4 &p0, const Vector4 &p1);
								Ray(const Ray &r);
								~Ray() {}

	void					Set(const Vector4 &p0, const Vector4 &p1);
	bool					IsValid() const { return valid; }
	
	Vector4				Origin() const { return origin; }
	Vector4				EndPoint() const { return endpoint; }
	Vector4				Direction() const { return direction; }
	
	Vector4				PointAt(double t) const;
	double				TForPoint(const Vector4 &P) const;
	
	void					Print() const;
															
private:

	Vector4				origin, endpoint;
	Vector4				direction;
	bool					valid;
};

inline Ray::Ray()
: origin(0.0,0.0,0.0,1.0),
endpoint(0.0,0.0,0.0,1.0),
direction(0.0,0.0,0.0,0.0),
valid(false)
{}

inline Ray::Ray(const Ray &r)
: origin(r.origin),
endpoint(r.endpoint),
direction(r.direction),
valid(r.valid)
{}

inline Ray::Ray(const Vector4 &p0, const Vector4 &p1)
: origin(p0),
endpoint(p1),
direction(p1 - p0),
valid(true)
{
	// make sure direction is unit length
	direction.Normalize();
}

inline void Ray::Set(const Vector4 &p0, const Vector4 &p1)
{
	origin = p0;
	endpoint = p1;
	direction = p1 - p0; direction.Normalize();
	valid = true;
}

inline Vector4 Ray::PointAt(double t) const
{
	return origin + (direction * t);
}

inline double Ray::TForPoint(const Vector4 &P) const
{
	// direction is unit, so the length of the vector between
	// P and the origin is also the t value needed to obtain P
	return (P-origin).Magnitude();
}

inline void Ray::Print() const
{
	printf("Ray: (%2.2f,%2.2f,%2.2f,%2.2f) + (%2.2f,%2.2f,%2.2f, %2.2f)*t\n",
									origin.X(),origin.Y(),origin.Z(),origin.W(),
									direction.X(),direction.Y(),direction.Z(),direction.W());
}

#endif

