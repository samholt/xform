#include "Recipe.h"

using namespace xform;

Recipe::Recipe(int num_rows, int num_cols, int n_chan_i, int n_chan_o):
    height(num_rows), width(num_cols),
    n_chan_i(n_chan_i), n_chan_o(n_chan_o)
{
    dc = XImage(num_rows, num_cols, n_chan_o);
    ac = ImageType_1(num_rows*n_chan_i, num_cols*n_chan_o);
    quantize_mins = new PixelType[n_chan_i*n_chan_o];
    quantize_maxs = new PixelType[n_chan_i*n_chan_o];
    quantize_levels = 255;
}

Recipe::~Recipe(){
    if(quantize_mins){
        delete quantize_mins;
        quantize_mins = NULL;
    }
    if(quantize_maxs){
        delete quantize_maxs;
        quantize_maxs = NULL;
    }
}

bool Recipe::read(const std::string& filename) {
    return true;
}
bool Recipe::write(const std::string& filename) {
    imwrite(this->ac, filename+"_ac.png");
    this->dc.write(filename+"_dc.png");
    return true;
}

void Recipe::set_dc(XImage& src) {
    dc = src;
}

XImage& Recipe::get_dc() {
    return dc;
}

void Recipe::set_coefficients(int i, int j, const MatType &coef) {
    for(int in_chan = 0; in_chan < coef.rows(); ++in_chan)
    for(int out_chan = 0; out_chan < coef.cols(); ++out_chan)
    {
        int ac_map_i = in_chan*height + i;
        int ac_map_j = out_chan*width +j;
        ac(ac_map_i, ac_map_j) = coef(in_chan,out_chan);
    }
}

void Recipe::get_coefficients(int i, int j, MatType &coef) {
    for(int in_chan = 0; in_chan < n_chan_i ; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o ; ++out_chan)
    {
        int ac_map_i = in_chan*height + i;
        int ac_map_j = out_chan*width +j;
        coef(in_chan,out_chan) = ac(ac_map_i, ac_map_j);
    }
}

void Recipe::quantize() {
    // Uniform quantize each coefficient map separately
    for(int in_chan = 0; in_chan < n_chan_i; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o; ++out_chan)
    {

        PixelType mini = ac.block(in_chan*height, out_chan*width, height, width).minCoeff();
        PixelType maxi = ac.block(in_chan*height, out_chan*width, height, width).maxCoeff();
        quantize_mins[in_chan*n_chan_o + out_chan] = mini;
        quantize_maxs[in_chan*n_chan_o + out_chan] = maxi;
        PixelType rng = maxi-mini;
        if(rng == 0){
            rng = 1;
        }
        for(int i = 0; i<height; ++i)
        for(int j = 0; j<width; ++j)
        {
            int ac_map_i = in_chan*height + i;
            int ac_map_j = out_chan*width +j;
            ac(ac_map_i,ac_map_j) -= mini;
            ac(ac_map_i,ac_map_j) /= rng;
            ac(ac_map_i,ac_map_j) = floor(ac(ac_map_i,ac_map_j)*quantize_levels)/quantize_levels ;
        }
    }
}

void Recipe::dequantize() {
    // Uniform quantize each coefficient map separately
    for(int in_chan = 0; in_chan < n_chan_i; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o; ++out_chan)
    {

        PixelType mini = quantize_mins[in_chan*n_chan_o + out_chan];
        PixelType maxi = quantize_maxs[in_chan*n_chan_o + out_chan];
        PixelType rng = maxi-mini;
        if(rng == 0){
            rng = 1;
        }
        for(int i = 0; i<height; ++i)
        for(int j = 0; j<width; ++j)
        {
            int ac_map_i = in_chan*height + i;
            int ac_map_j = out_chan*width +j;
            ac(ac_map_i,ac_map_j) *= rng;
            ac(ac_map_i,ac_map_j) += mini;
        }
    }
}
