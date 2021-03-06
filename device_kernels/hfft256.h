#include"hfft.h"

__global__ void d_hfft256x( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int bat )
{															
	extern __shared__ float smem[];						
	float2 c[16], RF[15], temp;
	unsigned int slot=blockIdx.x*blockDim.y+threadIdx.y;
	if(slot>=bat) return;
	d_i+=((slot<<8)+threadIdx.x);	
	d_o+=((slot<<8)+threadIdx.x);
	float* sst=&smem[272*threadIdx.y+threadIdx.x];
	float* sld=&smem[272*threadIdx.y+17*threadIdx.x];
	RF[0]=d_RF[threadIdx.x];	
	mLOAD16(c,d_i,16,)										
	mCALRF16(RF)										
	mFFT16(c,)											
	mHMRF16(c,RF)											
	mPERMUTE(16,sst,sld,c,17,1,0)							
	mFFT16(c,)											
	mISTORE16(d_o,c,16,)										
}
__global__ void d_hifft256x( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int bat )
{															
	extern __shared__ float smem[];						
	float2 c[16], RF[15], temp;	
	unsigned int slot=blockIdx.x*blockDim.y+threadIdx.y;
	if(slot>=bat) return;
	d_i+=((slot<<8)+threadIdx.x);	
	d_o+=((slot<<8)+threadIdx.x);
	float* sst=&smem[272*threadIdx.y+threadIdx.x];
	float* sld=&smem[272*threadIdx.y+17*threadIdx.x];
	RF[0]=d_RF[threadIdx.x];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,16,)										
	mCALRF16(RF)											
	mFFT16(c,i)											
	mHMRF16(c,RF)											
	mPERMUTE(16,sst,sld,c,17,1,0)							
	mFFT16(c,i)											
	mISTORE16(d_o,c,16,)										
}
__global__ void __launch_bounds__(512,1) d_hfft256Sx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF )
{
	__shared__ float smem[16*512];
	float2 c[16], RF[15], temp;
	unsigned int tidx=(threadIdx.y<<8)+(blockIdx.x<<5)+threadIdx.x;
	d_i+=(blockIdx.y<<16)+tidx;
	d_o+=(blockIdx.y<<16)+tidx;
	float* sst=&smem[ 32*threadIdx.y+threadIdx.x];
	float* sld=&smem[512*threadIdx.y+threadIdx.x];
	RF[0]=d_RF[tidx];
	mLOAD16(c,d_i,4096,)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	RF[0]=d_RF[(tidx&255)<<4];
	mPERMUTE(16,sst,sld,c,512,32,7)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	mISTORE16(d_o,c,4096,)
}
__global__ void __launch_bounds__(512,1) d_hifft256Sx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF )
{
	__shared__ float smem[16*512];
	float2 c[16], RF[15], temp;
	unsigned int tidx=(threadIdx.y<<8)+(blockIdx.x<<5)+threadIdx.x;
	d_i+=(blockIdx.y<<16)+tidx;
	d_o+=(blockIdx.y<<16)+tidx;
	float* sst=&smem[ 32*threadIdx.y+threadIdx.x];
	float* sld=&smem[512*threadIdx.y+threadIdx.x];
	RF[0]=d_RF[tidx];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,4096,)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	RF[0]=d_RF[(tidx&255)<<4];
	RF[0].y=-RF[0].y;
	mPERMUTE(16,sst,sld,c,512,32,7)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	mISTORE16(d_o,c,4096,)
}
__global__ void __launch_bounds__(256,2) d_hfft256Tx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int n )
{															
	__shared__ float smem[16*272];						
	float2 c[16], RF[15], temp;
	d_i+=blockIdx.y*n*256+(blockIdx.x<<12)+(threadIdx.y<<8)+threadIdx.x;
	d_o+=blockIdx.y*n*256+(blockIdx.x<<4)+threadIdx.y*n+threadIdx.x;
	float* sst=&smem[272*threadIdx.y+threadIdx.x];
	float* sld=&smem[272*threadIdx.x+17*threadIdx.y];
	RF[0]=d_RF[threadIdx.x*n];
	mLOAD16(c,d_i,16,)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	mPERMUTE(16,sst,sld,c,17,1,7)
	mFFT16(c,)
	mISTORE16(d_o,c,16*n,)
}
__global__ void __launch_bounds__(256,2) d_hifft256Tx( float2* d_o, const float2* __restrict__ d_i, const float2* d_RF, int n )
{															
	__shared__ float smem[16*272];						
	float2 c[16], RF[15], temp;
	d_i+=blockIdx.y*n*256+(blockIdx.x<<12)+(threadIdx.y<<8)+threadIdx.x;
	d_o+=blockIdx.y*n*256+(blockIdx.x<<4)+threadIdx.y*n+threadIdx.x;
	float* sst=&smem[272*threadIdx.y+threadIdx.x];
	float* sld=&smem[272*threadIdx.x+17*threadIdx.y];
	RF[0]=d_RF[threadIdx.x*n];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,16,)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	mPERMUTE(16,sst,sld,c,17,1,7)
	mFFT16(c,i)
	mISTORE16(d_o,c,16*n,)
}