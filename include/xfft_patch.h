#ifndef __xfft_patch_h__
#define __xfft_patch_h__

#include"xfft_kernel.h"

void fft2d_patch_bki( xfft_kernel_t* const, CUmodule, CUdeviceptr, int, int );

#endif