/*
	cachemanager.h
	$Id: cachemanager.h,v 1.4 2003/05/08 01:34:19 prok Exp $

	$Log: cachemanager.h,v $
	Revision 1.4  2003/05/08 01:34:19  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
*/

#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include <mpi.h>
#include "lrudiskcache.h"
#include "messagetags.h"

class WageSlave {
public:
													WageSlave(char *cachedir);
													~WageSlave();

	void										Run();

	void										LoadDataset(char *name)
													{ mCache->LoadDataset(name); }
private:

	LRUDiskCache						*mCache;
	int											mRank;

};

#endif
