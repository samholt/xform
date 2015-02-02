#include <Halide.h>
#include "tools/resample.h"
#include "tools/color_transform.h"
#include "tools/resize.h"

using namespace Halide;

int main(int argc, char **argv){

Var x("x"), y("y"), xi("xi"), xo("xo"), yi("yi"), yo("yo"), c("c"),
    k("k"), ni("ni"), no("no");

  const int J = 5;
  const int step = 16;
  const float scaleFactor = float(std::pow(2, J-1));

  ImageParam input(Float(32), 3);
  Param<int>  level;

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);

  //Func ds("ds");
  //ds(x,y,c) = downsample_n(my_yuv, J)(x,y,c);

  Func ds("ds");
  Func ds_x("ds_x");
  ds_x(x, y, c) = resize_x(my_yuv, 1.0/scaleFactor)(x, y, c);
  ds(x, y, c) = resize_y(ds_x, 1.0/scaleFactor)(x, y, c);

   // YUV2RGB
  Func rgb_out("rgb_out");
  rgb_out(x, y, c) = yuv2rgb(ds)(x, y, c);

  Func final("final");
  final(x, y, c) = clamp(rgb_out(x, y, c), 0.0f, 1.0f);

  /* Scheduling */
  final.split(y, yo, yi, 4).parallel(yo); 
  ds_x.store_at(final, yo).compute_at(final, yi);
  
  std::vector<Argument> args(1);
  args[0] = input;
  final.compile_to_file("halide_downsample", args);

  return 0;
}
