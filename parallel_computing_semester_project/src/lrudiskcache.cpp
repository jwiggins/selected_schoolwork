#include "lrudiskcache.h"

char *mCacheDir;
void *mBlockCache;
int *mBlockIDs;

LRUDiskCache::LRUDiskCache(char *cachedir)
{
	mCacheDir = strdup(cachedir);
	mBlockCache = NULL;
	mBlockIDs = NULL;
	mMapSize = 0;
	mNumBlocks = 0;
	mDatatype = 0;

	// which processor are we?
	MPI_Comm_rank(MPI_COMM_WORLD, &mRank);
}

LRUDiskCache::~LRUDiskCache()
{
	if (mCacheDir)
		free(mCacheDir);
	if (mBlockCache)
		munmap(mBlockCache, mMapSize);
	if (mBlockIDs)
		free(mBlockIDs);
}

void LRUDiskCache::LoadDataset(char *name)
{
	char fname[256];
	
	// load the blocks
	sprintf(fname, "%s/%s_%03d.blks", mCacheDir, name, mRank);
	map_blockfile(fname);
	
	// load the block IDs
	sprintf(fname, "%s/%s.ids", mCacheDir, name);
	read_blockids(fname);

	// read the data type
	sprintf(fname, "%s/%s.info", mCacheDir, name);
	read_datatype(fname);
}

void *LRUDiskCache::FetchBlock(int block_id)
{
	void *ret = NULL;
	unsigned char *cPtr = (unsigned char *)mBlockCache;
	unsigned short *sPtr = (unsigned short *)mBlockCache;
	float *fPtr = (float *)mBlockCache;
	int index;

	if (mBlockCache && mBlockIDs)
	{
		// find out where the requested block resides
		index = find_block(block_id, 0, mNumBlocks-1);

		// set the return pointer
		switch(mDatatype)
		{
			case 1:
				ret = (void *)&cPtr[index*1000];
				break;
			case 2:
				ret = (void *)&sPtr[index*1000];
				break;
			case 4:
				ret = (void *)&fPtr[index*1000];
				break;
			default:
				printf("Bad datatype set for LRUDiskCache[%d]: %d\n", mRank, mDatatype);
				break;
		}
	}
	
	return ret;
}

int LRUDiskCache::find_block(int block_id, int start, int end)
{
	int mid = (start+end)/2;

	// base case
	if (start == mid)
	{
		if (mBlockIDs[start] == block_id)
			return start;
		else if (mBlockIDs[end] == block_id)
			return end;
		else
		{
			printf("fix LRUDiskCache::find_block()\n");
			exit(-1);
			return 0;
		}
	}
	else // recursive case
	{
		if (block_id < mBlockIDs[mid])
			return find_block(block_id, start, mid);
		else if (block_id > mBlockIDs[mid])
			return find_block(block_id, mid, end);
		else
		{
			assert(mBlockIDs[mid] == block_id);
			return mid;
		}
	}
}

void LRUDiskCache::map_blockfile(char *filename)
{
	FILE *fp;

	// open the block file
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("failed to open blockfile %s\n", filename);
		exit(-1);
	}

	// blow away any currently loaded dataset
	if (mBlockCache)
		munmap(mBlockCache, mMapSize);

	// find out how big the file is
	fseek(fp, 0, SEEK_END);
	mMapSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	// map the file
	mBlockCache = mmap((void *)0, mMapSize, PROT_READ, MAP_SHARED, fileno(fp), 0);
	if (mBlockCache == (void *)-1)
	{
		printf("failed to mmap() blockfile\n");
		printf("reason: %s\n", strerror(errno));
		exit(-1);
	}
	
	// close the file. (doesn't unmap the file)
	fclose(fp);
}

void LRUDiskCache::read_blockids(char *filename)
{
	unsigned char *buffer;
	int fsize=0;
	FILE *fp;

	// open the block id file
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("failed to open block id file %s\n", filename);
		exit(-1);
	}

	// determine the size of the file
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// allocate a buffer big enough to hold the file
	buffer = (unsigned char *)malloc(fsize);

	// read the file into the buffer
	fread(buffer, fsize, 1, fp);

	// find out how many blocks this processor has
	mNumBlocks = 0;
	for (int i=0; i < fsize; i++)
		if (buffer[i] == (unsigned char)mRank)
			mNumBlocks++;

	// allocate the block id buffer
	if (mBlockIDs)
		free(mBlockIDs);
	mBlockIDs = (int *)malloc(mNumBlocks*sizeof(int));

	// fill the block id buffer
	for (int i=0,j=0; i < fsize; i++)
		if (buffer[i] == (unsigned char)mRank)
			mBlockIDs[j++] = i;

	// free the temp buffer
	free(buffer);
	
	// close the file
	fclose(fp);
}

void LRUDiskCache::read_datatype(char *filename)
{
	FILE *fp;
	int dummy, type;

	// read the info file for the dataset
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("unable to open dataset info file %s\n", filename);
		exit(-1);
	}
	// read the info file
	// data dimensions
	fscanf(fp, "%d %d %d\n", &dummy, &dummy, &dummy);
	// block dimensions
	fscanf(fp, "%d %d %d\n", &dummy, &dummy, &dummy);
	// data type
	fscanf(fp, "%d\n", &type);

	// set the datatype
	mDatatype = type;
#ifdef _DEBUG
	//printf("disk cache datatype = %d\n", mDatatype);
	//fflush(stdout);
#endif

	// close the file
	fclose(fp);
}

