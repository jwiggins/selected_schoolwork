#include "cblas.h"

#define  min( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )

void my_gemm_util( int, int, int, const double *, int, 
		   const double *, int,
		   double *, int );

void my_gemm( int m, int n, int k, const double *a, int lda, 
                                   const double *b, int ldb, 
                                   double *c, int ldc )
{
  int i, j, p, curn, curm, curk;

  for ( j=0; j<n; j+=40 ){
    curn = min( 40, n-j );
    for  ( p=0; p<k; p+=40 ){
      curk = min( 40, k-p );
      for ( i=0; i<m; i+=40 ){
	curm = min( 40, m-i );
	/*
	my_gemm_util( curm, curn, curk, &a[ p*lda + i ], lda,
		      &b[ j*ldb + p ], ldb,
		      &c[ j*ldc + i ], ldc );
	*/
	cblas_dgemm( CblasColMajor, CblasNoTrans, CblasNoTrans,
		     curm, curn, curk, 1.0, 
		     &a[ p*lda + i ], lda,
		      &b[ j*ldb + p ], ldb, 
		      1.0, &c[ j*ldc + i ], ldc );
      }
    }
  }
}
	

void my_gemm_util( int m, int n, int k, const double *a, int lda, 
                                   const double *b, int ldb, 
                                   double *c, int ldc )
{
  int 
    i, j, p;

  register double bpj;

  const double *ap;
  double *cp, *cp_past, *cp_first;

  for ( j=0; j<n; j++ ){
    cp_first = &c[ j*ldc ];
    cp_past = cp_first+m;
    for ( p=0; p<k; p++ ){
      cp = cp_first;
      bpj = b[ j*ldb + p ];
      ap = &a[ p*lda ];

      for ( ; cp != cp_past; ){
	/* c(i,j) = a( i,p ) * b( p,j ) + c( i,j ) */
	cp[0] += bpj * ap[0];
	cp[1] += bpj * ap[1];
	cp[2] += bpj * ap[2];
	cp[3] += bpj * ap[3];
	cp += 4;
	ap += 4;
      }
    }
  }
}

