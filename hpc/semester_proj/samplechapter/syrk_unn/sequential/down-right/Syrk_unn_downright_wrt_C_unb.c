#include "FLAME.h"

void Syrk_un_downright_wrt_C_unb( FLA_Obj A, FLA_Obj C )
{
  FLA_Obj     CTL, CTR,     C00,  c01,     C02,         AT,      A0,
              CBL, CBR,     C10, gamma11, c12t,         AB,      a1t,
                            C20,  C21,     C22,     		 A2;
  int         b;

  FLA_Part_2x2( C,  &CTL, /**/ &CTR,
                  /* ************** */
                    &CBL, /**/ &CBR,   0, 0,     /* submatrix */ FLA_TL );

  FLA_Part_2x1( A,  &AT, 
                   /***/
                    &AB,               0, /* length submatrix */ FLA_TOP );

  while ( 0 != FLA_Obj_length( CBR ) ){
    FLA_Repart_2x2_to_3x3( CTL, /**/ CTR,         &C00,  /**/ &c01,     &C02,
                        /* ************* */    /* *************************** */
						  &C10, /**/ &gamma11, &c12t,
                           CBL, /**/ CBR,         &C20,  /**/ &C21,     &C22,   
                           1, 1, /* gamma11 from */ FLA_BR );

    FLA_Repart_2x1_to_3x1( AT,                   &A0,
                           /**/                  /**/
                                                 &a1t,
                           AB,                   &A2,    
                           1, /* length a1t from */ FLA_BOTTOM );

    /* ********************************************************************* */

    FLA_Dot_x( ONE, a1t, a1t, ONE, gamma11 );

    FLA_Gemv(FLA_NO_TRANSPOSE, ONE, A2, a1t, ONE, c12t );

    /* ********************************************************************* */

    FLA_Cont_with_3x3_to_2x2( &CTL, /**/ &CTR,         C00,   c01,     /**/ C02,
                                    /**/    	       C10,  gamma11, /**/ c12t,         
			     /* ************** */   /* ************************ */    
                              &CBL, /**/ &CBR,         C20,   C21,     /**/ C22, 
			      /* gamma11 added to */ FLA_TL );

    FLA_Cont_with_3x1_to_2x1( &AT,                     A0,
                                                       a1t,
			     /***/                    /**/
                              &AB,                     A2,                 
			      /* a1t added to */ FLA_TOP );
  }
}
