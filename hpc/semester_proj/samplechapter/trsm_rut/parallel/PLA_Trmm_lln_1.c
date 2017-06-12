#include "PLA.h"

void Trmm_lln_rowlazy_wrt_L_rec( PLA_Obj A, PLA_Obj B, int nb_alg )
{
  PLA_Obj     ATL=NULL, ATR=NULL,     A00=NULL, A01=NULL, A02=NULL,    BT=NULL,           B0=NULL,
              ABL=NULL, ABR=NULL,     A10=NULL, A11=NULL, A12=NULL,    BB=NULL,           B1=NULL,
                            A20=NULL, A21=NULL, A22=NULL,                  B2=NULL;
  PLA_Obj MINUS_ONE=NULL, ZERO=NULL, ONE=NULL;
  int         b;

  PLA_Create_constants_conf_to( A, &MINUS_ONE, &ZERO, &ONE );

  PLA_Part_2x2( A,  &ATL, /**/ &ATR,
                  /* ************** */
                    &ABL, /**/ &ABR,   0, 0,     /* submatrix */ PLA_BR );

  PLA_Part_2x1( B,  &BT, 
                   /***/
                    &BB,               0, /* length submatrix */ PLA_BOTTOM );

  while ( b = min( PLA_Obj_length( ATR ), nb_alg ) ){
    PLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00, &A01, /**/ &A02,
                                /**/              &A10, &A11, /**/ &A12,
                        /* ************* */    /* ********************* */
                           ABL, /**/ ABR,         &A20, &A21, /**/ &A22,   
                           b, b, /* A11 from */ PLA_TL );

    PLA_Repart_2x1_to_3x1( BT,                   &B0,
                                                 &B1,
                          /**/                   /**/
                           BB,                   &B2,    
                           b, /* length B1 from */ PLA_TOP );

    /* ********************************************************************* */

    if ( b <= 8 )
      // Trmm_lln_rowlazy_wrt_L_unb( A11, B1 );
      PLA_Trmm( PLA_SIDE_LEFT, PLA_LOWER_TRIANGULAR,
		PLA_NO_TRANSPOSE, PLA_NONUNIT_DIAG,
		ONE, A11, B1 );
    else
      Trmm_lln_rowlazy_wrt_L_rec( A11, B1, nb_alg/2 );

    PLA_Gemm( PLA_NO_TRANSPOSE, PLA_NO_TRANSPOSE, ONE, A10, B0, ONE, B1 );

    /* ********************************************************************* */

    PLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,         A00, /**/ A01, A02,
                            /* ************** */   /* ****************** */
                                    /**/               A10, /**/ A11, A12,
                              &ABL, /**/ &ABR,         A20, /**/ A21, A22, 
			      /* A11 added to */ PLA_BR );

    PLA_Cont_with_3x1_to_2x1( &BT,                     B0,
                             /***/                    /**/
                                                       B1,
                              &BB,                     B2,                 
			      /* B1  added to */ PLA_BOTTOM );
  }

  PLA_Obj_free( &ATL );
  PLA_Obj_free( &ATR );
  PLA_Obj_free( &A00 );
  PLA_Obj_free( &A01 );
  PLA_Obj_free( &A02 );
  PLA_Obj_free( &BT );
  PLA_Obj_free( &B0 );
  PLA_Obj_free( &ABL );
  PLA_Obj_free( &ABR );
  PLA_Obj_free( &A10 );
  PLA_Obj_free( &A11 );
  PLA_Obj_free( &A12 );
  PLA_Obj_free( &BB );
  PLA_Obj_free( &B1 );
  PLA_Obj_free( &A20 );
  PLA_Obj_free( &A21 );
  PLA_Obj_free( &A22 );
  PLA_Obj_free( &B2 );
  PLA_Obj_free( &MINUS_ONE );
  PLA_Obj_free( &ZERO );
  PLA_Obj_free( &ONE );
}
