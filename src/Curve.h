#include <Eigen/Dense>
#include "XImage.h"

#ifndef SRC_CURVE_H
#define SRC_CURVE_H

namespace xform{

/* Global curve for tone adjusemtn*/

class Curve{
 public:
  Curve(){};
  // r(i) = g0 + sign(i-g0) * sigma * (|i-g0|/sigma)^alpha0
  void sShape(const ImageType_1& im_in, const PixelType sigma, 
            const PixelType g0, const double alpha, ImageType_1* im_out) const;

  void sShape(const PixelType& in, const PixelType sigma, 
            const PixelType g0, const double alpha, PixelType* out) const;

  void expShape(const ImageType_1& im_in) const; //More parameters...


};

} // namespace xform

#endif // SRC_CURVE_H
