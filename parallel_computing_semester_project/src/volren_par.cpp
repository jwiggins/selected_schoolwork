/*
	volren_par.cpp
	$Id: volren_par.cpp,v 1.9 2003/05/08 23:04:20 prok Exp $

	$Log: volren_par.cpp,v $
	Revision 1.9  2003/05/08 23:04:20  prok
	miniscule changes... everything seems to be working except rendering for the
	SRS00002 dataset. (not sure why) This commit is just to sync changes between
	prism and eyes.
	
	Revision 1.8  2003/05/08 02:44:50  prok
	Revamped load balancing. Made one of the render nodes a dedicated load balancer.
	Now instead of render nodes picking the patches they will render and then
	rendering them, they will contact the load balancer for a work unit of 5 or
	fewer patches. This _should_ resolve the massive load imbalance in the previous
	version.
	
	Revision 1.7  2003/05/08 01:34:20  prok
	Previous changes backed out. When they say MPICH is not thread safe... they
	mean it. Too bad, because it really would have been nice if it had worked.
	
	Revision 1.6  2003/05/08 00:45:58  prok
	Experimental change to render on all nodes. This means that the disk cache
	process runs concurrently with the render process. I'll find out very soon
	if MPICH can handle this kind of abuse. (multiple threads calling MPI_* funcs)
	If this works, then the next step is to write a master/worker style work
	allocation to get past the current load-(im)balancing issues.
	
	Revision 1.5  2003/05/07 16:13:47  prok
	fixed timing code. added more timers.
	
	Revision 1.4  2003/05/07 04:53:44  prok
	Added timing code.
	
	Revision 1.3  2003/05/05 17:15:42  prok
	more debugging.
	
	Revision 1.2  2003/05/04 23:18:05  prok
	Cleanup day! Several preemptive changes made before testing begins:
	- interfe.cpp got some changes related to outputting view params so that output
	from the renderer more closely matches OpenGL output
	- cachmanager.cpp lrumemcache.* and volren_par.cpp got some code to assist in
	exiting cleanly at the end of the render
	- extra attention was paid to asynchronous I/O voodoo in lrumemcache.*.
	Specifically, I/O now happens in two threads, requests get stuffed into a queue
	before being handled, requests stay in a list until they are received. All told,
	I think this code will actually work with the way it is being called now.(lots
	of calls to FetchBlock() were a vector for ugliness)
	- fixed a tiny bug in pvolume.cpp where a ray's state could change even if it
	didn't actually complete a sample.
	- moved sequential rendering out of interfe.cpp and into volren_seq.cpp.
	
	Revision 1.1  2003/05/02 23:55:16  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
*/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include "veclib.h"
#include "renderslave.h"
#include "cachemanager.h"
#include "loadbalancer.h"
#include "messagetags.h"
#include "timer.h"

void usage(char *app)
{
	printf("usage: %s -h | [-f <rawiv filename> -r <cache directory>\n", app);
	printf("\t-c <# cache nodes> -t <vinay file> -v <view params file>\n");
	printf("\t-m <max # MB for render mem.>]\n");
}

void load_view_params(char *file, Vector4 & eye, Vector4 & lookat, Vector4 & up)
{
	FILE *fp;
	float vec[4];

	fp = fopen(file, "r");
	if (fp == NULL)
	{
		printf("could not open view params file %s\n", file);
		exit(-1);
	}

	// eye
	for (int i=0; i<4; i++)
		fscanf(fp, "%f", &vec[i]);
	eye.Set(vec[0],vec[1],vec[2],vec[3]);
	// lookat
	for (int i=0; i<4; i++)
		fscanf(fp, "%f", &vec[i]);
	lookat.Set(vec[0],vec[1],vec[2],vec[3]);
	// up
	for (int i=0; i<4; i++)
		fscanf(fp, "%f", &vec[i]);
	up.Set(vec[0],vec[1],vec[2],vec[3]);
	
	fclose(fp);
}

