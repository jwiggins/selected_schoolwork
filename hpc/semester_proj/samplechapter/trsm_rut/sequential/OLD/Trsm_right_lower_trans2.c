#include "FLAME.h"
#include "stdio.h"

int Trsm_right_lower_trans2( FLA_Obj A, FLA_Obj B, int nb_alg )
{
  FLA_Obj     ATL, ATR,     A00, A01, A02,    BL,  BR,  B0,  B1,  B2,
              ABL, ABR,     A10, A11, A12,
                            A20, A21, A22;
  int         b;
  FLA_Part_2x2( A,  &ATL, /**/ &ATR,
                  /* ************** */
                    &ABL, /**/ &ABR,   
		0, 0,     /* submatrix */ FLA_TL );

  FLA_Part_1x2( B,  &BL,  /**/ &BR,    
		0, /* width  submatrix */ FLA_LEFT );



  while ( b = min( FLA_Obj_length( ABR ), nb_alg ) ){
    FLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00, /**/ &A01, &A02,
                        /* ************* */    /* ********************* */
                                /**/              &A10, /**/ &A11, &A12,
                           ABL, /**/ ABR,         &A20, /**/ &A21, &A22,   
			   b, b, /* A11 from */ FLA_BR );

    FLA_Repart_1x2_to_1x3( BL, /**/ BR,           &B0,  /**/ &B1,  &B2,    
			   b, /* width B1 from */ FLA_RIGHT );

    /* ********************************************************************* */

    // if ( b <= 4 )
    //  FLA_Trsm_right_lower_trans_lev2( FLA_NONUNIT_DIAG, ONE, A11, B1 );
    // else
    //   FLA_Trsm_right_lower_trans( FLA_NONUNIT_DIAG, A11, B1 , b/2 );
    FLA_Gemm( FLA_NO_TRANSPOSE, FLA_TRANSPOSE, MINUS_ONE, B0, A10, ONE, B1 );
    FLA_Trsm( FLA_RIGHT, FLA_LOWER_TRIANGULAR, FLA_TRANSPOSE, FLA_NONUNIT_DIAG, ONE, A11, B1 );

    /* ********************************************************************* */

    FLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,         A00, A01, /**/ A02,
                                    /**/               A10, A11, /**/ A12,
                            /* ************** */    /* ****************** */
                              &ABL, /**/ &ABR,         A20, A21, /**/ A22, 
			      /* A11 added to */ FLA_TL );

    FLA_Cont_with_1x3_to_1x2( &BL, /**/ &BR,           B0,  B1,  /**/ B2, 
			      /* B1  added to */ FLA_LEFT );
  }

  return FLA_SUCCESS;
}
