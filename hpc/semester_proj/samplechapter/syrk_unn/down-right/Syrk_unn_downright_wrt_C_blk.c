#include "FLAME.h"

void Syrk_un_downright_wrt_C_blk( int rec, FLA_Obj A, FLA_Obj C, int nb_alg )
{
 FLA_Obj     CTL, CTR,      C00, C01, C02,         AT,      A0,
             CBL, CBR,      C10, C11, C12,         AB,      A1,
                            C20, C21, C22,         A2;
  int         b;

  FLA_Part_2x2( C,  &CTL, /**/ &CTR,
                  /* ************** */
                    &CBL, /**/ &CBR,   0, 0,     /* submatrix */ FLA_TL );

  FLA_Part_2x1( A,  &AT, 
                   /***/
                    &AB,               0, /* length submatrix */ FLA_TOP );

  while ( b = min( FLA_Obj_length( CBR ), nb_alg ) ){
    FLA_Repart_2x2_to_3x3( CTL, /**/ CTR,         &C00, /**/  &C01,   &C02,
                        /* ************* */    /* *************************** */
						  &C10, /**/  &C11,   &C12,
                           CBL, /**/ CBR,         &C20, /**/  &C21,   &C22,   
                           b, b, /* C11 from */ FLA_BR );

    FLA_Repart_2x1_to_3x1( AT,                   &A0,
                           /**/                  /**/   
                                                 &A1,
                           AB,                   &A2,    
                           b, /* length A1 from */ FLA_BOTTOM );
	
    /* ********************************************************************* */

    if ( rec == FLA_RECURSIVE && b > 8 )
      Syrk_un_downright_wrt_C_blk( rec, A1, C11, nb_alg/2 );
    else
      Syrk_un_downright_wrt_C_unb( A1, C11 );

    FLA_Gemm(FLA_NO_TRANSPOSE,FLA_TRANSPOSE,ONE,A1,A2,ONE,C12);
    
    /* ********************************************************************* */


    FLA_Cont_with_3x3_to_2x2( &CTL, /**/ &CTR,         C00,  C01,  /**/    C02,
                                    /**/    	       C10,  C11,  /**/    C12,
			     /* ************** */   /* ************************ */ 
                              &CBL, /**/ &CBR,         C20,  C21,  /**/    C22, 
			      /* C11 added to */ FLA_TL );

  
    FLA_Cont_with_3x1_to_2x1( &AT,                     A0,
                                                       A1,
			     /***/                    /**/
                              &AB,                     A2,                 
			      /* A1 added to */ FLA_TOP );
  }
}
