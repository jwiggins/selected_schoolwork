#include <math.h>
#include <stdio.h>
#include <time.h>

#include "FLAME.h"

#define dabs( x ) ( (x) < 0 ? -(x) : (x) )

extern int version;

int debugging = 1;

extern double dclock();

#define min(x,y) ( (x) < (y) ? (x) : (y) )

int main(int argc, char *argv[])
{
  int 
    n, nb_alg, nlast,
    repeat, irep;

  double
    dtime, mflops,
    d_one = 1.0,
    diff,
    matrix_compare();

  FLA_Obj
    A, C, Cold, Cref;
  
  FLA_Init( );

  /* Check if the dclock routine is properly calibrated */

  dtime = dclock();
  sleep( 1 );

  dtime = dclock() - dtime;
  if ( dabs( dtime - 1 ) >= 0.01 ){
    printf("%c callibrate clock: elapsed time = %lf\n", '%', dtime );
    printf("%c modify dclock_s.s in the obvious way\n", '%');
  }

  /* Every time trial is repeated "repeat" times */

  printf("%c number of repeats:", '%');
  scanf("%d", &repeat );

  printf( "\n
%c Note: the columns marked with diff give
%c the difference in the results between the reference
%c implementation and your implementation.
%c This should be small\n", '%', '%', '%', '%' );

  printf( "\n" );

  printf("%c Performance in MFLOPS/sec (1e6 flops per second)\n\n", '%');
  printf("%c   n   n  nb    REF unblocked diff   blocked   diff   recursive   diff\n", '%');
  printf("%c =======================================================================\n", '%');
  printf("data = [\n");

  while ( TRUE ){
    nlast = n;

    /* Get the dimensions of A and C 
       and the algorithmic block size to be used */
    printf( "%c enter n, nb_alg:\n", '%' );
    scanf( "%d%d", &n, &nb_alg );

    /* if n == -1 we are done */
    if ( n == -1 ) break;

    /* Allocate space for the matrices */
    
    FLA_Obj_create( FLA_DOUBLE, n, n, &A );
    FLA_Obj_create( FLA_DOUBLE, n, n, &C );

    /* Cold will hold the original contents of C
       so we can run timings for different implementations
       using the same data */

    FLA_Obj_create( FLA_DOUBLE, n, n, &Cold );

    /* Cref will hold the result of C = op(A) * op(A)^T + C for
       the reference implementation.  It is the
       reference implementation that your implementation
       should beat in performance.  Alternatively,
       your implementation should reduce the workspace
       required. */

    FLA_Obj_create( FLA_DOUBLE, n, n, &Cref );

    for ( irep = 0 ; irep < repeat; irep++ ){
      /* Generate random matrices A and C */
      printf("%4d %4d %3d  ", n, n, nb_alg );

      random_matrix( A );
      random_matrix( C );

      FLA_Copy( C, Cref );
      FLA_Copy( C, Cold );

      /* Time reference implementation */

      dtime = dclock();

      // C <- A^T A + C
      REF_Sytrrk( FLA_UPPER_TRIANGULAR, FLA_LOWER_TRIANGULAR,
		  FLA_TRANSPOSE, A, Cref );

      dtime = dclock() - dtime;
      
      mflops = 0.333 * n * n * n;

      printf("%5.1lf  ", mflops / dtime / 1e6 );
      fflush( stdout );

      /* Time your level2 BLAS implementation */

      FLA_Copy( Cold, C );
      
      dtime = dclock();

      /* Naturally, you want to call your routine instead! */
      MY_Sytrrk_uln( A, C );
      /*REF_Sytrrk( FLA_UPPER_TRIANGULAR, FLA_LOWER_TRIANGULAR,
		  FLA_TRANSPOSE, A, C );*/
	
      dtime = dclock() - dtime;

      mflops = 0.333 * n * n * n;

      printf("%5.1lf  ", mflops / dtime / 1e6 );
      fflush( stdout );

      diff = matrix_compare( n, n, C.buffer, C.ldim,
			     Cref.buffer, C.ldim );

      printf("%5.2le  ", diff );
      fflush( stdout );

      /* Time your blocked non-recursive implementation */

      FLA_Copy( Cold, C );

      dtime = dclock();

      /* Naturally, you want to call your routine instead! */
      MY_Sytrrk_uln_blocked( A, C, nb_alg, 0 );
      /*REF_Sytrrk( FLA_UPPER_TRIANGULAR, FLA_LOWER_TRIANGULAR,
		  FLA_TRANSPOSE, A, C );*/

      dtime = dclock() - dtime;

      mflops = 0.333 * n * n * n;

      printf("%5.1lf  ", mflops / dtime / 1e6 );
      fflush( stdout );

      diff = matrix_compare( n, n, C.buffer, C.ldim,
			     Cref.buffer, Cref.ldim );

      printf("%5.2le  ", diff );
      fflush( stdout );

			/* Time your blocked recursive implementation */

      FLA_Copy( Cold, C );

      dtime = dclock();

      /* Naturally, you want to call your routine instead! */
      MY_Sytrrk_uln_blocked( A, C, nb_alg, 1 );

      dtime = dclock() - dtime;

      mflops = 0.333 * n * n * n;

      printf("%5.1lf  ", mflops / dtime / 1e6 );
      fflush( stdout );

      diff = matrix_compare( n, n, C.buffer, C.ldim,
			     Cref.buffer, Cref.ldim );

      printf("%5.2le\n", diff );
      fflush( stdout );
    }

    /* If it is a small problem, create something that
       can be fed through MATLAB to check the answer. */
    if ( n <= 8 ){
      FLA_Obj_show( "A = [", A, "%lf ", "];" );
      FLA_Obj_show( "Cold = [", Cold, "%lf ", "];" );
      FLA_Obj_show( "C = [", Cref, "%lf ", "];" );
      printf( "%c The following should equal approximately 0\n", '%' );
      printf( "%c (Be aware of a 1.0e-12 * or something like that\n", '%' );
      printf( "%c before the matrix is printed.  This means it is small,\n", '%' );
      printf( "%c which is good!)\n", '%' );
      printf( "Cold + tril( triu(A)' * triu(A) ) - C\n" );

    }

    FLA_Obj_free( &A );
    FLA_Obj_free( &C );
    FLA_Obj_free( &Cold );
    FLA_Obj_free( &Cref );
  }

  printf("];\n");

  printf("
plot( data( :,1 ), data( :, 4 ), '-x', ...
      data( :,1 ), data( :, 5 ), '-*', ...
      data( :,1 ), data( :, 7 ), '-o', ...
      data( :,1 ), data( :, 9 ), '-+' ); \n");
  printf("legend( 'Reference', 'unblocked', 'blocked', 'recursive', 5 ); \n");
  printf("xlabel( 'matrix dimension n' );\n");
  printf("ylabel( 'MFLOPS/sec.' );\n");
  printf("axis( [ 0 %d 0 600 ] ); \n", nlast );
  printf("print -depsc2 sytrrk_uln_inv1.eps\n" );
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