int main(int argc, char **argv)
{
	// cmdline parsing stuff
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"file", required_argument, 0, 'f'},
		{"rootdir", required_argument, 0, 'r'},
		{"cachenum", required_argument, 0, 'c'},
		{"transfer", required_argument, 0, 't'},
		{"view", required_argument, 0, 'v'},
		{"memory", required_argument, 0, 'm'},
    { 0, 0, 0, 0}
	};
  char *short_options = "f:r:c:t:v:m:h", c;
	int indx=0;
	// cmdline variables
	char *dataset=NULL, *cachedir=NULL, *tfunc=NULL, *viewparam=NULL, *basename;
	int cachenodes=0,maxmem;
	// other variables
	int rank, size;

	// init MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// parse the command line args
	while ((c = getopt_long(argc, argv, short_options, long_options, &indx)) != - 1)
	{
		switch (c)
		{
        case 'f':
            dataset = optarg;
            break;
        case 'r':
            cachedir = optarg;
            break;
        case 'c':
            cachenodes = atoi(optarg);
            break;
				case 't':
						tfunc = optarg;
						break;
				case 'v':
						viewparam = optarg;
						break;
				case 'm':
						maxmem = atoi(optarg);
						break;
				case 'h':
            if (rank == 0)
							usage(argv[0]);
            exit(0);
						break;
        case '?':
				default:
            break;
		}
	}
	
	// bail out if our options aren't initialized
	if (dataset == NULL || cachedir == NULL || tfunc == NULL
			|| viewparam == NULL || cachenodes == 0)
	{
		if (rank == 0)
			usage(argv[0]);
		exit(-1);
	}

	// strip leading characters off of the filename to get the basename
	basename = strrchr(dataset, '/');
	if (basename == NULL)
		basename = dataset;
	else
		basename++;
	// and finally, change dataset
	dataset = basename;

	// what are we about to do?
	if (rank == 0)
	{
		printf("Parallel render beginning on %d processors:\n", size);
		printf("%d cache nodes / 1 load balancer / %d renderers\n",
					cachenodes, size-cachenodes-1);
		printf("dataset: %s\n", dataset);
		fflush(stdout);
	}

	
	// pick a path and stick to it
	if (rank < cachenodes)
	{
		// this node is a cache node
		WageSlave *ioSlave = new WageSlave(cachedir);

		// load the dataset
		ioSlave->LoadDataset(dataset);
		// go!
		ioSlave->Run();

		// send a quit message to the load balancer
		int msg=-1;
		if (rank == 0)
			MPI_Send(&msg, 1, MPI_INT, cachenodes, PVOL_PTCH_REQ_TAG, MPI_COMM_WORLD);

		// send a quit message to render slave receiver threads
		//if (rank == 0)
		//{
		//	int msg[2] = {0,-1};
		//	for (int i=cachenodes; i < size; i++)
		//		MPI_Send(msg, 2, MPI_INT, i, PVOL_BLK_RPLY1_TAG, MPI_COMM_WORLD);
		//}

		// cleanup
		delete ioSlave;
	}
	else if (rank == cachenodes)
	{
		// this node is the load balancer
		LoadBalancer *balance = new LoadBalancer(500,500,10);

		// go!
		balance->Run();

		// clean up
		delete balance;
	}
	else
	{
		// this node is a render node
		RenderSlave *picasso = new RenderSlave(cachenodes, cachedir, maxmem);
		Vector4 eye,lookat,up;
		Timer renderTime("total render time");

		// load the view parameters
		load_view_params(viewparam, eye, lookat, up);

		// order of initialization is important!
		// load the transfer function
		picasso->LoadTransferFunc(tfunc);
		// load the dataset
		picasso->LoadDataset(dataset);

		// set the output size
		picasso->SetOutputSize(500,500);
		// set the patch size
		picasso->SetPatchSize(10);
		// set up the camera
		picasso->SetViewParams(eye,lookat,up);

		// start the render timer
		renderTime.Start();
		
		// go!
		picasso->Run();

		// stop the render timer
		renderTime.Stop();

		// write the output file
		if (rank == cachenodes+1)
		{
			picasso->WriteImageFile("render.ppm");
			// print how long it took to render
			renderTime.Print();
			fflush(stdout);
		}

		// send a quit message to cache nodes
		if (rank == cachenodes+1)
		{
			int msg[2] = {0,-1};
			for (int i=0; i < cachenodes; i++)
				MPI_Send(msg, 2, MPI_INT, i, PVOL_BLK_REQ1_TAG, MPI_COMM_WORLD);
		}

		// cleanup
		delete picasso;
	}

	// wait for everyone to finish
	MPI_Barrier(MPI_COMM_WORLD);
	
	MPI_Finalize();
	
	return 0;
}

