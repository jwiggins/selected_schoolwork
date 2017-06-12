/*
	trace.cpp
	$Id: trace.cpp,v 1.5 2003/04/28 03:40:31 prok Exp $
	
	$Log: trace.cpp,v $
	Revision 1.5  2003/04/28 03:40:31  prok
	This is the initial checkin of the parallel code. It's just a skeleton so far.
	cachemanager.* : defines the WageSlave class, which manages datasets on disk.
	lrudiskcache.* : defines the LRUDiskCache class, which does most of the work
	for WageSlave.
	renderslave.* : defines the RenderSlave class, which renders subsets of images.
	lrumemcache,* : defines the LRUMemCache class, which manages the unholy task of keeping RenderSlave fed with volume blocks from remote WageSlave processes.
	
	Revision 1.4  2003/04/25 21:43:32  prok
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
	
	Revision 1.1.1.1  2003/04/19 05:18:21  prok
	This is my parallel ray casting volume renderer. I'm writing it for CS377, but
	if it turns out that this parallelization strategy is fast, I might develop it
	further.
	
*/

#include "trace.h"

Trace::Trace(Volume *v)
: light_list(0)
{
	vol = v;
}

Trace::Trace(const Trace &t)
: light_list(0), vol(t.vol)
{
	// fill the light list
	for (int i=0; i < t.light_list.size(); i++)
		light_list.push_back(new Light(*(t.light_list[i])));
}

Trace::~Trace()
{
	// clean up
	for (int i=0; i < light_list.size(); i++)
		delete light_list[i];
}

void Trace::AddLightToScene(Light *light)
{
	//printf("Trace::AddLightToScene(%p)\n", light);
	light_list.push_back(light);
}

// This function returns the color accumulated by a ray as it traverses the scene,
// possibly breaking up recursively into many child rays
color_t Trace::RayCast(const Ray &r) const
{
	color_t result(0.0,0.0,0.0);
	double t;

	//r.Print();
	
	// check ray validity
	if (!r.IsValid())
		; // do nothing (return black)
	else
	{
		// color pixels!
		if ((t = vol->NearestIntersection(r)) > 0.0)
		{
			float opacity=0.0, density;
			double tInc, gMag;
			bool inVolume=true;
			Vector4 span, grad,P;
			int samples=0;
			bool overflow = false;

			// set the increment to half a cell diagonal
			vol->GetCellSpan(span);
			//tInc = span.Magnitude() / 2.0;
#define MIN3(x,y,z)\
			((x <= y && x <= z) ? x : (y <= x && y <= z) ? y : z)
			tInc = MIN3(span.X(),span.Y(),span.Z()) / 3.0;

			// integrate opacity and color along the ray
			while (inVolume && (opacity < 0.95))
			{
				// increment t, samples
				t += tInc;
				samples++;

				// the point on the ray where we want to take a sample
				P = r.PointAt(t);

				// take the sample
				inVolume = vol->GetDensityAndGradient(P,density,grad);
				
				// if the point was inside the volume, accumulate color
				if (inVolume)
				{
					float o;
					color_t c;

					// convert density to color and opacity
					o = (vol->GetTransferFunction()).GetOpacity(density);
					c = (vol->GetTransferFunction()).GetColor(density);

					// get the magnitude of the gradient, then normalize the gradient
					//gMag = grad.Magnitude();
					//grad.Normalize();

					// flip the direction of the gradient if it's pointing
					// away from the eye
					//if (grad*r.Direction() > 0.0)
					//	grad *= -1.0;
					
	
					//printf("(%2.2f,(%2.2f,%2.2f,%2.2f)) ", o, c.r,c.g,c.b);
					//printf("(%2.2f, (%2.2f,%2.2f,%2.2f)) ", density, grad.X(), grad.Y(), grad.Z());

					// transform the sample color
					//c = compute_direct_illumination(c, P, r.Direction(), grad);

					// blend color into result
					//result = (1.0-opacity)*result + o*c;
					result = result + (1.0-opacity)*o*c;

					// accumulate opacity
					opacity = opacity + (1.0-opacity)*o;

					//if ((result.r > 1.0 || result.g > 1.0 || result.b > 1.0) && !overflow)
					//{
					//	printf("color overflow: c = (%2.2f,%2.2f,%2.2f), o = %f, opacity = %f\n",
					//						c.r,c.g,c.b,o,opacity);
					//	printf("sample = %d\n", samples);
					//	overflow = true;
					//}
				}
			}

			//printf(" :%d\n",samples);
			//result = color_t(1.0,1.0,1.0);
			//r.PointAt(t).Print();
		}
	}
	
	return result;
}

color_t Trace::compute_direct_illumination(color_t c, const Vector4 &P,
								const Vector4 &V, const Vector4 &N) const
{
	color_t result = c;//(0.0,0.0,0.0);

//	static int count=0;	
//	printf("Trace::compute_direct_illumination() : %d\n", count++);
	
	// loop through all the lights
	for (int i=0; i < light_list.size(); i++)
	{
		Light *light = light_list[i];
		Ray r(P, light->Position()); // ray from P -> light
		Vector4 H, L;
				
		// the light vector
		L = r.Direction();
		// find the half vector
		H = N+L; H.Normalize();

		// compute the intensity from this light
		result = result + (light->Intensity() * local_illumination(c,P,N,L,H));
	}
	
	return result;
}

color_t Trace::local_illumination(color_t c, const Vector4 &P, const Vector4 &N,
																		const Vector4 &L, const Vector4 &H) const
{
	color_t result;
	color_t spec(1.0,1.0,1.0), diff;
	float shiny = 32.0;
	double ndotl, ndoth;
	
	// diffuse color
	diff = c;
	
	// N*L and N*H
	ndotl = fabs(N*L);
	ndoth = fabs(N*H);
	
	// local illumination (not including ambient light)
	result = (diff*ndotl) + (spec*(float)pow(ndoth, shiny));
	
	return result;
}

