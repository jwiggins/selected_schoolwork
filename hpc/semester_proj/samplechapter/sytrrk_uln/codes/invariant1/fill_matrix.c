/*
Copyright (C) 2001 the University of Texas at Austin (U. T. Austin)

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Submit software operation questions to: Robert van de Geijn,
Department of Computer Sciences,U. T. Austin, Austin, Texas 78712
(rvdg@cs.utexas.edu).

Submit commercialization requests to: Office of the Executive Vice
President and Provost, U. T. Austin, 201 Main Bldg., Austin, Texas,
78712, ATTN: Technology Licensing Specialist.
*/

#include "FLAME.h"

#define BUFFER( i, j ) buff[ (j)*lda + (i) ]

void random_matrix( FLA_Obj A )
{
  int datatype, m, n, lda;
  
  datatype = FLA_Obj_datatype( A );
  m        = FLA_Obj_length( A );
  n        = FLA_Obj_width ( A );
  lda      = FLA_Obj_ldim  ( A );

  if ( datatype == FLA_DOUBLE ){
    double *buff;
    int    i, j;

    buff = ( double * ) FLA_Obj_buffer( A );

    for ( j=0; j<n; j++ )
      for ( i=0; i<m; i++ )
	BUFFER( i,j ) = i+j*0.01;
  }
  else FLA_Abort( "Datatype not yet supported", __LINE__, __FILE__ );
}
