/*
	veclib.h
	$Id: veclib.h,v 1.1.1.1 2003/04/19 05:18:21 prok Exp $
	
	$Log: veclib.h,v $
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#ifndef VECLIB_H
#define VECLIB_H

#include <math.h>
#include <stdio.h>

class Vector4 {
public:
															Vector4();
															Vector4(double x, double y, double z, double w);
															Vector4(const Vector4 &v);
															~Vector4() {}
															
	void												Set(double x, double y, double z, double w);
	
	/* access */
	double											X() const { return vec[0]; }
	double											Y() const { return vec[1]; }
	double											Z() const { return vec[2]; }
	double											W() const { return vec[3]; }
	
	/* assignment */
	Vector4											operator=(const Vector4 &v);
	/* opposite */
	Vector4											operator-();
	/* subtraction */
	Vector4											operator-(const Vector4 &v) const;
	Vector4	&										operator-=(const Vector4 &v);
	/* addition */
	Vector4											operator+(const Vector4 &v) const;
	Vector4	&										operator+=(const Vector4 &v);
	/* scalar multiply */
	Vector4											operator*(double a) const;
	Vector4	&										operator*=(double a);											
	/* vector ops */
	double											operator*(const Vector4 &v) const;
	Vector4											Cross(const Vector4 &v) const;
	void												Normalize();
	double											Magnitude() const;
	/* matrix vector multiply */
	Vector4											operator*(double m[16]) const;
	
	void												Print() const;
	
	friend Vector4							operator*(float a, const Vector4 &v);

private:

	double											vec[4];
};

//class VecLib {
//public:
//										VecLib() { /* nothing */ }
//	
//	static Vector4		PlaneNormal(const Vector4 &A, const Vector4 &B, const Vector4 &C);
//	static float			PointToPlaneDist(const Vector4 &P, const Vector4 &N, float d);
//};

/////////////////////////////////////////////////////////////////////////////

inline Vector4::Vector4()
{
	vec[0] = vec[1] = vec[2] = vec[3] = 0.0;
}

inline Vector4::Vector4(double x, double y, double z, double w)
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	vec[3] = w;
}

inline Vector4::Vector4(const Vector4 &v)
{
	vec[0] = v.vec[0];
	vec[1] = v.vec[1];
	vec[2] = v.vec[2];
	vec[3] = v.vec[3];
}

inline void Vector4::Set(double x, double y, double z, double w)
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	vec[3] = w;
}

inline Vector4 Vector4::operator=(const Vector4 &v)
{
	vec[0] = v.vec[0];
	vec[1] = v.vec[1];
	vec[2] = v.vec[2];
	vec[3] = v.vec[3];
	
	return *this;
}

inline Vector4 Vector4::operator-()
{
	Vector4 res;
	
	res.vec[0] = -vec[0];
	res.vec[1] = -vec[1];
	res.vec[2] = -vec[2];
	res.vec[3] = vec[3];
	
	return res;
}

inline Vector4 Vector4::operator-(const Vector4 &v) const
{
	Vector4 res;
	
	res.vec[0] = vec[0] - v.vec[0];
	res.vec[1] = vec[1] - v.vec[1];
	res.vec[2] = vec[2] - v.vec[2];
	res.vec[3] = vec[3];
	
	return res;
}

inline Vector4 & Vector4::operator-=(const Vector4 &v)
{
	vec[0] -= v.vec[0];
	vec[1] -= v.vec[1];
	vec[2] -= v.vec[2];
	//vec[3] -= v.vec[3];
	
	return *this;
}

inline Vector4 Vector4::operator+(const Vector4 &v) const
{
	Vector4 res;
	
	res.vec[0] = vec[0] + v.vec[0];
	res.vec[1] = vec[1] + v.vec[1];
	res.vec[2] = vec[2] + v.vec[2];
	res.vec[3] = vec[3];
	
	return res;
}

