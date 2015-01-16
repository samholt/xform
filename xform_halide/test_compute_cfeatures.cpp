#include <fstream>
#include "halide_compute_cfeatures.h"
#include "static_image.h"
#include "image_io.h"
#include <iostream>
#include <sys/time.h>
using namespace std;

int main(int argc, char** argv){
  
  Image<float> input = load<float>(argv[1]);

  const int num_chrom_feat = 4;
  const int level = 3;
  Image<float> output(input.width(), 
                      input.height(), 
                      num_chrom_feat);

  halide_compute_cfeatures(input, output);
}
