copy_matrix( int m, int n, double *a, int lda, double *b, int ldb )
{
  int
    i, j;

  for ( j=0; j<n; j++ )
    for ( i=0; i<m; i++ )
      b[ j*ldb+i ] = a[ j*lda+i ];
}
