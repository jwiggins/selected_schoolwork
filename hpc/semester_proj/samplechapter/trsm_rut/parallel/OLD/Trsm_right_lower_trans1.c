#include "PLA.h"

void Trsm_right_lower_trans( PLA_Obj A, PLA_Obj B, int nb_alg )
{
  PLA_Obj     ATL=NULL, ATR=NULL,     A00=NULL, A01=NULL, A02=NULL,
  ABL=NULL, ABR=NULL,     A10=NULL, A11=NULL, A12=NULL,
  A20=NULL, A21=NULL, A22=NULL,
    
  BL=NULL,  BR=NULL,  B0=NULL,  B1=NULL,  B2=NULL,
  MINUS_ONE=NULL, ZERO=NULL, ONE=NULL;
  int         b;

  PLA_Create_constants_conf_to( A, &MINUS_ONE, &ZERO, &ONE );
  PLA_Part_2x2( A,  &ATL,	/**/ &ATR,
	       /* ************** */
	       &ABL,		/**/ &ABR,   
	       0, 0,		/* submatrix */ PLA_TL );

  PLA_Part_1x2( B,  &BL,	/**/ &BR,    
	       0,		/* width  submatrix */ PLA_LEFT );



  while ( b = min( PLA_Obj_length( ABR ), nb_alg ) ){
    PLA_Repart_2x2_to_3x3( ATL, /**/ ATR,         &A00, /**/ &A01, &A02,
			  /* ************* */    /* ********************* */
			  /**/              &A10, /**/ &A11, &A12,
			  ABL,	/**/ ABR,         &A20, /**/ &A21, &A22,   
			  b, b, /* A11 from */ PLA_BR );

    PLA_Repart_1x2_to_1x3( BL,	/**/ BR,           &B0,  /**/ &B1,  &B2,    
			  b,	/* width B1 from */ PLA_RIGHT );

    /* ********************************************************************* */

    PLA_Trsm( PLA_RIGHT, PLA_LOWER_TRIANGULAR, PLA_TRANSPOSE, PLA_NONUNIT_DIAG, ONE, A11, B1 );
    PLA_Gemm( PLA_NO_TRANSPOSE, PLA_TRANSPOSE, MINUS_ONE, B1, A21, ONE, B2 );

    /* ********************************************************************* */

    PLA_Cont_with_3x3_to_2x2( &ATL, /**/ &ATR,         A00, A01, /**/ A02,
			     /**/               A10, A11, /**/ A12,
			     /* ************** */    /* ****************** */
			     &ABL, /**/ &ABR,         A20, A21, /**/ A22, 
			     /* A11 added to */ PLA_TL );

    PLA_Cont_with_1x3_to_1x2( &BL, /**/ &BR,           B0,  B1,  /**/ B2, 
			     /* B1  added to */ PLA_LEFT );
  }
  PLA_Obj_free( &ATL );
  PLA_Obj_free( &ATR );
  PLA_Obj_free( &A00 );
  PLA_Obj_free( &A01 );
  PLA_Obj_free( &A02 );
  PLA_Obj_free( &BL );
  PLA_Obj_free( &B0 );
  PLA_Obj_free( &ABL );
  PLA_Obj_free( &ABR );
  PLA_Obj_free( &A10 );
  PLA_Obj_free( &A11 );
  PLA_Obj_free( &A12 );
  PLA_Obj_free( &BR );
  PLA_Obj_free( &B1 );
  PLA_Obj_free( &A20 );
  PLA_Obj_free( &A21 );
  PLA_Obj_free( &A22 );
  PLA_Obj_free( &B2 );
  PLA_Obj_free( &MINUS_ONE );
  PLA_Obj_free( &ZERO );
  PLA_Obj_free( &ONE );
}
