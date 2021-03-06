#include"hfft.h"
#include"cuintrin.h"

__global__ void d_fft4x2x( float2* d_c, int bat )
{													
	float2 c[8], temp;						
	unsigned int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_c+=(tidx<<3)+threadIdx.x;
	mLOAD8(c,d_c,1,)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	BFLYU10(c[0],c[4])
	BFLYU10(c[1],c[5])
	BFLYU10(c[2],c[6])
	BFLYU10(c[3],c[7])
	mISTORE4x2(d_c,c,4,1,)
}
__global__ void d_ifft4x2x( float2* d_c, int bat )
{													
	float2 c[8], temp;						
	unsigned int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_c+=(tidx<<3)+threadIdx.x;
	mLOAD8(c,d_c,1,)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	BFLYU10(c[0],c[4])
	BFLYU10(c[1],c[5])
	BFLYU10(c[2],c[6])
	BFLYU10(c[3],c[7])
	mISTORE4x2(d_c,c,4,1,)
}
__global__ void d_fft4x4x( float2* d_c, int bat )
{											
	float2 c[16], temp;						
	unsigned int slice_id=blockIdx.x*blockDim.x+threadIdx.x;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<4);	
	mLOAD16(c,d_c,1,)
	mFFT4(&c[ 0],)
	mFFT4(&c[ 4],)
	mFFT4(&c[ 8],)
	mFFT4(&c[12],)
	
	BFLYU10(c[0],c[ 8])
	BFLYU10(c[1],c[ 9])
	BFLYU10(c[2],c[10])
	BFLYU10(c[3],c[11])

	BFLYU10(c[4],c[12])
	BFLYU10(c[5],c[13])
	BFLYU10(c[6],c[14])
	BFLYU10(c[7],c[15])

	BFLYU10(c[0],c[4])
	BFLYU10(c[1],c[5])
	BFLYU10(c[2],c[6])
	BFLYU10(c[3],c[7])

	BFLYU01(c[ 8],c[12])
	BFLYU01(c[ 9],c[13])
	BFLYU01(c[10],c[14])
	BFLYU01(c[11],c[15])

	mISTORE16(d_c,c,1,)
}
__global__ void d_ifft4x4x( float2* d_c, int bat )
{											
	float2 c[16], temp;						
	unsigned int slice_id=blockIdx.x*blockDim.x+threadIdx.x;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<4);	
	mLOAD16(c,d_c,1,)
	mFFT4(&c[ 0],i)
	mFFT4(&c[ 4],i)
	mFFT4(&c[ 8],i)
	mFFT4(&c[12],i)
	
	BFLYU10(c[0],c[ 8])
	BFLYU10(c[1],c[ 9])
	BFLYU10(c[2],c[10])
	BFLYU10(c[3],c[11])

	BFLYU10(c[4],c[12])
	BFLYU10(c[5],c[13])
	BFLYU10(c[6],c[14])
	BFLYU10(c[7],c[15])

	BFLYU10(c[0],c[4])
	BFLYU10(c[1],c[5])
	BFLYU10(c[2],c[6])
	BFLYU10(c[3],c[7])

	iBFLYU01(c[ 8],c[12])
	iBFLYU01(c[ 9],c[13])
	iBFLYU01(c[10],c[14])
	iBFLYU01(c[11],c[15])

	mISTORE4(d_c+0*4,&c[ 0],1,)
	mISTORE4(d_c+1*4,&c[ 8],1,)
	mISTORE4(d_c+2*4,&c[ 4],1,)
	mISTORE4(d_c+3*4,&c[12],1,)
}

__global__ void d_fft4x16x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[80*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	RF[0]=d_RF[slot];
	mLOAD4(c,d_c,16,)
	mCALRF4(RF)
	mFFT4(c,)
	mHMRF4(c,RF)
	mISTORE4(spx,c,20,.x)
	mLOAD4(c,spy,4,.x)
	mISTORE4(spx,c,20,.y)
	mLOAD4(c,spy,4,.y)
	mFFT4(c,)
	mISTORE4(spy,c,5,.x)
	mLOAD4(c,spz,1,.x)
	mISTORE4(spy,c,5,.y)
	mLOAD4(c,spz,1,.y)
	mFFT4(c,)
	mISTORE4(spz,c,1,.x)
	mLOAD4(c,spy,5,.x)
	mISTORE4(spz,c,1,.y)
	mLOAD4(c,spy,5,.y)
	mSTORE4(d_c,c,16,)
}
__global__ void d_ifft4x16x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[80*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	RF[0]=d_RF[slot];
	RF[0].y=-RF[0].y;
	mLOAD4(c,d_c,16,)
	mCALRF4(RF)
	mFFT4(c,i)
	mHMRF4(c,RF)
	mISTORE4(spx,c,20,.x)
	mLOAD4(c,spy,4,.x)
	mISTORE4(spx,c,20,.y)
	mLOAD4(c,spy,4,.y)
	mFFT4(c,i)
	mISTORE4(spy,c,5,.x)
	mLOAD4(c,spz,1,.x)
	mISTORE4(spy,c,5,.y)
	mLOAD4(c,spz,1,.y)
	mFFT4(c,i)
	mISTORE4(spz,c,1,.x)
	mLOAD4(c,spy,5,.x)
	mISTORE4(spz,c,1,.y)
	mLOAD4(c,spy,5,.y)
	mSTORE4(d_c,c,16,)
}
__global__ void d_fft4x64x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[320*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[40*slot+lane];
	float* spz=&sbase[5*threadIdx.x];
	RF[0]=d_RF[slot];
	mLOAD8(c,d_c,32,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spx,c,40,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE8(spx,c,40,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,)
	mISTORE8(spy,c,5,.x)
	mLOAD4x2(c,spz,160,1,.x)
	mISTORE8(spy,c,5,.y)
	mLOAD4x2(c,spz,160,1,.y)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spz,c,160,1,.x)
	mLOAD8(c,spy,5,.x)
	mISTORE4x2(spz,c,160,1,.y)
	mLOAD8(c,spy,5,.y)
	mSTORE8(d_c,c,32,)
}
__global__ void d_ifft4x64x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[320*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[40*slot+lane];
	float* spz=&sbase[5*threadIdx.x];
	RF[0]=d_RF[slot];
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,32,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spx,c,40,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE8(spx,c,40,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,i)
	mISTORE8(spy,c,5,.x)
	mLOAD4x2(c,spz,160,1,.x)
	mISTORE8(spy,c,5,.y)
	mLOAD4x2(c,spz,160,1,.y)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spz,c,160,1,.x)
	mLOAD8(c,spy,5,.x)
	mISTORE4x2(spz,c,160,1,.y)
	mLOAD8(c,spy,5,.y)
	mSTORE8(d_c,c,32,)
}