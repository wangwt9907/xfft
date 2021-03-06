#include"hfft.h"

__global__ void d_hfft128x( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int bat )
{															
	extern __shared__ float smem[];
	float2 c[16], RF[15], temp;
	unsigned int slice=blockIdx.x*blockDim.y+threadIdx.y;
	if(slice>=bat) return;
	d_i+=((slice<<7)+threadIdx.x);
	d_o+=((slice<<7)+threadIdx.x);
	float* sst=&smem[168*threadIdx.y+threadIdx.x];				
	float* sld=&smem[168*threadIdx.y+9*threadIdx.x];			
	RF[0]=d_RF[threadIdx.x];								
	mLOAD16(c,d_i,8,)										
	mCALRF16(RF)											
	mFFT16(c,)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(sst,sld,c,9,72,1,0)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mISTORE8x2(d_o,c,8,16,)
}
__global__ void d_hifft128x( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int bat )
{															
	extern __shared__ float smem[];
	float2 c[16], RF[15], temp;
	unsigned int slice=blockIdx.x*blockDim.y+threadIdx.y;
	if(slice>=bat) return;
	d_i+=((slice<<7)+threadIdx.x);
	d_o+=((slice<<7)+threadIdx.x);
	float* sst=&smem[168*threadIdx.y+threadIdx.x];				
	float* sld=&smem[168*threadIdx.y+9*threadIdx.x];			
	RF[0]=d_RF[threadIdx.x];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,8,)										
	mCALRF16(RF)											
	mFFT16(c,i)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(sst,sld,c,9,72,1,0)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mISTORE8x2(d_o,c,8,16,)
}
__global__ void d_hfft128Sx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int n )
{
	__shared__ float smem[16*256];
	float2 c[16], RF[15], temp;
	unsigned int tidx=threadIdx.y*n+(blockIdx.x<<5)+threadIdx.x;
	d_i+=blockIdx.y*n*128+tidx;
	d_o+=blockIdx.y*n*128+tidx;
	float* sst=&smem[ 32*threadIdx.y+threadIdx.x];
	float* sld=&smem[256*threadIdx.y+threadIdx.x];
	RF[0]=d_RF[tidx];
	mLOAD16(c,d_i,8*n,)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	RF[0]=d_RF[(tidx&(n-1))<<4];
	mPERMUTE_S16_L8x2(sst,sld,c,256,2048,32,0x7)
	mCALRF8(RF)
	mFFT8(&c[0],)	
	mFFT8(&c[8],)	
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mISTORE8x2(d_o,c,8*n,16*n,)
}
__global__ void d_hifft128Sx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int n )
{
	__shared__ float smem[16*256];
	float2 c[16], RF[15], temp;
	unsigned int tidx=threadIdx.y*n+(blockIdx.x<<5)+threadIdx.x;
	d_i+=blockIdx.y*n*128+tidx;
	d_o+=blockIdx.y*n*128+tidx;
	float* sst=&smem[ 32*threadIdx.y+threadIdx.x];
	float* sld=&smem[256*threadIdx.y+threadIdx.x];
	RF[0]=d_RF[tidx];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,8*n,)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	RF[0]=d_RF[(tidx&(n-1))<<4];
	RF[0].y=-RF[0].y;
	mPERMUTE_S16_L8x2(sst,sld,c,256,2048,32,0x7)
	mCALRF8(RF)
	mFFT8(&c[0],i)	
	mFFT8(&c[8],i)	
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mISTORE8x2(d_o,c,8*n,16*n,)
}
__global__ void d_hfft128Tx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF )
{															
	__shared__ float smem[8*168];
	float2 c[16], RF[15], temp;
	d_i+=(blockIdx.y<<14)+(blockIdx.x<<10)+(threadIdx.y<<7)+threadIdx.x;
	d_o+=(blockIdx.y<<14)+(blockIdx.x<< 3)+(threadIdx.y<<7)+threadIdx.x;
	float* sst=&smem[168*threadIdx.y+threadIdx.x];				
	float* sld=&smem[168*threadIdx.x+9*threadIdx.y];			
	RF[0]=d_RF[threadIdx.x<<7];								
	mLOAD16(c,d_i,8,)										
	mCALRF16(RF)											
	mFFT16(c,)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(sst,sld,c,9,72,1,0x7)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mISTORE8x2(d_o,c,1024,2048,)
}
__global__ void d_hifft128Tx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF )
{															
	__shared__ float smem[8*168];
	float2 c[16], RF[15], temp;
	d_i+=(blockIdx.y<<14)+(blockIdx.x<<10)+(threadIdx.y<<7)+threadIdx.x;
	d_o+=(blockIdx.y<<14)+(blockIdx.x<< 3)+(threadIdx.y<<7)+threadIdx.x;
	float* sst=&smem[168*threadIdx.y+threadIdx.x];				
	float* sld=&smem[168*threadIdx.x+9*threadIdx.y];			
	RF[0]=d_RF[threadIdx.x<<7];		
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,8,)										
	mCALRF16(RF)											
	mFFT16(c,i)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(sst,sld,c,9,72,1,0x7)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mISTORE8x2(d_o,c,1024,2048,)
}
