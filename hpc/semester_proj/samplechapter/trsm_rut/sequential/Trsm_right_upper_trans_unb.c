#include "FLAME.h"
#define FLA_VARIANT_ONE 1
#define FLA_VARIANT_TWO 2

void Trsm_right_upper_trans_unb( int variant, FLA_Obj A, FLA_Obj B )
{
  FLA_Obj     ATL, ATR,     A00,  a01,     A02,    BL, BR, B0, b1, B2,
              ABL, ABR,     a10t, alpha11, a12t,
                            A20,  a21,     A22;
  int         b;

  FLA_Part_2x2( A,  &ATL, /**/ &ATR,
                  /* ************** */
                    &ABL, /**/ &ABR,   
		0, 0,     /* submatrix */ FLA_BR );

  FLA_Part_1x2( B,  &BL,  /**/ &BR,    
		0, /* width  submatrix */ FLA_RIGHT );



  while ( 0 != FLA_Obj_width( BL ) ){
    FLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00,   &a01,     /**/  &A02,
                                /**/              &a10t,  &alpha11, /**/  &a12t,
                        /* ************* */    /* ************************** */
                           ABL, /**/ ABR,         &A20,   &a21,     /**/  &A22,  
			   1, 1, /* alpha11 from */ FLA_TL );

    FLA_Repart_1x2_to_1x3( BL, /**/ BR,           &B0,  &b1, /**/  &B2,    
			   1, /* width b1 from */ FLA_LEFT );

    /* ********************************************************************* */

    if ( variant == FLA_VARIANT_ONE )
      FLA_Gemv( FLA_NO_TRANSPOSE, MINUS_ONE, B2, a12t, ONE, b1 );

    FLA_Inv_scal( alpha11, b1 );

    if ( variant == FLA_VARIANT_TWO )
      FLA_Ger( MINUS_ONE, b1, a01, B0 );
    
    /* ********************************************************************* */

    FLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,        A00,  /**/ a01,     A02,
                            /* ************** */   /* ********************* */
                                    /**/              a10t, /**/ alpha11, a12t,
                              &ABL, /**/ &ABR,        A20,  /**/ a21,     A22, 
			      /* alpha11 added to */ FLA_BR );

    FLA_Cont_with_1x3_to_1x2( &BL, /**/ &BR,           B0, /**/ b1,  B2, 
			      /* b1  added to */ FLA_RIGHT );
  }
}





