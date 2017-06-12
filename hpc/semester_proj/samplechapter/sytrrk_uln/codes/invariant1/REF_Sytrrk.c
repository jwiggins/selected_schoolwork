#include "FLAME.h"

/*
  REF_Sytrrk

  Purpose:  Reference implementation of C <- A A^T + C 
            where A is upper or lower triangular and C is symmetric,
            stored in the upper or lower triangular part of matrix C.

  if uplo_C = FLA_LOWER_TRIANGULAR only the lower triangular part of C is updated
  if uplo_C = FLA_UPPER_TRIANGULAR only the upper triangular part of C is updated

  uplo_A               trans             operation
  FLA_LOWER_TRIANGULAR FLA_NO_TRANSPOSE  C <- A A^T + C
                                         where A is lower triangular
  FLA_LOWER_TRIANGULAR FLA_TRANSPOSE     C <- A^T A + C
                                         where A is lower triangular
  FLA_UPPER_TRIANGULAR FLA_NO_TRANSPOSE  C <- A A^T + C
                                         where A is upper triangular
  FLA_UPPER_TRIANGULAR FLA_TRANSPOSE     C <- A^T A + C
                                         where A is upper triangular
*/

void REF_Sytrrk( int uplo_C, int uplo_A, int trans,
		 FLA_Obj A, FLA_Obj C )
{
  FLA_Obj Acopy;

  /* Copy A to Acopy */
  FLA_Obj_create_conf_to( FLA_NO_TRANSPOSE, A, &Acopy );
  FLA_Copy( A, Acopy );

  /* Set appropriate part of Acopy to zero */
  FLA_Triangularize( uplo_A, FLA_NONUNIT_DIAG, Acopy );

  FLA_Syrk( uplo_C, trans, ONE, Acopy, ONE, C );

  FLA_Obj_free( &Acopy );
}

