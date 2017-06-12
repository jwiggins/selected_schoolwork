#include "lruheap.h"

LRUHeap::LRUHeap()
: mBlockSet(1000)
{
	// init the heap
	mMaxItems = 1000;
	mHeap = (heap_ent *)malloc(mMaxItems*sizeof(heap_ent));
	memset(mHeap, 0, mMaxItems*sizeof(heap_ent));
	// heaps start at 1
	mHeapTail = 1;
	
	// zero the reference count
	mRefCount = 0;

	// init the STL Timer
	mSTLTimer = new Timer("stl timer");
}

LRUHeap::~LRUHeap()
{
	// free the heap
	free(mHeap);

	//mSTLTimer->Print();
	delete mSTLTimer;

	// iterate through the block set and free all the blocks
	for (hash_map<int, block_ent>::iterator i = mBlockSet.begin();
				i != mBlockSet.end(); ++i)
	{
		//unsigned char *ptr = (unsigned char *)(i->second).ptr;
		//int total=0;

		//for (int j=0; j<1000; j++)
		//	total += ptr[j];

		//total /= 1000;

		//printf("block average ~ %d\n", total);
		//fflush(stdout);
		
					free((i->second).ptr);
	}
}

void LRUHeap::AddBlock(int id, void *ptr)
{
#ifdef _DEBUG
	//printf("`");
	//fflush(stdout);
#endif

	// make sure a block with the same id isn't already in the heap
	// and that the heap is not full
	if (mBlockSet.count(id) == 0)
	{
		// if the heap is full, remove some blocks
		if (mHeapTail == mMaxItems)
			remove_old_blocks(10); // remove 10 blocks
	
		// proceed with the insertion
		
		// create a block_ent and a heap_ent
		block_ent bent;
		heap_ent hent;
		
		// fill the heap_ent
		hent.id = id;
		hent.ref = mRefCount++; // insert blocks at top of heap
		
		// fill the block_ent
		bent.id = id;
		bent.ptr = ptr;
		// insert into the heap
		bent.heapindx = heap_insert(hent);

		// insert the block_ent into mBlockSet
		mBlockSet[id] = bent;
	}
#ifdef _DEBUG
	//printf("\'");
	//fflush(stdout);
#endif
}

void *LRUHeap::FetchBlock(int id)
{
#ifdef _DEBUG
	//printf("<");
	//fflush(stdout);
#endif

	void *ret = NULL;

	// only return blocks that are in the set
	if (mBlockSet.count(id) != 0)
	{
		block_ent bent;
		heap_ent hent;
	
		//mSTLTimer->Start();
		// get the block
		bent = mBlockSet[id];
		//mSTLTimer->Stop();

		// update the reference count of the block
		hent = mHeap[bent.heapindx];
		hent.ref = mRefCount++;
		// tell the heap to adjust
		bent.heapindx = heap_adjust(bent.heapindx);
		
		// set the return value
		ret = bent.ptr;
	}

#ifdef _DEBUG
	//printf(">");
	//fflush(stdout);
#endif

	return ret;
}

void LRUHeap::Reset(int num)
{
	// set the new max # of items
	mMaxItems = num;
	// reset the heap
	mHeap = (heap_ent *)realloc(mHeap, mMaxItems*sizeof(heap_ent));
	memset(mHeap, 0, mMaxItems*sizeof(heap_ent));
	mHeapTail = 1;
	mRefCount = 0;

	// iterate through the block set and free all the blocks
	for (hash_map<int, block_ent>::iterator i = mBlockSet.begin();
				i != mBlockSet.end(); ++i)
					free((i->second).ptr);

	// clear the block set
	mBlockSet.clear();
	// resize the block set
	mBlockSet.resize(mMaxItems);
	
}

int LRUHeap::heap_insert(heap_ent hent)
{
	int index = mHeapTail++;
	
	// start at the bottom, percolate up
	while (index > 1)
	{
		if (mHeap[index/2].ref < mHeap[index].ref)
		{
			mHeap[index] = mHeap[index/2];
			index /= 2;
		}
		else // we found a spot for the new entry
			break;
	}
	// put the new entry in its place
	mHeap[index] = hent;

	// return the entry's index
	return index;
}

int LRUHeap::heap_adjust(int index)
{
	heap_ent tmp = mHeap[index];

	// start at the entry being updated, percolate up
	while (index > 1)
	{
		// if the parent of the current item has a lower ref,
		// replace the current item with it's parent and make the
		// parent's index the new current item
		if (mHeap[index/2].ref < mHeap[index].ref)
		{
			mHeap[index] = mHeap[index/2];
			index /= 2;
		}
		else
			break;
	}
	// the item goes here
	mHeap[index] = tmp;
	
	// return the new index
	return index;
}

void LRUHeap::remove_old_blocks(int num)
{
	// remove the last 10 blocks in the heap
	for (int i=mHeapTail-num; i < mHeapTail; i++)
	{
		// grab the heap entry
		heap_ent hent = mHeap[i];
		// and the block entry
		block_ent bent = mBlockSet[hent.id];
		
		// free the memory used by the block
		free(bent.ptr);

		// erase the block from the block set
		mBlockSet.erase(hent.id);
		// zero the heap location
		memset(&mHeap[i], 0, sizeof(heap_ent));
	}

	// reduce mHeapTail
	mHeapTail -= num;
}

