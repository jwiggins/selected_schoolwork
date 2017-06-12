#include "PLA.h"

#define datatype MPI_DOUBLE

int main(int argc, char *argv[])
{
  MPI_Comm comm;
  PLA_Template template = NULL;
  PLA_Obj  A = NULL,           B = NULL,   Bref=NULL,
           MINUS_ONE = NULL,  ZERO = NULL, ONE = NULL,
           Bref_msc = NULL, B_msc = NULL, A_msc = NULL,
           add_to_diag = NULL;
  double   time, flops;
  double   *buff_add_to_diag;
  int      m, n, nb_distr, nb_alg, me, nprocs, dummy, info = 0;
  int      nprows, npcols, ierror;
  int      iside, side, iuplo, uplo, itrans, trans;
  double   diff, diff_max, PLA_Local_abs_diff();

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &me);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);


  if (0 == me) {
    printf("enter nprows, npcols:\n");
    scanf("%d %d", &nprows, &npcols );
    printf("enter m, n, , distr. block size, and alg. block size:\n");
    scanf("%d %d %d %d", &m, &n, &nb_distr, &nb_alg );
    printf("enter side: (0=left, 1=right)");
    scanf("%d", &iside );
    printf("enter uplo: (0=lower, 1=upper)");
    scanf("%d", &iuplo );
    printf("enter trans: (0=notrans, 1=trans)");
    scanf("%d", &itrans );
    printf("Turn on error checking? (1 = YES, 0 = NO):");
    scanf("%d", &ierror );
    printf("\n");
  }

  MPI_Bcast(&nprows,   1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&npcols,   1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&m,        1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n,        1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&nb_distr, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&nb_alg,   1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&iside,    1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&iuplo,    1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&itrans,   1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&ierror, 1, MPI_INT, 0, MPI_COMM_WORLD);

  side =  ( iside ==  0 ? PLA_SIDE_LEFT : PLA_SIDE_RIGHT );
  uplo =  ( iuplo ==  0 ? PLA_LOWER_TRIANGULAR : PLA_UPPER_TRIANGULAR );
  trans = ( itrans == 0 ? PLA_NO_TRANSPOSE : PLA_TRANSPOSE );

  if ( ierror ) 
    PLA_Set_error_checking( ierror, TRUE, TRUE, FALSE );
  else
    PLA_Set_error_checking( ierror, FALSE, FALSE, FALSE );

  PLA_Comm_1D_to_2D(MPI_COMM_WORLD, nprows, npcols, &comm);

  PLA_Init(comm);

  PLA_Temp_create( nb_distr, 0, &template );

  PLA_Matrix_create(  datatype, 
		      ( iside == 0 ? m : n ), ( iside == 0 ? m : n ),
                      template, PLA_ALIGN_FIRST, PLA_ALIGN_FIRST, &A );

  PLA_Matrix_create(  datatype, m, n, 
                      template, PLA_ALIGN_FIRST, PLA_ALIGN_FIRST, &B );

  PLA_Matrix_create(  datatype, m, n, 
                      template, PLA_ALIGN_FIRST, PLA_ALIGN_FIRST, &Bref );

  /* Create usual duplicated scalar constants with same datatype and
     template as A_panel */

  PLA_Create_constants_conf_to( A, &MINUS_ONE, &ZERO, &ONE );

  PLA_Mscalar_create_conf_to( ONE, PLA_ALL_ROWS, PLA_ALL_COLS, &add_to_diag );

  /* Create random matrices */

  create_random_data( A );

  PLA_Obj_local_buffer( add_to_diag, ( void ** ) &buff_add_to_diag );
  *buff_add_to_diag = ( double ) ( iside == 0 ? m : n );

  PLA_Obj_add_to_diagonal( A, add_to_diag );

  create_random_data( B );

  PLA_Copy( B, Bref );


  MPI_Barrier( MPI_COMM_WORLD );
  time = MPI_Wtime( );

  if ( iside == 0 ){
    if ( iuplo == 0 ){
      if ( itrans == 0 )
	PLA_Trsm_left_lower_notrans( PLA_NONUNIT_DIAG, A, B, nb_alg );
      else
	PLA_Trsm_left_lower_trans( PLA_NONUNIT_DIAG, A, B, nb_alg );
    }
    else{
      if ( itrans == 0 )
	PLA_Trsm_left_upper_notrans( PLA_NONUNIT_DIAG, A, B, nb_alg );
      else
	PLA_Trsm_left_upper_trans( PLA_NONUNIT_DIAG, A, B, nb_alg );
    }
  }
  else {
    if ( iuplo == 0 ){
      if ( itrans == 0 )
	PLA_Trsm_right_lower_notrans( PLA_NONUNIT_DIAG, A, B, nb_alg );
      else
	Trsm_right_lower_trans( A, B, nb_alg );
    }
    else{
      if ( itrans == 0 )
	PLA_Trsm_right_upper_notrans( PLA_NONUNIT_DIAG, A, B, nb_alg );
      else
	PLA_Trsm_right_upper_trans( PLA_NONUNIT_DIAG, A, B, nb_alg );
    }
  }

//  PLA_Trsm( side, uplo, trans, PLA_NONUNIT_DIAG,
//   	    ONE, A, B );


  MPI_Barrier( MPI_COMM_WORLD );

  time = MPI_Wtime() - time;

  flops = 1.0 * m * ( iside == 0 ? m : n ) * n;

  if ( me == 0 )
    printf("m = %d, n = %d, time = %lf, MFLOPS/node = %lf\n", 
	   m, n, time, flops / time * 1.0e-6 / nprocs );

  PLA_Trsm( side, uplo, trans, PLA_NONUNIT_DIAG,
	   ONE, A, Bref );

  diff = PLA_Local_abs_diff( B, Bref );

  MPI_Reduce( &diff, &diff_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );

  if ( me == 0 )
    printf("diff = %le\n", diff_max );

  PLA_Obj_free(&A);            
  PLA_Obj_free(&B);            
  PLA_Obj_free(&Bref );
  PLA_Obj_free(&MINUS_ONE);            
  PLA_Obj_free(&ZERO);
  PLA_Obj_free(&ONE);            
  PLA_Obj_free(&add_to_diag);            

  PLA_Temp_free(&template);
  PLA_Finalize( );
  MPI_Finalize( );
}
