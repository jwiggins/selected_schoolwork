#ifndef PROK_QUATERNION_H
#define PROK_QUATERNION_H

#include <math.h>

class Quaternion {
public:
                        Quaternion();
                        Quaternion(double real, double ix, double iy, double iz);
                        Quaternion(const Quaternion &q);

  static Quaternion     RotationQuat(double theta, double ax, double ay, double az);

  double                Magnitude() const;
  Quaternion            Conjugate() const;
  
  void                  Normalize();

  Quaternion            operator*(const Quaternion &q) const;

  void                  RotationMatrix(double fourXfour[]) const;
private:
  double                w,x,y,z;
};

inline Quaternion::Quaternion()
{
  w = 1.0;
  x = 0.0;
  y = 0.0;
  z = 0.0;
}

inline Quaternion::Quaternion(double real, double ix, double iy, double iz)
{
  w = real;
  x = ix;
  y = iy;
  z = iz;
}

inline Quaternion::Quaternion(const Quaternion &q)
{
  w = q.w;
  x = q.x;
  y = q.y;
  z = q.z;
}

inline Quaternion Quaternion::RotationQuat(double theta, double ax,
                                                double ay, double az)
{
  Quaternion result;
  double s = sin(theta * 0.5);
  
  result.w = cos(theta * 0.5);
  result.x = s*ax;
  result.y = s*ay;
  result.z = s*az;
  
  result.Normalize();
  
  return result;
}

inline double Quaternion::Magnitude() const
{
  return sqrt(w*w + x*x + y*y + z*z);
}

inline Quaternion Quaternion::Conjugate() const
{
  Quaternion result(w, -x, -y, -z);

  return result;
}

inline void Quaternion::Normalize()
{
  double mag = Magnitude();
  
  if (mag != 0.0)
  {
    w /= mag;
    x /= mag;
    y /= mag;
    z /= mag;
  }
}

inline Quaternion Quaternion::operator*(const Quaternion &q) const
{
  Quaternion result;

  result.w = w*q.w - x*q.x - y*q.y - z*q.z;
  result.x = y*q.z - z*q.y + w*q.x + q.w*x;
  result.y = z*q.x - x*q.z + w*q.y + q.w*y;
  result.z = x*q.y - y*q.x + w*q.z + q.w*z;

  return result;
}

inline void Quaternion::RotationMatrix(double fourXfour[]) const
{
  double two_xy, two_wz, two_xz, two_wy, two_yz, two_wx;
  double w_sqr, x_sqr, y_sqr, z_sqr;
  
  two_xy = 2.0*x*y;
  two_wz = 2.0*w*z;
  two_xz = 2.0*x*z;
  two_wy = 2.0*w*y;
  two_yz = 2.0*y*z;
  two_wx = 2.0*w*x;
  w_sqr = w*w;
  x_sqr = x*x;
  y_sqr = y*y;
  z_sqr = z*z;

  fourXfour[0] = w_sqr + x_sqr - y_sqr - z_sqr;
  fourXfour[1] = two_xy + two_wz;
  fourXfour[2] = two_xz - two_wy;
  fourXfour[3] = 0.0;

  fourXfour[4] = two_xy - two_wz;
  fourXfour[5] = w_sqr - x_sqr + y_sqr - z_sqr;
  fourXfour[6] = two_yz + two_wx;
  fourXfour[7] = 0.0;

  fourXfour[8] = two_xz + two_wy;
  fourXfour[9] = two_yz - two_wx;
  fourXfour[10] = w_sqr - x_sqr - y_sqr + z_sqr;
  fourXfour[11] = 0.0;

  fourXfour[12] = 0.0;
  fourXfour[13] = 0.0;
  fourXfour[14] = 0.0;
  fourXfour[15] = 1.0; // this should be a unit quaternion
                  // = w_sqr + x_sqr + y_sqr + z_sqr;
}

#endif /* PROK_QUATERNION_H */
