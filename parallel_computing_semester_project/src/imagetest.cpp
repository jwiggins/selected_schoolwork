#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <list>
#include "myrandom.h"
#include "mpilib.h"

unsigned char *mFrameBuf;
int mWidth, mHeight;
int mPatchDim;
int mRendNodeStart;
unsigned char cR,cG,cB;

using namespace std;

void RenderPatch(int x1, int y1, int x2, int y2)
{
	int rank;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		//printf("rendering patch (%d,%d)->(%d,%d)\n",x1,y1,x2,y2);
		//fflush(stdout);
	}
	
	for (int y=y1; y < y2; y++)
	{
		for (int x=x1; x < x2; x++)
		{
			mFrameBuf[(y*mWidth+x)*3+0] = cR;
			mFrameBuf[(y*mWidth+x)*3+1] = cG;
			mFrameBuf[(y*mWidth+x)*3+2] = cB;
		}
	}
}

void Run()
{
	int rank, size;
	int xPatches, yPatches, rseed;
	list<int> patches;

	// get some MPI info to start
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//
	// collectively determine which patches to render
	if (rank == 0) printf("generating work for nodes\n");
	
	// what are the dimensions of the output in patches?
	xPatches = (mWidth / mPatchDim) + ((mWidth%mPatchDim)?1:0); 
	yPatches = (mHeight / mPatchDim) + ((mHeight%mPatchDim)?1:0); 

	// seed the rng
	if (rank == mRendNodeStart)
		rseed = time(NULL);
	// limited broadcast
	my_bcast(&rseed, 1, MPI_INT, mRendNodeStart, mRendNodeStart, size-1,
									MPI_COMM_WORLD);
	// seed on all nodes
	mysrandom(rseed);
	
	// dole out the patches
	for (int i=0; i < xPatches*yPatches; i++)
	{
		int p = myrandom(mRendNodeStart, size-1);

		// if p equals our rank, we must render this patch
		if (rank == p)
			patches.push_back(i);
	}

	// get an idea of how uniform the work distribution is
	printf("[%d} %d patches\n", rank, patches.size());
	fflush(stdout);

	//if (rank == 0)
	//{
	//	printf("patch stats:\n");
	//	printf("xPatches = %d, yPatches = %d\n", xPatches,yPatches);
	//	for (list<int>::iterator i=patches.begin(); i!= patches.end(); i++)
	//		printf("%d ", *i);
	//	printf("\n");
	//	fflush(stdout);
	//}
	
	//
	// render the patches
	if (rank == 0) printf("rendering\n");
	
	// loop until all the patches are gone
	while (!patches.empty())
	{
		int p, x1,x2,y1,y2;
	 	
		// grab the first patch in the list
		p	= patches.front();
		patches.pop_front();

		// compute the corners of the patch
		x1 = (p % xPatches) * mPatchDim;
		y1 = (p / xPatches) * mPatchDim;
		x2 = x1+mPatchDim; x2 -= (x2>mWidth)?x2-mWidth:0;
		y2 = y1+mPatchDim; y2 -= (y2>mHeight)?y2-mHeight:0;
		
		// render the patch
		RenderPatch(x1,y1, x2,y2);
	}
	
	//
	// merge outputs
	if (rank == 0) printf("merging\n");

	logNMergeImage(mFrameBuf, mWidth*mHeight*3, mRendNodeStart, mRendNodeStart,
								size-1, MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
	FILE *fp;
	int rank;
	
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc < 2)
	{
		printf("usage: %s <output file>\n", argv[0]);
		exit(0);
	}

	// init variables
	mWidth = 512;
	mHeight = 512;
	mPatchDim = 10;
	mFrameBuf = (unsigned char *)malloc(mWidth*mHeight*3);
	mRendNodeStart = 2;

	srand(time(NULL));
	cR = rand() % 256;
	cG = rand() % 256;
	cB = rand() % 256;

	if (rank == mRendNodeStart)
	{
		fp = fopen(argv[1], "w");
		if (fp == NULL)
		{
			printf("Could not open output file %s\n", argv[0]);
			exit(-1);
		}
	}

	// draw!
	Run();
	
	// write the image and close the file
	if (rank == mRendNodeStart)
	{
		fprintf(fp, "P6\n%d %d\n255\n", mWidth, mHeight);
		fwrite(mFrameBuf, mWidth*mHeight*3, 1, fp);
		fclose(fp);
	}

	free(mFrameBuf);
	
	MPI_Finalize();
}

