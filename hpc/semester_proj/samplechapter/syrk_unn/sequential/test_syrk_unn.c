#include <math.h>
#include <stdio.h>
#include <time.h>

#include "FLAME.h"

#define dabs( x ) ( (x) < 0 ? -(x) : (x) )

int debugging = 0;

extern double dclock();

int main(int argc, char *argv[])
{
  int 
    m_first, m_last, m_inc,
    m_eq_n,
    m, n, nb_alg,
    m_A, n_A, 
    m_C, n_C, 
    iter,
    repeat, irep,
    variant;

  double
    dtime, mflops,
    dtime_ref, dtime_flame, dtime_unblk, dtime_blk, dtime_rec,
    d_one = 1.0,
    diff,
    diff_flame, diff_unblk, diff_blk, diff_rec,
    matrix_compare();

  char 
    data[20];
  
  FLA_Obj
    A, C, Cold, Cref;
  
  FLA_Init( );

  dtime = dclock();
  sleep( 1 );
  dtime = dclock() - dtime;
  if ( dabs( dtime - 1 ) >= 0.01 )
    printf("callibrate clock: elapsed time = %lf\n", dtime );
      

  printf("%c number of repeats:", '%');
  scanf("%d", &repeat );

  printf("\nfigure\n\n");

  while ( TRUE ){
    /* We will time all m
       from  mfirst through mlast in 
       increments of minc */

    printf("%c enter mfirst, mlast, minc:", '%' );
    scanf( "%d", &m_first );

    if ( m_first == -1 ) break;

    scanf( "%d", &m_last );
    scanf( "%d", &m_inc );

    printf("%c enter n: (-1 means n = m)", '%' );
    scanf("%d", &n);
    if ( n == -1 ){
      printf("%c we will take m=n in these experiments\n", '%' );
      m_eq_n = TRUE;
    }
    else
      m_eq_n = FALSE;

    printf( "%c enter nb_alg:", '%'  );
    scanf( "%d", &nb_alg );

    printf( "%c enter variant: (0=up-left wrt C, 1=down-right wrt C)", '%'  );
    scanf( "%d", &variant );

    printf("%c Performance in MFLOPS/sec (1e6 flops per second)\n\n", '%' );
    printf("%c  m    n  nb    REF FLAME   diff      unb      diff    blk     diff   rec    diff\n", '%' );
    printf("%c ==================================================================================\n", '%' );

    switch ( variant ) {
    case 0:
      sprintf( data, "data_upleft" );
      break;

    case 1:
      sprintf( data, "data_downright" );
      break;
    }

    printf("%s = [\n", data );

    iter = 0;

    for ( m=m_first; m<=m_last; m+=m_inc ){
      {
	if ( m_eq_n )
	  n = m;

	m_A = m;
	n_A = n;
	m_C = m;
	n_C = m;
    
	/* Allocate space for the three matrices */
    
	FLA_Obj_create( FLA_DOUBLE, m_A, n_A, &A );
	FLA_Obj_create( FLA_DOUBLE, m_C, n_C, &C );
	FLA_Obj_create( FLA_DOUBLE, m_C, n_C, &Cold );
	FLA_Obj_create( FLA_DOUBLE, m_C, n_C, &Cref );
    
	mflops = 1.0 * m_C * n_C * n_A;

	/* Generate random matrices A and C */
      
	fill_matrix( A );
	fill_matrix( C );
	  
	FLA_Copy( C, Cref );
	FLA_Copy( C, Cold );

	for ( irep = 0 ; irep < repeat; irep++ ){

	  FLA_Copy( Cold, Cref );

	  /* Time reference implementation */
	  
	  dirty_cache();

	  dtime = dclock();

	  cblas_dsyrk( CblasColMajor,
		       CblasUpper,
		       CblasNoTrans,
		       m, n, 
		       d_one, 
		       ( double * ) A.buffer, A.ldim, 
		       d_one,
		       ( double * ) Cref.buffer, Cref.ldim ); 
	  
	  dtime = dclock() - dtime;
	  
	  if ( irep == 0 )
	    dtime_ref = dtime;
	  else
	    dtime_ref = ( dtime_ref < dtime ? dtime_ref : dtime );

	  /* Time FLAME implementation */

	  dirty_cache();

	  dtime = dclock();
	  
	  FLA_Syrk( FLA_UPPER_TRIANGULAR, FLA_NO_TRANSPOSE,
		    ONE, A, ONE, C ); 
	
	  dtime = dclock() - dtime;
      
	  if ( irep == 0 )
	    dtime_flame = dtime;
	  else
	    dtime_flame= ( dtime_flame < dtime ? dtime_flame : dtime );

	  if ( irep == 0 ) 
	    diff_flame = matrix_compare( C.m, C.n, C.buffer, C.ldim,
				   Cref.buffer, Cref.ldim );
	
	  /* Time your level2 BLAS implementation */

	  /* Note: this takes too long for large problems, 
             so we skip */

	  if ( m <= 500 ){
	    FLA_Copy( Cold, C );
      
	    dirty_cache();

	    dtime = dclock();
      
	    switch ( variant ) {
	    case 0:
	      Syrk_un_upleft_wrt_C_unb( A, C );
	      break;

	    case 1:
	      Syrk_un_downright_wrt_C_unb( A, C );
	      break;
	    }

	    dtime = dclock() - dtime;
      
	    if ( irep == 0 )
	      dtime_unblk = dtime;
	    else
	      dtime_unblk= ( dtime_unblk < dtime ? dtime_unblk : dtime );

	    if ( irep == 0 ) 
	      diff_unblk = matrix_compare( C.m, C.n, C.buffer, C.ldim,
					   Cref.buffer, Cref.ldim );
	  }

	  /* Time your blocked implementation */

	  FLA_Copy( Cold, C );
      
	  dirty_cache();

	  dtime = dclock();
      
	  switch ( variant ) {
	  case 0:
	    Syrk_un_upleft_wrt_C_blk( FLA_NONRECURSIVE, A, C, nb_alg );
	    break;

	  case 1:
	    Syrk_un_downright_wrt_C_blk( FLA_NONRECURSIVE, A, C, nb_alg );
	    break;
	  }
	
	  dtime = dclock() - dtime;
      
	  if ( irep == 0 )
	    dtime_blk = dtime;
	  else
	    dtime_blk= ( dtime_blk < dtime ? dtime_blk : dtime );

	  if ( irep == 0 ) 
	    diff_blk = matrix_compare( C.m, C.n, C.buffer, C.ldim,
				       Cref.buffer, Cref.ldim );

	  /* Time your recursive implementation */

	  FLA_Copy( Cold, C );
      
	  dirty_cache();

	  dtime = dclock();
      
	  switch ( variant ) {
	  case 0:
	    Syrk_un_upleft_wrt_C_blk( FLA_RECURSIVE, A, C, nb_alg );
	    break;

	  case 1:
	    Syrk_un_downright_wrt_C_blk( FLA_RECURSIVE, A, C, nb_alg );
	    break;
	  }
	
	  dtime = dclock() - dtime;
      
	  if ( irep == 0 )
	    dtime_rec = dtime;
	  else
	    dtime_rec= ( dtime_rec < dtime ? dtime_rec : dtime );

	  if ( irep == 0 ) 
	    diff_rec = matrix_compare( C.m, C.n, C.buffer, C.ldim,
				       Cref.buffer, Cref.ldim );
	}

	printf("%4d %4d %3d  ", m, n, nb_alg );

	/* print MFLOPS for reference implementation */
	printf( "%5.1lf  ", mflops / dtime_ref / 1.0e6 );

	/* print MFLOPS for flame implementation */
	printf( "%5.1lf  ", mflops / dtime_flame / 1.0e6 );

	/* print difference in result for flame implementation */
	printf( "%7.1le  ", diff_flame );

	if ( m <= 500 ){
	  iter++;

	  /* print MFLOPS for unblocked implementation */
	  printf( "%5.1lf  ", mflops / dtime_unblk / 1.0e6 );

	  /* print difference in result for unblocked implementation */
	  printf( "%7.1le  ", diff_unblk );
	}
	else{
	  /* print MFLOPS for unblocked implementation */
	  printf( "%5.1lf  ", 0.0 );

	  /* print difference in result for unblocked implementation */
	  printf( "%7.1le  ", 0.0 );
	}

	/* print MFLOPS for blocked implementation */
	printf( "%5.1lf  ", mflops / dtime_blk / 1.0e6 );

	/* print difference in result for blocked implementation */
	printf( "%7.1le  ", diff_blk );

	/* print MFLOPS for recursive implementation */
	printf( "%5.1lf  ", mflops / dtime_rec / 1.0e6 );

	/* print difference in result for recursive implementation */
	printf( "%7.1le\n", diff_rec );
      }

      FLA_Obj_free( &A );
      FLA_Obj_free( &C );
      FLA_Obj_free( &Cold );
      FLA_Obj_free( &Cref );
    }
    
    printf("]\n");

    printf("plot( %s( :, 1 ), %s( :, 4 ), '-', ... \n", data, data );
    printf("      %s( :, 1 ), %s( :, 5 ), '-.', ... \n", data, data );
    printf("      %s( 1:%d, 1 ), %s( 1:%d, 7 ), '--x', ... \n", data, iter, data, iter);
    printf("      %s( :, 1 ), %s( :, 9 ), '--o', ... \n", data, data );
    printf("      %s( :, 1 ), %s( :, 11 ), '--+' )\n", data, data );
    
    printf("legend( 'Reference', 'FLAME', 'Unblocked', 'Blocked', 'Recursive', 1 )\n");
    // printf("legend( 'Reference', 'Unblocked', 'Blocked', 'Recursive', 1 )\n");

    printf("axis( [0, %d, 0, 650 ] )\n", m_last );
    printf("grid on\n" );

    switch ( variant ){
    case 0:
      printf("title( 'C <- A A^T + C (up-left wrt C) ", nb_alg );
      break;

    case 1:
      printf("title( 'C <- A A^T + C (down-right wrt C) ", nb_alg );
      break;
    }

    if ( m_eq_n )
      printf("nb = %d' )\n", nb_alg );
    else
      printf("'k = %d, nb = %d' )\n", n, nb_alg );

    if ( m_eq_n )
      printf("xlabel( 'n = k' )\n");
    else
      printf("xlabel( 'n' )\n");

    printf("ylabel( 'MFLOPS/sec.' )\n");
    
    switch ( variant ){
    case 0:
      printf( "print -depsc2 syrk_un_upleft_wrt_C_%d.eps\n", nb_alg );
      break;

    case 1:
      printf( "print -depsc2 syrk_un_downright_wrt_C_%d.eps\n", nb_alg );
      break;
    }
  }
  
  FLA_Finalize( );
}

#define A(i,j) a[ (j)*lda + (i) ]
#define B(i,j) b[ (j)*ldb + (i) ]

double max_entry( int m, int n, double *a, int lda )
{
  double 
    d_max = 0.0;
  int
    i, j;
  
  for ( j=0; j<n; j++)
    for (i=0; i<m; i++ )
      if ( dabs( A(i,j) ) > d_max ) 
	d_max = dabs( (double) A(i,j) );
  
  return d_max;
}

double matrix_compare( int m, int n, double *a, int lda, double *b, int ldb )
{
  double 
    d_max = 0.0;
  int
    i, j;
  
  for ( j=0; j<n; j++)
    for (i=0; i<m; i++ )
      if ( dabs( (double) ( A(i,j) - B(i,j) ) ) > d_max ) 
	d_max = dabs( (double) ( A(i,j) - B(i,j) ) );
  
  return d_max;
}
