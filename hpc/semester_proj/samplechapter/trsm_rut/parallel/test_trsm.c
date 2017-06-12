/*
   PLAPACK Release 3.0
   
   Copyright (C) 2000
   Robert A. van de Geijn and The University of Texas at Austin

   For GNU-license details see the file GNU_license in the 
   PLAPACK root directory.
*/

#include "mpi.h"
#include "PLA.h"

int main(int argc, char *argv[])
{
  MPI_Comm 
    comm = MPI_COMM_NULL;
  MPI_Datatype
    datatype;
  PLA_Template 
    templ = NULL;
  PLA_Obj  
    A      = NULL,
    Amsc   = NULL,
    B      = NULL, 
    Bmsc   = NULL,
    Bmsc2   = NULL,
    MINUS_ONE = NULL, ZERO   = NULL, ONE  = NULL;
  int      
    n,
    nb_distr, nb_alg,
    error, parameters, sequential,
    me, nprocs, 
    itype;
  double 
    time,
    flops,
    diff,
    local_matrix_compare();

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &me);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (me==0) {
    /* The distribution block size is the size of the blocks
       into which the matrix is blocked and then distributed */
    printf("enter distr. block size:\n");
    scanf("%d", &nb_distr );
    printf("nb_distr = %d\n", nb_distr );

    /* The algorithmic block size is the block size "b"
       that you encountered in the derivation of your 
       blocked algorithm */
    printf("enter alg. block size:\n");
    scanf("%d", &nb_alg );
    printf("nb_alg = %d\n", nb_alg );
  }

  /* Broadcast the information so all processors know it */
  MPI_Bcast(&nb_distr, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&nb_alg,   1, MPI_INT, 0, MPI_COMM_WORLD);

  /* A PLAPACK routine that sets the algorithmic block
     size used by all the PLAPACK BLAS */
  pla_Environ_set_nb_alg( PLA_OP_ALL_ALG, nb_alg );

  /* Set the error checking parameters */
  PLA_Set_error_checking( TRUE, TRUE, FALSE, FALSE );

  /* Create a 2D view of the processors with a ratio of
     rows and column of the mesh that is approximately 1.0 */
  PLA_Comm_1D_to_2D_ratio(MPI_COMM_WORLD, 1.0, &comm);

  /* Initialize PLAPACK */
  PLA_Init(comm);
    
  /* Create a template for distributing matrices and vectors */
  PLA_Temp_create( nb_distr, 0, &templ );
    
  while ( TRUE ){
    if (me==0) {
      printf("enter datatype:\n");
      printf("-1 = quit\n");
      printf(" 0 = float\n");
      printf(" 1 = double\n");
      printf(" 2 = complex\n");
      printf(" 3 = double complex\n");
      scanf("%d", &itype );
      printf("itype = %d\n", itype );
    }
    MPI_Bcast(&itype, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if ( itype == -1 ) break;
    switch( itype ){
    case 0:
      datatype = MPI_FLOAT;
      break;
    case 1:
      datatype = MPI_DOUBLE;
      break;
    case 2:
      /*
      datatype = MPI_COMPLEX;
      break;
    case 3:
      datatype = MPI_DOUBLE_COMPLEX;
      break;
      */
    default:
      PLA_Abort( "unknown datatype", __LINE__, __FILE__ );
    }

    /* Get the problem size.  For now, we assume all matrices
       to be square */
    if (me==0) {
      printf("enter n:\n");
      scanf("%d", &n );
      printf("n = %d\n", n );
    }
    MPI_Bcast(&n,     1, MPI_INT, 0, MPI_COMM_WORLD);

    /* Create matrices A and B */
    PLA_Matrix_create( datatype, 
		        n, 
		        n,
			templ, 
                        PLA_ALIGN_FIRST, 
                        PLA_ALIGN_FIRST, 
                        &A ); 

    PLA_Matrix_create( datatype, 
		        n, 
		        n,
			templ, 
                        PLA_ALIGN_FIRST, 
                        PLA_ALIGN_FIRST, 
                        &B ); 

    /* Fill the matrices with random numbers */
    create_problem( A );
    create_problem( B );

    /* If your operation is TRSM, uncomment the following
       to make sure your problem is numerically stable */
    {
      double d_n;

      d_n = (double) n;
      PLA_Shift( A, MPI_DOUBLE, &d_n );
    }

    /* Create a "multiscalar"s that can hold the entire matrices
       A and B on each processor. */
    PLA_Mscalar_create_conf_to( A, PLA_ALL_ROWS, PLA_ALL_COLS, &Amsc );
    PLA_Mscalar_create_conf_to( B, PLA_ALL_ROWS, PLA_ALL_COLS, &Bmsc );

    /* Copy the contents of original matrices A and B to Amsc and Bmsc */
    PLA_Copy( A, Amsc );
    PLA_Copy( B, Bmsc );

    /* Create the usual constants -1, 0, 1.  Note: in FLAME
       these are predefined */
    PLA_Create_constants_conf_to( A, &MINUS_ONE, &ZERO, &ONE );

    /* Start the timer */
    MPI_Barrier( MPI_COMM_WORLD );
    time = MPI_Wtime ();

    /* Call the routine to be tested and timed */

    /* Notice that I start by calling the PLAPACK
       provided call for the operation */

    PLA_Trsm( PLA_SIDE_RIGHT, PLA_UPPER_TRIANGULAR, 
	      PLA_TRANSPOSE, PLA_NONUNIT_DIAG,
	      ONE, A, B );

    /* After it all works for the call to the
       PLAPACK version, uncomment the call below. */
    
/*
    Trsm_right_upper_trans_blk();
    Trmm_lln_rowlazy_wrt_L_rec( A, B, nb_alg );
*/

    /* Stop the timer */
    MPI_Barrier( MPI_COMM_WORLD );
    time = MPI_Wtime () - time;

    flops = 1.0 * n * n * n;

    if ( me == 0 ) 
      printf("%d time = %f, MFLOPS/node = %10.4lf \n", n, time,
	       flops / time * 1.0e-6 / nprocs );

    /* Copy the result into a mscalar so all processors have a copy */
    PLA_Mscalar_create_conf_to( B, PLA_ALL_ROWS, PLA_ALL_COLS, &Bmsc2 );

    PLA_Copy( B, Bmsc2 );

    /* Factor the copy of the original matrix B */
    PLA_Local_trsm( PLA_SIDE_RIGHT, PLA_UPPER_TRIANGULAR, 
		   PLA_TRANSPOSE, PLA_NONUNIT_DIAG,
		   ONE, Amsc, Bmsc );

    if ( n <= 10 ){
      /* Subtract the copy of the result computed in parallel
	 from the copy computed locally */
      PLA_Local_axpy( MINUS_ONE, Bmsc2, Bmsc );

      /* Show the contents of the difference (should be a zero matrix) */
      PLA_Global_show( "Result=[", Bmsc, "%lf ", "]" );
    }
    else{
      diff = local_matrix_compare( Bmsc, Bmsc2 );
      if ( diff > .00000001 )
	printf("diff = %lf\n", diff );
    }
  }

  
  /* Free all the PLA_Objs */
  PLA_Obj_free( &A );
  PLA_Obj_free( &Amsc );
  PLA_Obj_free( &B );
  PLA_Obj_free( &Bmsc );
  PLA_Obj_free( &Bmsc2 );
  PLA_Obj_free( &MINUS_ONE );
  PLA_Obj_free( &ZERO );
  PLA_Obj_free( &ONE );

  /* Free the template for distribution */
  PLA_Temp_free(&templ);

  /* Finalize PLAPACK */
  PLA_Finalize( );

  /* Finalize MPI */
  MPI_Finalize( );
}

#define AA(i,j) ap[ j*ldim_A + i ]
#define BB(i,j) bp[ j*ldim_B + i ]

double local_matrix_compare( PLA_Obj A, PLA_Obj B )
{
  int local_m, local_n, i, j, ldim_A, ldim_B;
  double result=0.0;
  MPI_Datatype datatype;

  PLA_Obj_local_length( A, &local_m );
  PLA_Obj_local_width( A, &local_n );
  PLA_Obj_local_ldim( A, &ldim_A );
  PLA_Obj_local_ldim( B, &ldim_B );
  PLA_Obj_datatype( A, &datatype );

  if ( datatype == MPI_DOUBLE ){
    double *ap, *bp;

    PLA_Obj_local_buffer( A, ( void ** ) &ap );
    PLA_Obj_local_buffer( B, ( void ** ) &bp );

    for ( j=0; j<local_n; j++ )
      for ( i=0; i<local_m; i++ )
	if ( dabs( AA( i,j ) - BB( i,j ) ) > result )
	  result = dabs( AA( i,j ) - BB( i,j ) );
  }
  else{
    printf("datatype not yet implemented\n");
  }

  return result;
}


