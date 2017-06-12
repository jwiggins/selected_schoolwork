/*

	my_gemm.c - optimized matrix multiplication

	Jason Bradford <jbrad@cs.utexas.edu>
	John Wiggins <prok@mail.utexas.edu>	
	
*/


#define  min( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )

static void my_gemm_util(int m, int n, int k,
                    const double * const a, int lda,
                    const double * const b, int ldb,
                    double *c, int ldc );

void my_gemm( int m, int n, int k, const double * const a, int lda,
                                   const double * const b, int ldb,
                                   double *c, int ldc)
{
  int i, j, p, curn, curm, curk;

  for ( j=0; j<n; j+=40 )
  {
    curn = min( 40, n-j );
    for  ( p=0; p<k; p+=40 )
    {
      curk = min( 40, k-p );
      for ( i=0; i<m; i+=40 )
      {
      	curm = min( 40, m-i );
        my_gemm_util( curm, curn, curk, &a[ p*lda + i ], lda,
                        &b[ j*ldb + p ], ldb,
                        &c[ j*ldc + i ], ldc );
      }
    }
  }
}

void my_gemm_util(int m, int n, int k,
                    const double * const a, int lda,
                    const double * const b, int ldb,
                    double *c, int ldc )
{
  int i, j, p;
  register int b_off, c_off;
  register double c0,c1,c2,c3,c4;

  /* Matrix(row, col) */
  /* C(i,j) += A(i,p) * B(p,j) */
  for ( j=0; j != n; j++ )
  {
  	b_off = j*ldb;
  	c_off = j*ldc;
    for ( i=0; i <= m-5; i+=5 )
    {
      c0 = c[ c_off + i ];
      c1 = c[ c_off + i+1 ];
      c2 = c[ c_off + i+2 ];
      c3 = c[ c_off + i+3 ];
      c4 = c[ c_off + i+4 ];

      for ( p=0; p != k; p++ )
      {
        c0 += a[ p*lda + i ] * b[ b_off + p ];
        c1 += a[ p*lda + i+1 ] * b[ b_off + p ];
        c2 += a[ p*lda + i+2 ] * b[ b_off + p ];
        c3 += a[ p*lda + i+3 ] * b[ b_off + p ];
        c4 += a[ p*lda + i+4 ] * b[ b_off + p ];
      }

      c[ c_off + i ] = c0;
      c[ c_off + i+1 ] = c1;
      c[ c_off + i+2 ] = c2;
      c[ c_off + i+3 ] = c3;
      c[ c_off + i+4 ] = c4;
    }
    for ( ; i < m; i++ )
    {
      for ( p=0; p != k; p++ )
      {
        c[ c_off + i ] += a[ p*lda + i ] * b[ b_off + p ];
      }
    }
  }
}

