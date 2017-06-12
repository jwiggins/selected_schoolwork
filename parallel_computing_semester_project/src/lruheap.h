#ifndef LRU_HEAP_H
#define LRU_HEAP_H

#include <hash_map.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "timer.h"

struct block_ent {
	int		id;
	int		heapindx;
	void	*ptr;
};

struct heap_ent {
	unsigned int ref;
	int id;
};

class LRUHeap {
public:
																LRUHeap();
																~LRUHeap();

	void													AddBlock(int id, void *ptr);
	void	*												FetchBlock(int id);

	void													Reset(int num);

	int														Size() const { return mBlockSet.size(); }

private:

	int														heap_insert(heap_ent hent);
	int														heap_adjust(int index);
	void													remove_old_blocks(int num);

	heap_ent											*mHeap;
	hash_map<int, block_ent>			mBlockSet;
	unsigned int									mRefCount;
	int														mHeapTail, mMaxItems;
	Timer													*mSTLTimer;
};

#endif
