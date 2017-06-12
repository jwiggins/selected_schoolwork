#undef L2_SIZE_LARGE
#undef L1_SIZE_LARGE
#define L2_SIZE_LARGE 512
#define L1_SIZE_LARGE 64

#define TRUE 1
#define FALSE 0

static first_time = TRUE;
static double a[ L2_SIZE_LARGE * L1_SIZE_LARGE*2 ];
static double b[ L1_SIZE_LARGE * L2_SIZE_LARGE*2 ];
static double c[ L2_SIZE_LARGE * L2_SIZE_LARGE ];

dirty_cache()
{
  int 
    size_large = L2_SIZE_LARGE, 
    size_small = L1_SIZE_LARGE*2;
  double
    d_one = 1.0, d_zero = 0.0;

  if ( first_time ){
    first_time = FALSE;

    /* generate random matrices a and b */
//    random_matrix( size_large, size_small, a, size_large );
//    random_matrix( size_small, size_large, b, size_small );
//    random_matrix( size_large, size_large, c, size_large );
  }

  /* do multiply to dirty cache */

  dgemm_( "Notranspose", "Notranspose", &size_large, &size_large, &size_small,
	  &d_one, a, &size_large, b, &size_small, &d_zero, c, &size_large );

}
