#include "FLAME.h"

void MY_Sytrrk_uln( FLA_Obj A, FLA_Obj C )
{
  FLA_Obj     CTL, CTR,     C00,  c01,     C02,
              CBL, CBR,     C10, gamma11, c12t,
                            C20,   C21,    C22,
              ATL, ATR,     A00,    A01,   A02,
              ABL, ABR,     a10t, alpha11, A12,
                            A20,    a21,   A22;

  FLA_Part_2x2( C,  &CTL, /**/ &CTR,
                   /* ************** */
                    &CBL, /**/ &CBR,   0, 0,  /* submatrix */ FLA_BR );

  FLA_Part_2x2( A,  &ATL, /**/ &ATR,
                   /* ************** */
                    &ABL, /**/ &ABR,   0, 0,  /* submatrix */ FLA_BR );

  while ( 0 != FLA_Obj_length( CTL ) )
  {
    FLA_Repart_2x2_to_3x3( CTL, /**/ CTR,         &C00, &c01,     /**/ &C02,
		        /* ************* */	  &C10, &gamma11, /**/ &c12t,
                                /**/           /* *************************** */
                           CBL, /**/ CBR,         &C20, &C21,     /**/ &C22,
                           1, 1, /* gamma11 from */ FLA_TL );


    FLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00,  &A01,     /**/  &A02,
		        /* ************* */ 	  &a10t, &alpha11, /**/  &A12,
                                /**/           /* *************************** */
                           ABL, /**/ ABR,         &A20,  &a21,     /**/  &A22,
                           1, 1, /* alpha11 from */ FLA_TL );

    /* c12t <- A22^T a21 + c12t */
    FLA_Trmv_x( FLA_LOWER_TRIANGULAR, FLA_TRANSPOSE, FLA_NONUNIT_DIAG,
		ONE, A22, a21, ONE, c12t );

    /* gamma11 <- a21^t a21 + gammall */
    FLA_Dot_x( ONE, a21, a21, ONE, gamma11 );
    /* gamma11 <- alpha11 alpha11 + gamma11 */
    FLA_Axpy( alpha11, alpha11, gamma11 );

    FLA_Cont_with_3x3_to_2x2( &CTL, /**/ &CTR,         C00, /**/ c01,     C02,
                            /* ************** */    /* ************************ */
                                                       C10, /**/ gamma11, c12t,
                              &CBL, /**/ &CBR,         C20, /**/ C21,     C22,
			      /* gamma11 added to */ FLA_BR );

    FLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,         A00, /**/  A01,     A02,
                            /* ************** */    /* ************************ */
                                    /**/    	       a10t,/**/  alpha11, A12,

                              &ABL, /**/ &ABR,         A20, /**/  a21,     A22,
			      /* alpha11 added to */ FLA_BR );

  }
}

