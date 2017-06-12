#include <math.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "FLAME.h"

#define dabs( x ) ( (x) < 0 ? -(x) : (x) )

extern int version;

int debugging = 0;
extern perform_copy;

extern double dclock();

#define min(x,y) ( (x) < (y) ? (x) : (y) )

#define OP_SYMM  0
#define OP_SYRK  1
#define OP_SYR2K 2
#define OP_TRMM  3
#define OP_TRSM  4

int main(int argc, char *argv[])
{
    int 
	m_first, m_last, m_inc,
	n_first, n_last, n_inc,
	m_eq_n,
	m, n, nb_alg,
	m_A, n_A, 
	m_B, n_B, 
	iter,
    algo,
	//    iop, iside, iuplo, itransa, idiag,
	repeat, irep;

    double
	dtime, mflops,
	dtime_ref, dtime_flame, dtime_unblk, dtime_blk, dtime_rec,
	d_one = 1.0,
	diff,
	diff_flame, diff_unblk, diff_blk, diff_rec,
	matrix_compare(),
	d_n;

    char 
	side[ 1 ], uplo[ 1 ], trans[ 1 ], diag[ 1 ];
  
    FLA_Obj
	A, B, Bold, Bref;
  
    FLA_Init( );

    dtime = dclock();
    sleep( 1 );
    dtime = dclock() - dtime;
    if ( dabs( dtime - 1 ) >= 0.01 )
	printf("callibrate clock: elapsed time = %lf\n", dtime );

    printf("%% which algorithm? (1 = column-lazy, 2 = lazy)");
    scanf("%d", &algo);
      

    printf("%c number of repeats:", '%');
    scanf("%d", &repeat );
  
    while ( TRUE ){
#if 0
	iop = 4;		/* TRSM is 4 */
	iside = 1;
	side[ 0 ] = 'R';

	iuplo = 0;
	uplo[ 0 ] = 'L';
    
	itransa = 1;		/* Transposed */
	trans[ 0 ] = 'Y';

	idiag = 0;
	diag[ 0 ] = 'N';
#endif

	/* We will time all m
	   from  mfirst through mlast in 
	   increments of minc */

	printf("%c enter mfirst, mlast, minc:", '%' );
	scanf( "%d", &m_first );

	if ( m_first == -1 ) break;

	scanf( "%d", &m_last );
	scanf( "%d", &m_inc );

	/* We will time all n
	   from  nfirst through nlast in 
	   increments of ninc */

	/*
	  printf("%c enter nfirst, nlast, ninc:", '%' );
	  scanf( "%d%d%d", &n_first, &n_last, &n_inc );
	*/

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
	if ( m == -1 ) break;

	printf("%c Performance in MFLOPS/sec (1e6 flops per second)\n\n", '%' );
	printf("%c    m    n  nb    REF  FLAME   diff      lev2   diff   blocked   diff   recurs.   diff\n", '%' );
	printf("%c ====================================================================\n", '%' );
	printf("data = [\n");

	iter = 0;

	for ( m=m_first; m<=m_last; m+=m_inc ){
	    /*      for ( n=n_first; n<=n_last; n+=n_inc ){ */
	    {
		if ( m_eq_n )
		    n = m;

		m_A = n_A = m;
		m_B = m;
		n_B = n;
    
		/* Allocate space for the three matrices */
    
		FLA_Obj_create( FLA_DOUBLE, m_A, n_A, &A );
		FLA_Obj_create( FLA_DOUBLE, m_B, n_B, &B );
		FLA_Obj_create( FLA_DOUBLE, m_B, n_B, &Bold );
		FLA_Obj_create( FLA_DOUBLE, m_B, n_B, &Bref );
    
		mflops = 1.0 * m_B * n_B * n_A;

		/* Generate random matrices A, B, and C */
      
		fill_matrix( A );
		fill_matrix( B );
	  
	
		d_n = ( double ) m_A;
	
		FLA_Add_to_diagonal( A, &d_n );
		FLA_Copy( B, Bref );
		FLA_Copy( B, Bold );

		for ( irep = 0 ; irep < repeat; irep++ ){

		    FLA_Copy( Bold, Bref );

		    /* Time reference implementation */
	  
		    dtime = dclock();

		    cblas_dtrsm( CblasColMajor,
				 CblasRight, CblasLower,
				 CblasTrans, CblasNonUnit,
				 m, n, 
				 d_one, 
				 ( double * ) A.buffer, A.ldim, 
				 ( double * ) Bref.buffer, Bref.ldim ); 
	  
		    dtime = dclock() - dtime;
	  
		    if ( irep == 0 )
			dtime_ref = dtime;
		    else
			dtime_ref = ( dtime_ref < dtime ? dtime_ref : dtime );

		    /* Time FLAME implementation */

		    dtime = dclock();
	  
		    FLA_Trsm( FLA_RIGHT, FLA_LOWER_TRIANGULAR, 
			      FLA_TRANSPOSE, FLA_NONUNIT_DIAG,
			      ONE, A, B ); 
	
		    dtime = dclock() - dtime;
      
		    if ( irep == 0 )
			dtime_flame = dtime;
		    else
			dtime_flame= ( dtime_flame < dtime ? dtime_flame : dtime );

		    if ( irep == 0 ) 
			diff_flame = matrix_compare( B.m, B.n, B.buffer, B.ldim,
						     Bref.buffer, Bref.ldim );
	
		    /* Time your level2 BLAS implementation */

		    /* Note: this takes too long for large problems, 
		       so we skip */

		    if ( m <= 500 && n <=500 ){
			FLA_Copy( Bold, B );
      
			dtime = dclock();

            switch (algo) {

                case 1:      
                    Trsm_right_lower_trans_lev2( A, B, 0 );
                    break;
                case 2:
                    Trsm_right_lower_trans2_lev2( A, B, 0 );
                    break;
                default:
                    assert(0);
                    break;
            }
	
			dtime = dclock() - dtime;
      
			if ( irep == 0 )
			    dtime_unblk = dtime;
			else
			    dtime_unblk= ( dtime_unblk < dtime ? dtime_unblk : dtime );

			if ( irep == 0 ) 
			    diff_unblk = matrix_compare( B.m, B.n, B.buffer, B.ldim,
							 Bref.buffer, Bref.ldim );
		    }

		    /* Time your blocked implementation */

		    FLA_Copy( Bold, B );
      
		    dtime = dclock();
      
            switch (algo) {

                case 1:      
		            Trsm_right_lower_trans( A, B, nb_alg );
                    break;
                case 2:
		            Trsm_right_lower_trans2( A, B, nb_alg );
                    break;
                default:
                    assert(0);
                    break;
            }
	
		    dtime = dclock() - dtime;
      
		    if ( irep == 0 )
			dtime_blk = dtime;
		    else
			dtime_blk= ( dtime_blk < dtime ? dtime_blk : dtime );

		    if ( irep == 0 ) 
			diff_blk = matrix_compare( B.m, B.n, B.buffer, B.ldim,
						   Bref.buffer, Bref.ldim );

		    /* Time your recursive implementation */

		    FLA_Copy( Bold, B );
      
		    dtime = dclock();
      
            switch (algo) {

                case 1:      
		            Trsm_right_lower_trans_recurse( A, B, nb_alg );
                    break;
                case 2:
		            Trsm_right_lower_trans2_recurse( A, B, nb_alg );
                    break;
                default:
                    assert(0);
                    break;
            }
	
		    dtime = dclock() - dtime;
      
		    if ( irep == 0 )
			dtime_rec = dtime;
		    else
			dtime_rec= ( dtime_rec < dtime ? dtime_rec : dtime );

		    if ( irep == 0 ) 
			diff_rec = matrix_compare( B.m, B.n, B.buffer, B.ldim,
						   Bref.buffer, Bref.ldim );
		}

		printf("%4d %4d %3d  ", m, n, nb_alg );

		/* print MFLOPS for reference implementation */
		printf( "%5.1lf  ", mflops / dtime_ref / 1.0e6 );

		/* print MFLOPS for flame implementation */
		printf( "%5.1lf  ", mflops / dtime_flame / 1.0e6 );

		/* print difference in result for flame implementation */
		printf( "%7.1le  ", diff_flame );

		if ( m <= 500 && n <=500 ){
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
	}
    
	printf("]\n");

	printf("plot( data( :, 1 ), data( :, 4 ), '-', ... \n");
	printf("      data( :, 1 ), data( :, 5 ), '-.', ... \n");
	printf("      data( 1:%d, 1 ), data( 1:%d, 7 ), '--x', ... \n", iter, iter);
	printf("      data( :, 1 ), data( :, 9 ), '--o', ... \n");
	printf("      data( :, 1 ), data( :, 11 ), '--+' )\n");
    
	printf("legend( 'Reference', 'FLAME', 'Unblocked', 'Blocked', 'Recursive', 1 )\n");

	printf("axis( [0, %d, 0, 600 ] )\n", m_last );
	printf("grid on\n" );

	if ( m_eq_n )
	    printf("title( 'B <- B L^-T  nb = %d' )\n", nb_alg );
	else
	    printf("title( 'n = %d     nb = %d' )\n", n, nb_alg );

	if ( m_eq_n )
	    printf("xlabel( 'm = n' )\n");
	else
	    printf("xlabel( 'm' )\n");

	printf("ylabel( 'MFLOPS/sec.' )\n");

	printf( "print -depsc2 ltrmm.eps\n" );

	FLA_Obj_free( &A );
	FLA_Obj_free( &B );
	FLA_Obj_free( &Bold );
	FLA_Obj_free( &Bref );
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
