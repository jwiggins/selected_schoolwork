#include "FLAME.h"

int Trsm_right_lower_trans_lev2( FLA_Obj A, FLA_Obj B, int nb_alg )
{
  FLA_Obj     ATL, ATR,     A00,  a01,     A02,    BL, BR,   B0, b1, B2,
              ABL, ABR,     a10t, alpha11, a12t,
                            A20,  a21,     A22;
  int         b;

  FLA_Part_2x2( A,  &ATL, /**/ &ATR,
                  /* ************** */
                    &ABL, /**/ &ABR,   
		0, 0,     /* submatrix */ FLA_TL );

  FLA_Part_1x2( B,  &BL,  /**/ &BR,    
		0, /* width  submatrix */ FLA_LEFT );



  while ( 0 != FLA_Obj_length( ABR ) ){
    FLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00, /**/  &a01,     &A02,
                        /* ************* */    /* ************************** */
                                /**/              &a10t, /**/ &alpha11, &a12t,
                           ABL, /**/ ABR,         &A20, /**/  &a21,     &A22,  
			   1, 1, /* alpha11 from */ FLA_BR );

    FLA_Repart_1x2_to_1x3( BL, /**/ BR,           &B0,  /**/ &b1,  &B2,    
			   1, /* width B1 from */ FLA_RIGHT );

    /* ********************************************************************* */

    FLA_Inv_scal( alpha11, b1 );

    FLA_Ger( MINUS_ONE, b1, a21, B2 );

/*    FLA_Gemm( FLA_NO_TRANSPOSE, FLA_TRANSPOSE, MINUS_ONE, b1, a21, ONE, B2 ); */

    /* ********************************************************************* */

    FLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,        A00,  a01,     /**/ A02,
                                    /**/              a10t, alpha11, /**/ a12t,
                            /* ************** */   /* ********************* */
                              &ABL, /**/ &ABR,        A20,  a21,     /**/ A22, 
			      /* alpha11 added to */ FLA_TL );

    FLA_Cont_with_1x3_to_1x2( &BL, /**/ &BR,           B0,  b1,  /**/ B2, 
			      /* b1  added to */ FLA_LEFT );
  }

  return FLA_SUCCESS;
}





