#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <android/log.h>
#include <android/bitmap.h>
//#include "XImage.h"
//#include "Filter.h"
#include "static_image.h"
#include "TransformModel.h"

#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t createPixel(int r, int g, int b, int a) {
  return ((a & 0xff) << 24)
       | ((r & 0xff) << 16)
       | ((g & 0xff) << 8)
       | ((b & 0xff));
}
void copy_to_HImage(JNIEnv * env, const jobject& bitmap, Image<float>* input, int nchannel){

	AndroidBitmapInfo  info;
	uint32_t          *pixels;
	int                ret;

	AndroidBitmap_getInfo(env, bitmap, &info);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
	  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
	}

	void* bitmapPixels;
	AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
	uint32_t* src = (uint32_t*) bitmapPixels;
	int stride = info.stride;
	int pixelsCount = info.height * info.width;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "HEIGHT: [%d]",info.height);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WIDTH: [%d]", info.width);

	*input = Image<float>(info.width, info.height, nchannel);
	/* Parse */
	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y)
	      {
	    	uint32_t zz = src[info.width * y + x];
                float b = static_cast<float>((zz%256))/255.0f;  zz /= 256;
                float g = static_cast<float>((zz%256))/255.0f;  zz /= 256;
                float r = static_cast<float>((zz%256))/255.0f;  zz /= 256;
	    	(*input)(x,y,0) = r;

	    	if (nchannel > 1){
                  (*input)(x,y,1) = g;
                  (*input)(x,y,2) = b;
	    	}
	      }
	}
	//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "IMAGE is READ");
}

void copy_to_jBuffer(JNIEnv * env, const Image<float>& output, jobject& bitmap){

	AndroidBitmapInfo  info;
	uint32_t          *pixels;
	int                ret;

	AndroidBitmap_getInfo(env, bitmap, &info);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
	  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
	}

	void* bitmapPixels;
	AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
	uint32_t* src = (uint32_t*) bitmapPixels;
	int stride = info.stride;
	int pixelsCount = info.height * info.width;

	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "HEIGHT: [%d]",info.height);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WIDTH: [%d]", info.width);

	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y){
	    	int r = static_cast<int>(output(x,y,0)*255);
	    	int g = static_cast<int>(output(x,y,1)*255);
	    	int b = static_cast<int>(output(x,y,2)*255);
	    	src[info.width * y + x] = createPixel(r, g, b, 0xff);
            }
	}
}

