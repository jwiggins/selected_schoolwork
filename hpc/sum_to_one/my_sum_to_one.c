/*
  John Wiggins <prok@mail.utexas.edu>
  Jason Bradford <jbrad@cs.utexas.edu>
*/
#include "mpi.h"

void my_sum_to_one_util( void *src, void *dst, int cnt, MPI_Datatype datatype,
				int cur_root, MPI_Comm comm, int left, 
				int right, int level);

int my_sum_to_one( void * send_buf, void * recv_buf, int count,
                  int root, MPI_Comm comm)
{

  int nprocs;

  MPI_Comm_size( comm, &nprocs );

  my_sum_to_one_util( send_buf, recv_buf, count, MPI_DOUBLE, root, comm, 
			0, nprocs-1 , 0);

  return 0;
}

void my_sum_to_one_util( void *src, void *dst, int cnt, MPI_Datatype datatype,
                            int cur_root, MPI_Comm comm, int left, int right, int level)
{
  int me, mid, sender, i;
  MPI_Status status;

  /* which proc am I? */
  MPI_Comm_rank( comm, &me);
  /* end recursion */
  if (left == right)
  {
    /*printf("my_sum_to_one_util():\n");*/
    /*printf("cur_root = %d, left = %d, right = %d, me = %d\n", cur_root, left, right, me);*/
    return;
  }

  /* where is the midpoint? */
  mid = (left+right) / 2;

  /* recursively sum-to-one on each half of the partition */
  if (cur_root <= mid) /* root is in left half, sender should be in right half */
  {
    /* set sender right of midpoint */
    sender = mid+1;

    /* split group again */
    if (me <= mid) /* we're in the left half, so split the left half (root = current root)*/
      my_sum_to_one_util(src, dst, cnt, datatype, cur_root, comm, left, mid, level+1);
    else           /* we're in the right half, so split the right half (root = sender)*/
      my_sum_to_one_util(src, dst, cnt, datatype, sender, comm, mid+1, right, level+1);
  }
  else /* root is in right half. sender should be in left half */
  {
    /* set sender left of midpoint */
    sender = left;

    /* split group again */
    if (me <= mid) /* we're in the left half, so split the left half (root = sender this time) */
      my_sum_to_one_util(src, dst, cnt, datatype, sender, comm, left, mid, level+1);
    else           /* we're in the right half, so split the right half (root = current root) */
      my_sum_to_one_util(src, dst, cnt, datatype, cur_root, comm, mid+1, right, level+1);
  }

  /*printf("my_sum_to_one_util():\n");
  printf("cur_root = %d, left = %d, right = %d, me = %d, sender = %d, level = %d\n",
       cur_root, left, right, me, sender, level);*/

  if ( me == sender )
  {
    /*printf("send: cur_root = %d, sender = %d, level = %d\n", cur_root, sender, level);*/
    MPI_Send( src, cnt, datatype, cur_root, 0, comm );
  }
  if ( me == cur_root )
  {

    MPI_Recv( dst, cnt, datatype, sender, MPI_ANY_TAG, comm, &status );

    /* sum */
    if (level > 0)
    {
      for ( i=0; i < cnt; i++ )
        ((double *)src)[i] += ((double *)dst)[i];
    }
    else /* level == 0, therefore this is the last combine */
    {
      for ( i=0; i < cnt; i++ )
        ((double *)dst)[i] += ((double *)src)[i];
    }

    /*printf("recv: cur_root = %d, sender = %d, level = %d\n", cur_root, sender, level);
    for(i=0; i < cnt; i++)
      printf("src[%d] = %f\n", i, s[i]);*/
  }
}
