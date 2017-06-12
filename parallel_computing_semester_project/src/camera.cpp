/*
	camera.cpp
	$Id: camera.cpp,v 1.1.1.1 2003/04/19 05:18:21 prok Exp $
	
	$Log: camera.cpp,v $
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#include "camera.h"
#include "invertMatrix.h"

Camera::Camera()
: COP(0.0,0.0,0.0,1.0),
v1(-1.0,1.0,1.0,1.0),
v2(1.0,1.0,1.0,1.0),
v3(1.0,-1.0,1.0,1.0),
v4(-1.0,-1.0,1.0,1.0),
xDir(1.0,0.0,0.0,0.0),
yDir(0.0,-1.0,0.0,0.0),
xStep(0.01),yStep(0.01)
{
}

Camera::Camera(const Vector4 &cop, const Vector4 &lookat, const Vector4 &up,
								double fov, double a_ratio, double width, double height)	
: COP(cop),
v1(),
v2(),
v3(),
v4(),
xDir(),
yDir()
{
	Vector4 dir, center, screen_vert, screen_horiz;
	double x_off, y_off;
	
	// direction we're pointed
	dir = lookat-cop; dir.Normalize();
	// center of the virtual screen
	center = cop + dir;
	// vector pointing "up" from center
	screen_vert = up - (up*dir)*dir; screen_vert.Normalize();
	// vector pointing "right" from center
	screen_horiz = up.Cross(dir); screen_horiz.Normalize();
	
	// convert fov to radians
	fov = fov * M_PI / 180.0;
	// distance from center to top/bottom of screen
	y_off = tan( fov / 2.0 );
	// distance from center to left/right edge of screen
	x_off = a_ratio * y_off;
	
	// corner vertices of the screen rectangle
	v1 = (center + (y_off * screen_vert)) - (x_off * screen_horiz);
	v2 = (center + (y_off * screen_vert)) + (x_off * screen_horiz);
	v3 = (center - (y_off * screen_vert)) - (x_off * screen_horiz);
	v4 = (center - (y_off * screen_vert)) + (x_off * screen_horiz);
	
	// pixel dimensions
	xStep = (v1-v2).Magnitude() / (width-1.0);
	yStep = (v1-v3).Magnitude() / (height-1.0);
	
	// x and y direction vectors for calculating points
	xDir = -screen_horiz;
	yDir = -screen_vert;
	
	//printf("Camera::Camera()\n");
	//printf("eye = "); cop.Print();
	//printf("look direction = "); dir.Print();
	//printf("x_off = %f, y_off = %f\n", x_off, y_off);
	//printf("center = "); center.Print();
	//printf("screen_vert = "); screen_vert.Print();
	//printf("screen_horiz = "); screen_horiz.Print();
	//printf("v1 = "); v1.Print();
	//printf("v2 = "); v2.Print();
	//printf("v3 = "); v3.Print();
	//printf("v4 = "); v4.Print();
	//printf("xStep = %f, yStep = %f\n", xStep, yStep);
	//printf("xDir = "); xDir.Print();
	//printf("yDir = "); yDir.Print();
}

Camera::Camera(const Camera &c)
: COP(c.COP),
v1(c.v1),
v2(c.v2),
v3(c.v3),
v4(c.v4),
xDir(c.xDir),
yDir(c.yDir),
xStep(c.xStep),yStep(c.yStep)
{
}

Camera::~Camera()
{
	// do nothing
}

Ray Camera::RayForPixel(int x, int y) const
{
	// the point for the pixel
	Vector4 P = v2 + ((x*xStep)*xDir) + ((y*yStep)*yDir);
	
	//P.Print();
	
	// return the ray
	return Ray(COP, P);
}
