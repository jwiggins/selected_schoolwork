void fill_matrix( int m, int n, double *a, int lda )
{
  int
    i, j;
  double 
    drand48();

  for ( j=0; j<n; j++ )
    for ( i=0; i<m; i++ )
      a[ j*lda+i ] = 2.0*drand48() - 1.0;
}