void Java_com_example_uploadtoserver_UploadToServer_recon
  (JNIEnv * env, jobject obj, jobject input_bp, jobject ac_lumin_bp, jobject ac_chrom_bp, 
                            jobject dc_bp, jfloatArray meta)
{
        timeval t0, t_copy_recipe, t_copy_output, t_recon;
        gettimeofday(&t0, NULL);

        xform::TransformModel client_model;
        const int meta_len = 2 * (3 * client_model.num_affine + 
              client_model.num_scale-1 + client_model.num_bins-1);
	xform::PixelType* my_meta = new xform::PixelType[meta_len];
	jfloat* flt1 = env->GetFloatArrayElements(meta,0);
	for(int i=0; i < meta_len; i++)
          my_meta[i] = static_cast<xform::PixelType>(flt1[i]);

        /* Recon by Halide */
        Image<float> HL_input, HL_ac_lumin, HL_ac_chrom, HL_dc;
        copy_to_HImage(env, input_bp, &HL_input, 3);
        copy_to_HImage(env, ac_lumin_bp, &HL_ac_lumin, 1);
        copy_to_HImage(env, ac_chrom_bp, &HL_ac_chrom, 1);
        copy_to_HImage(env, dc_bp, &HL_dc, 3);

        Image<float> HL_output(HL_input.width(), HL_input.height(), HL_input.channels());

        gettimeofday(&t_copy_recipe, NULL);
        //client_model_halide.reconstruct_by_Halide(HL_input, HL_ac, HL_dc, my_meta, &HL_output);
        client_model.reconstruct_separate_by_Halide(HL_input, HL_ac_lumin, HL_ac_chrom, HL_dc, my_meta, &HL_output);
        gettimeofday(&t_recon, NULL);

        copy_to_jBuffer(env, HL_output, input_bp);

        gettimeofday(&t_copy_output, NULL);

	AndroidBitmap_unlockPixels(env, input_bp);
	AndroidBitmap_unlockPixels(env, ac_lumin_bp);
	AndroidBitmap_unlockPixels(env, ac_chrom_bp);
	AndroidBitmap_unlockPixels(env, dc_bp);

        unsigned int t1 = (t_copy_recipe.tv_sec - t0.tv_sec) * 1000000 + (t_copy_recipe.tv_usec - t0.tv_usec);
        unsigned int t2 = (t_recon.tv_sec - t_copy_recipe.tv_sec) * 1000000 + (t_recon.tv_usec - t_copy_recipe.tv_usec);
        unsigned int t3 = (t_copy_output.tv_sec - t_recon.tv_sec) * 1000000 + (t_copy_output.tv_usec - t_recon.tv_usec);

        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "T_COPY_RECIPE [%d]", t1);
        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "T_RECON [%d]", t2);
        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "T_COPY_OUTPUT [%d]", t3);
}
void copy_to_XImage(JNIEnv * env, const jobject& bitmap, xform::XImage* input){

	AndroidBitmapInfo  info;
	uint32_t          *pixels;
	int                ret;

	AndroidBitmap_getInfo(env, bitmap, &info);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
	  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
	}

	void* bitmapPixels;
	AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
	uint32_t* src = (uint32_t*) bitmapPixels;
	int stride = info.stride;
	int pixelsCount = info.height * info.width;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "HEIGHT: [%d]",info.height);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WIDTH: [%d]", info.width);

    *input = xform::XImage(info.height, info.width, 3);

	/* Parse */
	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y)
	      {
	    	uint32_t zz = src[info.width * y + x];
                xform::PixelType b = static_cast<xform::PixelType>((zz%256))/255.0;  zz /= 256;
                xform::PixelType g = static_cast<xform::PixelType>((zz%256))/255.0;  zz /= 256;
                xform::PixelType r = static_cast<xform::PixelType>((zz%256))/255.0;  zz /= 256;
	    	input->at(0)(y,x) = r;
	    	input->at(1)(y,x) = g;
	    	input->at(2)(y,x) = b;
	    	//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%d]", (int)(r*255));
	      }
	}
}
xform::PixelType myClamp(xform::PixelType in){
	xform::PixelType out =0;
	if (in > 1.0f)
		out = 1.0;
	else if (in < 0.0f)
		out = 0.0;
	else
		out = in;
	return out;
}
void write_to_bitmap(JNIEnv * env, const xform::XImage& output, jobject& bitmap){


	AndroidBitmapInfo  info;
	uint32_t          *pixels;
	int                ret;

	AndroidBitmap_getInfo(env, bitmap, &info);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
	  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
	}

	void* bitmapPixels;
	AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
	uint32_t* src = (uint32_t*) bitmapPixels;
	int stride = info.stride;
	int pixelsCount = info.height * info.width;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "HEIGHT: [%d]",info.height);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WIDTH: [%d]", info.width);

	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y)
	      {
	    	int r = static_cast<int>((output.at(0)(y,x)) * 255.0);
	    	int g = static_cast<int>((output.at(1)(y,x)) * 255.0);
	    	int b = static_cast<int>((output.at(2)(y,x)) * 255.0);
	    	src[info.width * y + x] = createPixel(r, g, b, 0xff);
	      }
	}
}
	/*xform::XImage input,ac, dc;
	copy_to_XImage(env, input_bp, &input);
	copy_to_XImage(env, ac_bp, &ac);
	copy_to_XImage(env, dc_bp, &dc);

	// Recon
	xform::TransformModel client_model;
	client_model.set_from_recipe(input, ac.at(0), dc, my_meta);
	xform::XImage output  = client_model.predict();

	// Clamping
	for(int k=0; k < 3; k++){
		for(int i = 0 ; i < output.rows(); i++){
			for(int j = 0; j < output.cols(); j++){
				if (output.at(k)(i,j) > 1.0)
					output.at(k)(i,j)=1;
				if (output.at(k)(i,j) < 0.0)
					output.at(k)(i,j)=0;}}}
	write_to_bitmap(env, output, input_bp);*/


#ifdef __cplusplus
}
#endif
