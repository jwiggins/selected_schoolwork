/*
	loadbalancer.h
	$Id: loadbalancer.h,v 1.1 2003/05/08 02:44:49 prok Exp $

	$Log: loadbalancer.h,v $
	Revision 1.1  2003/05/08 02:44:49  prok
	Revamped load balancing. Made one of the render nodes a dedicated load balancer.
	Now instead of render nodes picking the patches they will render and then
	rendering them, they will contact the load balancer for a work unit of 5 or
	fewer patches. This _should_ resolve the massive load imbalance in the previous
	version.
	
*/

#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "myrandom.h"
#include "messagetags.h"

class LoadBalancer {
public:
											LoadBalancer(int w, int h, int p);
											~LoadBalancer();

	void								Run();

private:
	
	bool								*mPatches;
	int									mNumPatches;
	int									mPatchesRemaining;

};

#endif

