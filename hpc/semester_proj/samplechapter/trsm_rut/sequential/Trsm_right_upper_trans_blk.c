#include "FLAME.h"
#define FLA_VARIANT_ONE 1
#define FLA_VARIANT_TWO 2

void Trsm_right_upper_trans_blk( int variant, int rec, FLA_Obj A, FLA_Obj B, int nb_alg )
{
  FLA_Obj     ATL, ATR,     A00, A01, A02,    BL,  BR,  B0,  B1,  B2,
              ABL, ABR,     A10, A11, A12,
                            A20, A21, A22;
  int         b;
  
  FLA_Part_2x2( A,  &ATL, /**/ &ATR,
                  /* ************** */
                    &ABL, /**/ &ABR,
		0, 0,     /* submatrix */ FLA_BR );

  FLA_Part_1x2( B,  &BL,  /**/ &BR,
		0, /* width  submatrix */ FLA_RIGHT );



  while ( b = min( FLA_Obj_length( ATL ), nb_alg ) ){
    FLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00, &A01, /**/ &A02,
                                /**/              &A10, &A11, /**/ &A12,
                         /* *********** */      /* ******************* */                          
                           ABL, /**/ ABR,         &A20, &A21, /**/ &A22,   
			   b, b, /* A11 from */ FLA_TL );

    FLA_Repart_1x2_to_1x3( BL, /**/ BR,           &B0, &B1, /**/ &B2,    
			   b, /* width B1 from */ FLA_LEFT );

    /* ********************************************************************* */

    if ( variant == FLA_VARIANT_ONE )
      FLA_Gemm( FLA_NO_TRANSPOSE, FLA_TRANSPOSE, MINUS_ONE, B2, A12, ONE, B1 );
      
    if ( rec == FLA_RECURSIVE && b > 8 )
      Trsm_right_upper_trans_blk( variant, rec, A11, B1 , nb_alg/2 );
    else
      Trsm_right_upper_trans_unb( variant, A11, B1 );

    if ( variant == FLA_VARIANT_TWO )
      FLA_Gemm( FLA_NO_TRANSPOSE, FLA_TRANSPOSE, MINUS_ONE, B1, A01, ONE, B0 );

    /* ********************************************************************* */

    FLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,         A00, /**/ A01, A02,
                            /* ************** */    /* ****************** */
                                    /**/               A10, /**/ A11, A12,
                              &ABL, /**/ &ABR,         A20, /**/ A21, A22, 
			      /* A11 added to */ FLA_BR );

    FLA_Cont_with_1x3_to_1x2( &BL, /**/ &BR,           B0, /**/ B1, B2, 
			      /* B1  added to */ FLA_RIGHT );
  }
}
