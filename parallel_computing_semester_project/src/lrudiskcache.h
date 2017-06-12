#ifndef LRU_DISK_CACHE_H
#define LRU_DISK_CACHE_H

#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

class LRUDiskCache {
public:
												LRUDiskCache(char *cachedir);
												~LRUDiskCache();

	void									LoadDataset(char *name);

	void	*								FetchBlock(int block_id);
	int										BlockSize() const { return mDatatype*1000; }

private:

	int										find_block(int block_id, int start, int end);

	void									map_blockfile(char *filename);
	void									read_blockids(char *filename);
	void									read_datatype(char *filename);

	char									*mCacheDir;
	void									*mBlockCache;
	int										*mBlockIDs;
	size_t								mMapSize;
	int										mNumBlocks, mDatatype, mRank;
};

#endif

