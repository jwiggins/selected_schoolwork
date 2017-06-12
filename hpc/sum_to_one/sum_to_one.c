#include <stdio.h>
#include "mpi.h"

#define N 10    /* Size of the vector */
#define ROOT 0   /* Root of the sum-to-one */

int my_sum_to_one( void * send_buf, void * recv_buf, int count,
                   int root, MPI_Comm comm);

int main( int argc, char **argv )
{
    int rank;
    double data_in[N], data_out[N];
    int i;

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    for (i=0; i<N; i++) {
      data_in[i] = (double) i;
      data_out[i] = (double) -1;
    }

    my_sum_to_one( data_in, data_out, N, ROOT, MPI_COMM_WORLD );
    
    if (rank == ROOT) {
      printf("Process %d printing result vector:\n", rank);
      for (i=0; i<N; i++)
	printf("result[%d] = %f\n", i, data_out[i]);
    }

    MPI_Finalize( );

    return 0;
}

