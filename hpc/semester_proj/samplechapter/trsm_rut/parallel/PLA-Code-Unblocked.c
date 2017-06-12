#include "PLA.h"

void Trsm_right_upper_trans_unb( int variant, PLA_Obj A, PLA_Obj B )
{
  PLA_Obj     ATL=NULL, ATR=NULL,    A00=NULL,  a01=NULL,    A02=NULL,   BL=NULL, BR=NULL, B0=NULL, b1=NULL, B2=NULL,
              ABL=NULL, ABR=NULL,    a10t=NULL, alpha11=NULL, a12t=NULL,
             	                   A20=NULL,  a21=NULL,     A22=NULL;
  PLA_Obj MINUS_ONE=NULL, ZERO=NULL, ONE=NULL;
  int         b;

  PLA_Create_constants_conf_to( A, &MINUS_ONE, &ZERO, &ONE );

  PLA_Part_2x2( A,  &ATL, /**/ &ATR,
                  /* ************** */
                    &ABL, /**/ &ABR,   
		0, 0,     /* submatrix */ PLA_BR );

  PLA_Part_1x2( B,  &BL,  /**/ &BR,    
		0, /* width  submatrix */ PLA_RIGHT );

  while ( 0 != FLA_Obj_width( BL ) ){
    PLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00,   &a01,     /**/  &A02,
                                /**/              &a10t,  &alpha11, /**/  &a12t,
                        /* ************* */    /* ************************** */
                           ABL, /**/ ABR,         &A20,   &a21,     /**/  &A22,  
			   1, 1, /* alpha11 from */ PLA_TL );

    PLA_Repart_1x2_to_1x3( BL, /**/ BR,           &B0,  &b1, /**/  &B2,    
			   1, /* width b1 from */ PLA_LEFT );

    /* ********************************************************************* */

    if ( variant == PLA_VARIANT_LAZY )
      PLA_Gemv( PLA_NO_TRANSPOSE, MINUS_ONE, B2, a12t, ONE, b1 );

    PLA_Inv_scal( alpha11, b1 );

    if ( variant == PLA_VARIANT_COLLAZY )
      PLA_Ger( MINUS_ONE, b1, a01, B0 );
    
    /* ********************************************************************* */

    PLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,        A00,  /**/ a01,     A02,
                            /* ************** */   /* ********************* */
                                    /**/              a10t, /**/ alpha11, a12t,
                              &ABL, /**/ &ABR,        A20,  /**/ a21,     A22, 
			      /* alpha11 added to */ PLA_BR );

    PLA_Cont_with_1x3_to_1x2( &BL, /**/ &BR,           B0, /**/ b1,  B2, 
			      /* b1  added to */ PLA_RIGHT );
  }

  PLA_Obj_free( &ATL );
  PLA_Obj_free( &ATR );
  PLA_Obj_free( &ABL );
  PLA_Obj_free( &ABR );
  PLA_Obj_free( &A00 );
  PLA_Obj_free( &A01 );
  PLA_Obj_free( &A02 );
  PLA_Obj_free( &A10 );
  PLA_Obj_free( &A11 );
  PLA_Obj_free( &A12 );
  PLA_Obj_free( &A20 );
  PLA_Obj_free( &A21 );
  PLA_Obj_free( &A22 );
  PLA_Obj_free( &BT );
  PLA_Obj_free( &BB );
  PLA_Obj_free( &B0 );
  PLA_Obj_free( &B1 );
  PLA_Obj_free( &B2 );
  PLA_Obj_free( &MINUS_ONE );
  PLA_Obj_free( &ZERO );
  PLA_Obj_free( &ONE );

}





