LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_MODULE    := plasma
#LOCAL_SRC_FILES := plasma.c

ROOT = ../../
NDK = ../../../android-ndk-r10d/

LOCAL_MODULE    := xformRecon
LOCAL_SRC_FILES := recon.cpp\
		   $(ROOT)src/Filter.cpp\
		   $(ROOT)src/Warp.cpp\
		   $(ROOT)src/Recipe.cpp\
		   $(ROOT)src/TransformModel.cpp\
		   $(ROOT)src/XImage.cpp\

LOCAL_CFLAGS += -std=c++11 
LOCAL_CFLAGS += -Ofast
ARCH = armeabi-v7a
LOCAL_C_INCLUDES += $(ROOT)/src/
LOCAL_C_INCLUDES += $(ROOT)/third_party/eigen-eigen-1306d75b4a21/

include $(ROOT)/third_party/halide/apps/support/Makefile.inc    
LOCAL_C_INCLUDES += $(ROOT)/third_party/halide/apps/support/
LOCAL_C_INCLUDES += $(ROOT)/third_party/halide/apps/local_laplacian/

LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include/
LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ARCH)/include/

#LOCAL_MODULE    := ndk1
#LOCAL_SRC_FILES := naive.c

LOCAL_LDLIBS    := -lm -llog -ljnigraphics 

include $(BUILD_SHARED_LIBRARY)
