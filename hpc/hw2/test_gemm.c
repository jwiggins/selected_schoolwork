#include <math.h>
#include <stdio.h>
#include <time.h>

#include "cblas.h"

#define dabs( x ) ( (x) < 0 ? -(x) : (x) )

extern double dclock();
extern double matrix_compare( int, int, double *, int, double *, int );

#define min(x,y) ( (x) < (y) ? (x) : (y) )

int main(int argc, char *argv[])
{
  int 
    m, n, k,
    lda, ldb, ldc,
    nfirst, nlast, ninc,
    repeat, irep;

  double
    dtime, flops,
    dtime_ref, dtime_me,
    diff_me,
    *a, *b, *c, *cold, *cref,
    d_one = 1.0;

  dtime = dclock();
  sleep( 1 );
  dtime = dclock() - dtime;
  if ( dabs( dtime - 1 ) >= 0.01 )
    printf("%%callibrate clock: elapsed time = %lf\n", dtime );
      

  printf("%c number of repeats:", '%');
  scanf("%d", &repeat );
  
  for (;;)
  {
    printf("%c enter nfirst, nlast, ninc:", '%' );
    scanf( "%d", &nfirst );

    if ( nfirst == -1 ) break;

    scanf( "%d", &nlast );
    scanf( "%d", &ninc );

    /* We will time all n
       from  nfirst through nlast in 
       increments of ninc */

    printf("%c Performance in MFLOPS/sec (1e6 flops per second)\n\n", '%' );
    printf("%c    m    n   k    REF  ME   diff   \n", '%' );
    printf("%c ==================================\n", '%' );
    printf("data = [\n");

    for ( m=nfirst; m<=nlast; m+=ninc )    {
      /* Time the case where m = n = k only */
      n = k = m;
      lda = ldc = m;
      ldb = k;

       /* Allocate space for the five matrices */
    
      a =    ( double * ) malloc( lda * k * sizeof( double ) );
      b =    ( double * ) malloc( ldb * n * sizeof( double ) );
      cold = ( double * ) malloc( ldc * n * sizeof( double ) );
      c =    ( double * ) malloc( ldc * n * sizeof( double ) );
      cref = ( double * ) malloc( ldc * n * sizeof( double ) );

      flops = 2.0 * m * n * k;

      /* Generate random matrices A, B, and C */
      
      fill_matrix( m, k, a, m );
      fill_matrix( k, n, b, k );
      fill_matrix( m, n, cold, m );

      for ( irep = 0 ; irep < repeat; irep++ ){
	/* Time reference implementation */

	copy_matrix( m, n, cold, m, cref, m );

	dirty_cache();

	dtime = dclock();

	cblas_dgemm( CblasColMajor,
		     CblasNoTrans,
		     CblasNoTrans,
		     m, n, k,
		     d_one,
		     a, lda,
		     b, ldb,
		     d_one,
		     cref, ldc );
	  
	  dtime = dclock() - dtime;
	  

	  /* we time many interations and pick the fastest,
	     in case there are others on the system */

	  if ( irep == 0 )
	    dtime_ref = dtime;
	  else
	    dtime_ref = ( dtime_ref < dtime ? dtime_ref : dtime );

	  /* Time my implementation */

	  copy_matrix( m, n, cold, m, c, m );

	  dirty_cache();

	  dtime = dclock();
	  
          my_gemm( m, n, k, a, lda, b, ldb, c, ldc );

	  dtime = dclock() - dtime;
      
	  if ( irep == 0 )
	    dtime_me = dtime;
	  else
	    dtime_me= ( dtime_me < dtime ? dtime_me : dtime );

	  if ( irep == 0 ) 
	    diff_me = matrix_compare( m, n, c, ldc, cref, ldc );
      }

      printf("%4d %4d %4d  ", m, n, k );

      /* print MFLOPS for reference implementation */
      printf( "%5.1lf  ", flops / dtime_ref / 1.0e6 );

      /* print MFLOPS for my implementation */
      printf( "%5.1lf  ", flops / dtime_me / 1.0e6 );

      /* print difference in result for flame implementation */
      printf( "%7.1le  ", diff_me );

      printf( "\n" );
    }
    
    printf("]\n");

    printf("plot( data( :, 1 ), data( :, 4 ), '-', ... \n");
    printf("      data( :, 1 ), data( :, 5 ), '-.' ) \n");
    
    printf("legend( 'Reference', 'ME' );\n");

    printf("axis( [0, %d, 0, 600 ] )\n", nlast );
    printf("grid on\n" );

    printf("title( 'C <- A B + C' )\n" );

    printf("xlabel( 'm = n = k' )\n");

    printf("ylabel( 'MFLOPS/sec.' )\n");

    printf( "print -depsc2 gemm.eps\n" );

    free( a );
    free( b );
    free( cold );
    free( c );
    free( cref );
  }  
}

