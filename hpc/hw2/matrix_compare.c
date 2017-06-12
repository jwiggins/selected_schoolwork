#define dabs( x ) ( (x)<0.0 ? -(x) : (x) )
#define max( x, y ) ( (x) > (y) ? (x) : (y) )

double matrix_compare( int m, int n, double *a, int lda, double *b, int ldb )
{
  int
    i, j;

  double 
    value, cur_diff;

  value = 0.0;
  for ( j=0; j<n; j++ )
    for ( i=0; i<m; i++ ){
      cur_diff = dabs( a[ j*lda+i ] - b[ j*ldb+i ] );
      value = max( cur_diff, value );
    }

  return value;
}
