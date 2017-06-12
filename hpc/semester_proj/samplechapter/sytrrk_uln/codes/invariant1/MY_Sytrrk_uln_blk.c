#include "FLAME.h"

void MY_Sytrrk_uln_blocked( FLA_Obj A, FLA_Obj C, int nb_alg, int rec )
{
  int b;
  FLA_Obj    CTL, CTR,     C00, C01, C02,
             CBL, CBR,     C10, C11, C12,
                           C20, C21, C22,
              ATL, ATR,     A00, A01, A02,
              ABL, ABR,     A10, A11, A12,
                            A20, A21, A22,
     A10t;

  FLA_Part_2x2( C,  &CTL, /**/ &CTR,
                   /* ************** */
                    &CBL, /**/ &CBR,   0, 0,  /* submatrix */ FLA_TL );

  FLA_Part_2x2( A,  &ATL, /**/ &ATR,
                   /* ************** */
                    &ABL, /**/ &ABR,   0, 0,  /* submatrix */ FLA_TL );

  while ( b = min(FLA_Obj_length( CBR ), nb_alg) )
  {
    FLA_Repart_2x2_to_3x3( CTL, /**/ CTR,         &C00,  /**/ &C01,     &C02,
                        /* ************* */    /* *************************** */
						  &C10,  /**/ &C11,     &C12,
                           CBL, /**/ CBR,         &C20,  /**/ &C21,     &C22,
                           b, b, /* C11 from */ FLA_BR );


    FLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00,  /**/ &A01,     &A02,
                        /* ************* */    /* *************************** */
						  &A10,  /**/ &A11,     &A12,
                           ABL, /**/ ABR,         &A20,  /**/ &A21,     &A22,
                           b, b, /* A11 from */ FLA_BR );

    /* C01 <- A10^T A11 + C01 */
    FLA_Obj_create_conf_to( FLA_NO_TRANSPOSE, C01, &A10t );
    FLA_Copy_x( FLA_TRANSPOSE, A10, A10t );
    FLA_Trmm_x( FLA_RIGHT, FLA_LOWER_TRIANGULAR, FLA_NO_TRANSPOSE, FLA_NONUNIT_DIAG,
		ONE, A11, A10t, ONE, C01 );
    FLA_Obj_free( &A10t );
    /* C01 <- A20^T A21 + C01 */
    FLA_Gemm( FLA_TRANSPOSE, FLA_NO_TRANSPOSE, ONE, A20, A21, ONE, C01 );


    /* C11 <- A11^T A11 + C11 */
    if (rec == 1 && b > 32)
      MY_Sytrrk_uln_blocked( A11, C11, b/2, rec );
    else
      MY_Sytrrk_uln( A11, C11 );
    /* C11 <- A21^T A21 + C11 */
    FLA_Syrk( FLA_UPPER_TRIANGULAR, FLA_TRANSPOSE, ONE, A21, ONE, C11 );

    FLA_Cont_with_3x3_to_2x2( &CTL, /**/ &CTR,         C00,   C01,     /**/ C02,
                                    /**/    	       C10,   C11,     /**/ C12,
			     /* ************** */   /* ************************ */
                              &CBL, /**/ &CBR,         C20,   C21,     /**/ C22,
			      /* C11 added to */ FLA_TL );

    FLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,         A00,   A01,     /**/ A02,
                                     /**/    	       A10,   A11,     /**/ A12,
			     /* ************** */     /* ************************ */
                              &ABL, /**/ &ABR,         A20,   A21,     /**/ A22,
			      /* A11 added to */ FLA_TL );

  }
}
