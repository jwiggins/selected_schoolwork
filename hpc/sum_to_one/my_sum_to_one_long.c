/*
  John Wiggins <prok@mail.utexas.edu>
  Jason Bradford <jbrad@cs.utexas.edu>
*/

#include "mpi.h"

void my_reduce_scatter( void *send_buf, int send_size, MPI_Datatype send_datatype,
	        void *recv_buf, int recv_size, MPI_Datatype recv_datatype,
                MPI_Comm comm )
{
  /* our vars */
  int me, nprocs, left, right, index, index_next, i,j, typesize;
  char *recv_location, **send_location;
  /* MPI spoo */
  MPI_Status status;
  MPI_Request request;

  if ( send_datatype != recv_datatype ){
    printf(" send_datatype != recv_datatype not yet implemented\n" );
    exit ( 0 );
  }

  MPI_Comm_rank( comm, &me );
  MPI_Comm_size( comm, &nprocs );

  MPI_Type_size( send_datatype, &typesize );

  /* recv stuff */
  recv_location = ( char * ) malloc( recv_size * typesize );

  /* send stuff */
  /* allocate the send addresses */
  send_location = ( char ** ) malloc( (nprocs + 1 ) * sizeof( char * ) );
  /* set the first location */
  send_location[0] = (char *) send_buf;
  /* fill in rest of addresses */
  for ( i=0; i < nprocs; i++ )
    send_location[ i+1 ] = send_location[i] + (send_size * typesize);

  /* left and right of me */
  left = ( me-1+nprocs )%nprocs;
  right = ( me+1 )%nprocs;
  
  /* start off at item me+1 */
  index = (me+1) % nprocs;
  for ( i=1; i<nprocs; i++ )
  {
    index_next = ( index+1 ) %nprocs;

    /*printf("[%d] Receive from %d. index = %d, next_index = %d, size = %d\n",
       me, right, index, index_next, recv_size * typesize);*/

    MPI_Irecv( recv_location,
	     recv_size * typesize,
	     MPI_CHAR, right, MPI_ANY_TAG, comm, &request );

    /*printf("[%d] Send to %d. size = %d\n", me, left, send_size * typesize);*/
    MPI_Send( send_location[ index ],
	     send_size * typesize,
	     MPI_CHAR, left, 0, comm );

    /* wait for receive to complete  */
    MPI_Wait( &request, &status );

    /* do the reduce operation (addition) */
    /* add the contents of recv_location to send_location[index_next] */
    for (j=0; j < recv_size; j++)
    {
      if (i == (nprocs-1))
        ((double *)recv_buf)[(index_next*send_size) + j] =
           ((double *)send_location[index_next])[j] + ((double *)recv_location)[j];
      else
        ((double *)send_location[index_next])[j] += ((double *)recv_location)[j];
    }
    /* advance the index pointer  */
    index = index_next;
  }

 /* MPI_Gather (void* send_buf,int send_size,MPI_Datatype send_datatype, void* recv_buf, int recv_size,MPI_Datatype recv_datatype,int me, MPI_Comm comm); */


  free ( recv_location );
  free ( send_location );

  return;
}

void my_sum_to_one( void * send_buf, void * recv_buf, int count,
                  int root, MPI_Comm comm)
{

  int rank,size, typesize, sendsize;

  MPI_Comm_rank( comm, &rank );
  MPI_Comm_size( comm, &size );
  MPI_Type_size( MPI_DOUBLE, &typesize );

  sendsize = count/size;

  my_reduce_scatter( send_buf, sendsize, MPI_DOUBLE,
                    recv_buf, sendsize, MPI_DOUBLE,
                    comm );

  MPI_Gather(recv_buf+rank*(sendsize)*typesize, sendsize, MPI_DOUBLE,
            recv_buf, sendsize, MPI_DOUBLE, root, comm);

}

#if 0
#define N 10

int main( int argc, char **argv )
{
    int rank, size;
    double data_in[N], data_out[N];
    int i;

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    for (i=0; i<N; i++) {
      data_in[i] = (double) i;
      data_out[i] = (double) -1;
    }

    my_sum_to_one( data_in, data_out, N, 0, MPI_COMM_WORLD );

    if (rank == 0) {
      printf("Process %d printing result vector:\n", rank);
      for (i=0; i<N; i++)
        printf("result[%d] = %f\n", i, data_out[i]);
    }

    MPI_Finalize( );

    return 0;
}

#endif