inline Vector4 & Vector4::operator+=(const Vector4 &v)
{
	vec[0] += v.vec[0];
	vec[1] += v.vec[1];
	vec[2] += v.vec[2];
	//vec[3] += vec[3];
	
	return *this;
}

inline Vector4 Vector4::operator*(double a) const
{
	Vector4 res;
	
	res.vec[0] = a * vec[0];
	res.vec[1] = a * vec[1];
	res.vec[2] = a * vec[2];
	res.vec[3] = vec[3];
	
	return res;
}

inline Vector4 & Vector4::operator*=(double a)
{
	vec[0] *= a;
	vec[1] *= a;
	vec[2] *= a;
	//vec[3] *= a;
	
	return *this;
}

/* vector ops: dot, cross, normalize, & magnitude */

inline double Vector4::operator*(const Vector4 &v) const
{
	double res = (vec[0]*v.vec[0] + vec[1]*v.vec[1] 
									+ vec[2]*v.vec[2] + vec[3]*v.vec[3]);
	
	return res;
}

inline Vector4 Vector4::Cross(const Vector4 &v) const
{
	// this X v
	Vector4 res;
	
	res.vec[0] = (vec[1]*v.vec[2]) - (vec[2]*v.vec[1]);
	res.vec[1] = (vec[2]*v.vec[0]) - (vec[0]*v.vec[2]);
	res.vec[2] = (vec[0]*v.vec[1]) - (vec[1]*v.vec[0]);
	res.vec[3] = 0.0; // this is a vector
	
	return res;
}

inline void Vector4::Normalize()
{
	double mag = this->Magnitude();

	if (mag != 0.0)
	{
		vec[0] /= mag;
		vec[1] /= mag;
		vec[2] /= mag;
		vec[3] = 0.0;
	}
}

inline double Vector4::Magnitude() const
{
	return sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
}

inline void Vector4::Print() const
{
	printf("Point/Vector: (%2.2f, %2.2f, %2.2f, %2.2f)\n",
									vec[0], vec[1], vec[2], vec[3]);
}

inline Vector4 Vector4::operator*(double m[16]) const
{
	Vector4 res;

	res.vec[0] = vec[0]*m[0] + vec[1]*m[4] + vec[2]*m[8] + vec[3]*m[12];
	res.vec[1] = vec[0]*m[1] + vec[1]*m[5] + vec[2]*m[9] + vec[3]*m[13];
	res.vec[2] = vec[0]*m[2] + vec[1]*m[6] + vec[2]*m[10] + vec[3]*m[14];
	res.vec[3] = vec[0]*m[3] + vec[1]*m[7] + vec[2]*m[11] + vec[3]*m[15];

	return res;
}

/* friends */
inline Vector4 operator*(double a, const Vector4 &v)
{
	return v*a;
}

inline Vector4 operator*(double m[16], const Vector4 &v)
{
	return v*m;
}

/////////////////////////////////////////////////////////////////////////////////

//inline Vector4 VecLib::PlaneNormal(const Vector4 &A, const Vector4 &B, const Vector4 &C)
//{
	//				B
	//
	//
	//	C						A
	// p1,p2,p3 are the vertices of a triangle, given in counter-clockwise order
	// compute the normal of the plane they lie in. (normal points out of screen)
	// N = (A - B) X (C - B)
//	Vector4 U, V;
//	U = A-B;
//	V = C-B;
//	U.Print(); V.Print();
//	U = U.Cross(V);
//	U.Print();
//	U.Normalize();
//	return U;
//}

//inline float VecLib::PointToPlaneDist(const Vector4 &P, const Vector4 &N, float d)
//{
	// (P - Q) || N
	// => (P - Q) = k*N
	// => Q = P - k*N
	// N*Q - d = 0
	// => N*(P - k*N) - d = 0
	// => N*P - N*(k*N) - d = 0
	// => k = (N*P - d) / (N*N)
	// => k = N*P - d (because N is unit length)
//	float k;
	
//	k = N*P;
//	k -= d;
	
//	return k;
//}

#endif

