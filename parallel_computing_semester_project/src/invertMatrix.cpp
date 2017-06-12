#include "invertMatrix.h"
#include <math.h>

double m3_det(double *mat)
{
  double det;

  det = mat[0] * (mat[4]*mat[8] - mat[7]*mat[5])
      - mat[1] * (mat[3]*mat[8] - mat[6]*mat[5])
      + mat[2] * (mat[3]*mat[7] - mat[6]*mat[4]);

  return det;
}

void m4_submat(double *mr, double *mb, int i, int j)
{
  int ti, tj, idst, jdst;

  for (ti = 0; ti < 4; ti++)
  {
    if (ti < i)
      idst = ti;
    else if (ti > i)
      idst = ti-1;

    for (tj = 0; tj < 4; tj++)
    {
      if (tj < j)
        jdst = tj;
      else if (tj > j)
        jdst = tj-1;

      if (ti != i && tj != j)
        mb[idst*3 + jdst] = mr[ti*4 + tj ];
    }
  }
}

double m4_det(double *mr)
{
  double det, result = 0.0, i = 1.0, msub3[9];
  int n;

  for (n = 0; n < 4; n++, i *= -1.0)
  {
    m4_submat(mr, msub3, 0, n);

    det = m3_det(msub3);
    result += mr[n] * det * i;
  }

  return result;
}

int m4_inverse(double *mr, double *ma)
{
  double mtemp[9], mdet = m4_det(ma);
  int i, j, sign;

  if (fabs(mdet) == 0.0)
    return 0;

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 4; j++)
    {
      sign = 1 - ((i +j) % 2) * 2;

      m4_submat(ma, mtemp, i, j);

      mr[i+j*4] = (m3_det(mtemp) * sign) / mdet;
    }
  }

  return 1;
}

void m4_multiply(double *mr, double *ma, double *mb)
{
  int i,j;

  // i = column, j = row
  for (i=0; i < 4; i++)
  {
    for (j=0; j < 4; j++)
    {
      // ma[row j] * mb[column i]
      mr[i*4+j] = ma[(i+0)*4+j] * mb[i*4+j+0]
                + ma[(i+1)*4+j] * mb[i*4+j+1]
                + ma[(i+2)*4+j] * mb[i*4+j+2]
                + ma[(i+3)*4+j] * mb[i*4+j+3];
    }
  }
}

