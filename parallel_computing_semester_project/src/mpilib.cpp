/*
	mpilib.cpp
	$Id: mpilib.cpp,v 1.1 2003/05/02 23:55:14 prok Exp $

	$Log: mpilib.cpp,v $
	Revision 1.1  2003/05/02 23:55:14  prok
	One long day of coding. The parallel implementation is essentially done.
	The code compiles and links, but that is all. Now the bug hunting begins.
	
*/

#include "mpilib.h"

void recursiveMergeImage(unsigned char *,int,int,MPI_Comm,int,int);
void my_bcast_util(void *,int,MPI_Datatype,int,MPI_Comm,int,int);

void logNMergeImage(unsigned char *buffer, int bufsize, int root, int start,
								int end, MPI_Comm comm)
{
	// merge
  recursiveMergeImage(buffer, bufsize, root, comm, start, end);
}

void my_bcast(void *buf, int count, MPI_Datatype dt, int root, int start, int end,
								MPI_Comm comm)
{
	my_bcast_util(buf,count,dt, root, comm, start, end);
}

/****************************************************************/

void recursiveMergeImage(unsigned char *buf, int bufsize, int cur_root,
								 MPI_Comm comm, int left, int right)
{
  int me, mid, sender;
  MPI_Status status;

  // which proc am I?
  MPI_Comm_rank(comm, &me);
  // end recursion
  if (left == right)
  {
    //printf("cur_root = %d, left = %d, right = %d, me = %d\n",
    //cur_root, left, right, me);
    return;
  }

  // where is the midpoint?
  mid = (left+right) / 2;

  // recursively merge on each half of the partition
  if (cur_root <= mid) // root is in left half, sender should be in right half
  {
    // set sender right of midpoint
    sender = mid+1;

    // split group again
    if (me <= mid)
    // we're in the left half, so split the left half (root = current root)
      recursiveMergeImage(buf, bufsize, cur_root, comm, left, mid);
    else
    // we're in the right half, so split the right half (root = sender)
      recursiveMergeImage(buf, bufsize, sender, comm, mid+1, right);
  }
  else // root is in right half. sender should be in left half
  {

    // set sender left of midpoint
    sender = left;

    // split group again
    if (me <= mid)
      // we're in the left half, so split the left half (root = sender this time)
      recursiveMergeImage(buf, bufsize, sender, comm, left, mid);
    else
      // we're in the right half, so split the right half (root = current root)
      recursiveMergeImage(buf, bufsize, cur_root, comm, mid+1, right);
  }

  // actual merging happens as stack unwinds
  if (me == sender)
  {
    //printf("send image from %d to %d\n", me, cur_root);
    // send it to the current root
    MPI_Send(buf, bufsize, MPI_CHAR, cur_root, 0, comm);
  }
  if (me == cur_root)
  {
    // receive an image
    unsigned char *ibuf = (unsigned char *)malloc(bufsize);
    MPI_Recv(ibuf, bufsize, MPI_CHAR, sender, 0, comm, &status);

    //printf("receive image from %d at %d\n", sender, me);
    
    // merge
		for (int i=0; i < bufsize; i++)
			buf[i] += ibuf[i];
    
    // cleanup
    free(ibuf);
  }
}

void my_bcast_util(void *buffer, int count, MPI_Datatype datatype,
	            int cur_root, MPI_Comm comm, int left, int right)
{
  int me, mid, dest;
  MPI_Status status;
  
  if (left == right) return;

  MPI_Comm_rank(comm, &me);

  mid = (left + right)/2;

  if (cur_root <= mid)
    dest = mid + 1;
  else
    dest = left;

  if (me == cur_root)
    MPI_Send(buffer, count, datatype, dest, 0, comm);

  if (me == dest)
    MPI_Recv(buffer, count, datatype, cur_root, 0, comm, &status);

  if (cur_root <= mid)
	{
    if (me <= mid)
      my_bcast_util(buffer, count, datatype, cur_root, comm, left, mid);
    else		     
      my_bcast_util(buffer, count, datatype, dest, comm, mid+1, right);
  }
  else
	{
    if (me <= mid)
      my_bcast_util(buffer, count, datatype, dest, comm, left, mid);
    else		     
      my_bcast_util(buffer, count, datatype, cur_root, comm, mid+1, right);
  }

  return;
}

