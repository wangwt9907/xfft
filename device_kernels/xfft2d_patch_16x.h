#include"hfft.h"
#include"cuintrin.h"

/*
16x2   : block( 8,n)
16x4   : block(16,n)
16x8   : block(16,n)
16x16  : block(16,n)
16x32  : block(16,4)
16x64  : block(16,8)
16x128 : block(16,8)
*/

__global__ void d_fft16x2x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[40*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	RF[0]=d_RF[lane];
	mLOAD2(&c[0],d_c+0*8,16,)
	mLOAD2(&c[2],d_c+1*8,16,)
	mCALRF4(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mISTORE2(spx+0*8,&c[0],20,.x)
	mISTORE2(spx+1*8,&c[2],20,.x)
	mLOAD4(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],20,.y)
	mISTORE2(spx+1*8,&c[2],20,.y)
	mLOAD4(c,spy,4,.y)
	mFFT4(c,)
	mHMRF4(c,RF)
	mISTORE4(spy,c,5,.x)
	mLOAD4(c,spz,1,.x)
	mISTORE4(spy,c,5,.y)
	mLOAD4(c,spz,1,.y)
	mFFT4(c,)
	mISTORE4(spy,c,4,.x)
	mLOAD2(&c[0],spx+0*8,20,.x)
	mLOAD2(&c[2],spx+1*8,20,.x)
	mISTORE4(spy,c,4,.y)
	mLOAD2(&c[0],spx+0*8,20,.y)
	mLOAD2(&c[2],spx+1*8,20,.y)
	mSTORE2(d_c+0*8,&c[0],16,)
	mSTORE2(d_c+1*8,&c[2],16,)
}
__global__ void d_ifft16x2x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[40*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD2(&c[0],d_c+0*8,16,)
	mLOAD2(&c[2],d_c+1*8,16,)
	mCALRF4(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mISTORE2(spx+0*8,&c[0],20,.x)
	mISTORE2(spx+1*8,&c[2],20,.x)
	mLOAD4(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],20,.y)
	mISTORE2(spx+1*8,&c[2],20,.y)
	mLOAD4(c,spy,4,.y)
	mFFT4(c,i)
	mHMRF4(c,RF)
	mISTORE4(spy,c,5,.x)
	mLOAD4(c,spz,1,.x)
	mISTORE4(spy,c,5,.y)
	mLOAD4(c,spz,1,.y)
	mFFT4(c,i)
	mISTORE4(spy,c,4,.x)
	mLOAD2(&c[0],spx+0*8,20,.x)
	mLOAD2(&c[2],spx+1*8,20,.x)
	mISTORE4(spy,c,4,.y)
	mLOAD2(&c[0],spx+0*8,20,.y)
	mLOAD2(&c[2],spx+1*8,20,.y)
	mSTORE2(d_c+0*8,&c[0],16,)
	mSTORE2(d_c+1*8,&c[2],16,)
}
__global__ void d_fft16x4x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*80];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<6)+threadIdx.x;
	RF[0]=d_RF[lane];
	mLOAD4(c,d_c,16,)	
	mCALRF4(RF)
	mFFT4(c,)
	mPERMUTE(4,spx,spy,c,20,4,0)
	mFFT4(c,)
	mHMRF4(c,RF)
	mPERMUTE(4,spy,spz,c,5,1,0)
	mFFT4(c,)
	mPERMUTE(4,spy,spx,c,4,20,0)
	mSTORE4(d_c,c,16,)
}
__global__ void d_ifft16x4x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*80];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<6)+threadIdx.x;
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD4(c,d_c,16,)	
	mCALRF4(RF)
	mFFT4(c,i)
	mPERMUTE(4,spx,spy,c,20,4,0)
	mFFT4(c,i)
	mHMRF4(c,RF)
	mPERMUTE(4,spy,spz,c,5,1,0)
	mFFT4(c,i)
	mPERMUTE(4,spy,spx,c,4,20,0)
	mSTORE4(d_c,c,16,)
}
__global__ void d_fft16x8x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*176];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<7)+threadIdx.x;
	RF[0]=d_RF[lane];
	mLOAD8(c,d_c,16,)	
	mCALRF4(RF)
	mFFT8(c,)
	mPERMUTE_S8_L4x2(spx,spy,c,20,80,4,0)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spy,spz,c,80,5,80,1,0)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spy,spx,c,80,4,20,0)
	mSTORE8(d_c,c,16,)
}
__global__ void d_ifft16x8x( float2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[3], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*176];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<7)+threadIdx.x;
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,16,)	
	mCALRF4(RF)
	mFFT8(c,i)
	mPERMUTE_S8_L4x2(spx,spy,c,20,80,4,0)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spy,spz,c,80,5,80,1,0)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spy,spx,c,80,4,20,0)
	mSTORE8(d_c,c,16,)
}
__global__ void d_fft16x16x( float2* d_c, int bat )
{											
	extern __shared__ float smem[];		
	float2 c[16], temp;						
	unsigned int slice_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<8)+threadIdx.x;	
	float* sst=&smem[272*threadIdx.y+threadIdx.x];			
	float* sld=&smem[272*threadIdx.y+17*threadIdx.x];		
	mLOAD16(c,d_c,16,)						
	mFFT16(c,)							
	mPERMUTE(16,sst,sld,c,17,1,0)			
	mFFT16(c,)						
	mPERMUTE(16,sst,sld,c,17,1,0)			
	mSTORE16(d_c,c,16,)						
}
__global__ void d_ifft16x16x( float2* d_c, int bat )
{							
	extern __shared__ float smem[];		
	float2 c[16], temp;						
	unsigned int slice_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<8)+threadIdx.x;	
	float* sst=&smem[272*threadIdx.y+threadIdx.x];			
	float* sld=&smem[272*threadIdx.y+17*threadIdx.x];		
	mLOAD16(c,d_c,16,)				
	mFFT16(c,i);							
	mPERMUTE(16,sst,sld,c,17,1,0)			
	mFFT16(c,i)			
	mPERMUTE(16,sst,sld,c,17,1,0)			
	mSTORE16(d_c,c,16,)						
}
__global__ void d_fft16x32x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[640];
	float2 c[8], RF[7], temp;
	unsigned int tidx, lane, slot, p;
	tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<9)+tidx;
	slot=tidx>>2; 
	lane=tidx&3; 
	p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[80*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];
	RF[0]=d_RF[threadIdx.y];
	mLOAD8(c,d_c,64,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spx,c,80,.x)	
	__syncthreads();
	c[0].x=spy[     0];
	c[1].x=spy[    16];
	c[2].x=spy[    36];
	c[3].x=spy[    52];
	c[4].x=spy[320+ 0];
	c[5].x=spy[320+16];
	c[6].x=spy[320+36];
	c[7].x=spy[320+52];		
	__syncthreads();
	mISTORE8(spx,c,80,.y)	
	__syncthreads();
	c[0].y=spy[     0];
	c[1].y=spy[    16];
	c[2].y=spy[    36];
	c[3].y=spy[    52];
	c[4].y=spy[320+ 0];
	c[5].y=spy[320+16];
	c[6].y=spy[320+36];
	c[7].y=spy[320+52];		
	__syncthreads();	
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE4x2(spx,spu,c,72,144,288,4,0xf)
	RF[0]=d_RF[lane<<1];
	mCALRF4(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,288,4,288,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,288,4,72,0x7)
	mSTORE8(d_c,c,64,)
}
__global__ void d_ifft16x32x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[640];
	float2 c[8], RF[7], temp;
	unsigned int tidx, lane, slot, p;
	tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<9)+tidx;
	slot=tidx>>2; 
	lane=tidx&3; 
	p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[80*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];
	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,64,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spx,c,80,.x)	
	__syncthreads();
	c[0].x=spy[     0];
	c[1].x=spy[    16];
	c[2].x=spy[    36];
	c[3].x=spy[    52];
	c[4].x=spy[320+ 0];
	c[5].x=spy[320+16];
	c[6].x=spy[320+36];
	c[7].x=spy[320+52];		
	__syncthreads();
	mISTORE8(spx,c,80,.y)	
	__syncthreads();
	c[0].y=spy[     0];
	c[1].y=spy[    16];
	c[2].y=spy[    36];
	c[3].y=spy[    52];
	c[4].y=spy[320+ 0];
	c[5].y=spy[320+16];
	c[6].y=spy[320+36];
	c[7].y=spy[320+52];		
	__syncthreads();	
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE4x2(spx,spu,c,72,144,288,4,0xf)
	RF[0]=d_RF[lane<<1];
	RF[0].y=-RF[0].y;
	mCALRF4(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,288,4,288,1,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spu,spx,c,288,4,72,0x7)
	mSTORE8(d_c,c,64,)
}
__global__ void d_fft16x64x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[1280];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<4)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	unsigned int p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];

	RF[0]=d_RF[threadIdx.y];
	mLOAD8(c,d_c,128,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spx,c,144,.x) 
	__syncthreads();
	c[0].x=spy[    0];
	c[1].x=spy[   16];
	c[2].x=spy[   36];
	c[3].x=spy[   52];
	c[4].x=spy[72+ 0];
	c[5].x=spy[72+16];
	c[6].x=spy[72+36];
	c[7].x=spy[72+52];
	__syncthreads();
	mISTORE8(spx,c,144,.y) 
	__syncthreads();
	c[0].y=spy[    0];
	c[1].y=spy[   16];
	c[2].y=spy[   36];
	c[3].y=spy[   52];
	c[4].y=spy[72+ 0];
	c[5].y=spy[72+16];
	c[6].y=spy[72+36];
	c[7].y=spy[72+52];
	__syncthreads();
	mFFT8(c,)
	RF[0]=d_RF[lane<<2];
	mCALRF4(RF)
	mPERMUTE_S8_L4x2(spx,spu,c,144,576,4,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,576,4,576,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,576,4,144,0x7)
	mSTORE8(d_c,c,128,)
}
__global__ void d_ifft16x64x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[1280];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<4)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	unsigned int p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];

	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,128,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	RF[0]=d_RF[lane<<2];
	RF[0].y=-RF[0].y;
	mISTORE8(spx,c,144,.x) 
	__syncthreads();
	c[0].x=spy[    0];
	c[1].x=spy[   16];
	c[2].x=spy[   36];
	c[3].x=spy[   52];
	c[4].x=spy[72+ 0];
	c[5].x=spy[72+16];
	c[6].x=spy[72+36];
	c[7].x=spy[72+52];
	__syncthreads();
	mISTORE8(spx,c,144,.y) 
	__syncthreads();
	c[0].y=spy[    0];
	c[1].y=spy[   16];
	c[2].y=spy[   36];
	c[3].y=spy[   52];
	c[4].y=spy[72+ 0];
	c[5].y=spy[72+16];
	c[6].y=spy[72+36];
	c[7].y=spy[72+52];
	__syncthreads();	
	mFFT8(c,i)
	mCALRF4(RF)
	mPERMUTE_S8_L4x2(spx,spu,c,144,576,4,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,576,4,576,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,576,4,144,0x7)
	mSTORE8(d_c,c,128,)
}
__global__ void d_fft16x128x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[2304];
	float2 c[16], RF[15], temp;
	unsigned int tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	float* spx=&smem[ 16*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[272*threadIdx.y+threadIdx.x];
	float* spv=&smem[17*tidx];
	RF[0]=d_RF[threadIdx.y];
	mLOAD16(c,d_c,128,)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(spx,spy,c,144,1152,16,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mPERMUTE_S8x2_L16(spu,spv,c,17,34,1,0xf)
	mFFT16(c,)
	mPERMUTE(16,spv,spu,c,1,17,0x7)
	mLOAD16(c,spu,17,.y)
	mSTORE16(d_c,c,128,)
}
__global__ void d_ifft16x128x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[2304];
	float2 c[16], RF[15], temp;
	unsigned int tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	float* spx=&smem[ 16*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[272*threadIdx.y+threadIdx.x];
	float* spv=&smem[17*tidx];
	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_c,128,)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(spx,spy,c,144,1152,16,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mPERMUTE_S8x2_L16(spu,spv,c,17,34,1,0xf)
	mFFT16(c,i)
	mPERMUTE(16,spv,spu,c,1,17,0x7)
	mLOAD16(c,spu,17,.y)
	mSTORE16(d_c,c,128,)
}

//===========================================================================================================================================

__global__ void d_mfft16x2x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	ushort2 hc[4];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[40*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	RF[0]=d_RF[lane];
	mLOAD2(&hc[0],d_c+0*8,16,)
	mLOAD2(&hc[2],d_c+1*8,16,)
	mH2Sx4(c,hc)
	mCALRF4(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mISTORE2(spx+0*8,&c[0],20,.x)
	mISTORE2(spx+1*8,&c[2],20,.x)
	mLOAD4(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],20,.y)
	mISTORE2(spx+1*8,&c[2],20,.y)
	mLOAD4(c,spy,4,.y)
	mFFT4(c,)
	mHMRF4(c,RF)
	mISTORE4(spy,c,5,.x)
	mLOAD4(c,spz,1,.x)
	mISTORE4(spy,c,5,.y)
	mLOAD4(c,spz,1,.y)
	mFFT4(c,)
	mISTORE4(spy,c,4,.x)
	mLOAD2(&c[0],spx+0*8,20,.x)
	mLOAD2(&c[2],spx+1*8,20,.x)
	mISTORE4(spy,c,4,.y)
	mLOAD2(&c[0],spx+0*8,20,.y)
	mLOAD2(&c[2],spx+1*8,20,.y)
	mS2Hx4(hc,c)
	mSTORE2(d_c+0*8,&hc[0],16,)
	mSTORE2(d_c+1*8,&hc[2],16,)
}
__global__ void d_imfft16x2x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	ushort2 hc[4];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[40*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD2(&hc[0],d_c+0*8,16,)
	mLOAD2(&hc[2],d_c+1*8,16,)
	mH2Sx4(c,hc)
	mCALRF4(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mISTORE2(spx+0*8,&c[0],20,.x)
	mISTORE2(spx+1*8,&c[2],20,.x)
	mLOAD4(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],20,.y)
	mISTORE2(spx+1*8,&c[2],20,.y)
	mLOAD4(c,spy,4,.y)
	mFFT4(c,i)
	mHMRF4(c,RF)
	mISTORE4(spy,c,5,.x)
	mLOAD4(c,spz,1,.x)
	mISTORE4(spy,c,5,.y)
	mLOAD4(c,spz,1,.y)
	mFFT4(c,i)
	mISTORE4(spy,c,4,.x)
	mLOAD2(&c[0],spx+0*8,20,.x)
	mLOAD2(&c[2],spx+1*8,20,.x)
	mISTORE4(spy,c,4,.y)
	mLOAD2(&c[0],spx+0*8,20,.y)
	mLOAD2(&c[2],spx+1*8,20,.y)
	mS2Hx4(hc,c)
	mSTORE2(d_c+0*8,&hc[0],16,)
	mSTORE2(d_c+1*8,&hc[2],16,)
}

__global__ void d_mfft16x4x( ushort2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	ushort2 hc[4];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*80];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<6)+threadIdx.x;
	RF[0]=d_RF[lane];
	mLOAD4(hc,d_c,16,)
	mH2Sx4(c,hc)
	mCALRF4(RF)
	mFFT4(c,)
	mPERMUTE(4,spx,spy,c,20,4,0)
	mFFT4(c,)
	mHMRF4(c,RF)
	mPERMUTE(4,spy,spz,c,5,1,0)
	mFFT4(c,)
	mPERMUTE(4,spy,spx,c,4,20,0)
	mS2Hx4(hc,c)
	mSTORE4(d_c,hc,16,)
}
__global__ void d_imfft16x4x( ushort2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[4], RF[3], temp;
	ushort2 hc[4];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*80];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<6)+threadIdx.x;
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD4(hc,d_c,16,)	
	mH2Sx4(c,hc)
	mCALRF4(RF)
	mFFT4(c,i)
	mPERMUTE(4,spx,spy,c,20,4,0)
	mFFT4(c,i)
	mHMRF4(c,RF)
	mPERMUTE(4,spy,spz,c,5,1,0)
	mFFT4(c,i)
	mPERMUTE(4,spy,spx,c,4,20,0)
	mS2Hx4(hc,c)
	mSTORE4(d_c,hc,16,)
}
__global__ void d_mfft16x8x( ushort2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[3], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*176];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<7)+threadIdx.x;
	RF[0]=d_RF[lane];
	mLOAD8(hc,d_c,16,)
	mH2Sx8(c,hc)
	mCALRF4(RF)
	mFFT8(c,)
	mPERMUTE_S8_L4x2(spx,spy,c,20,80,4,0)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spy,spz,c,80,5,80,1,0)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spy,spx,c,80,4,20,0)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,16,)
}
__global__ void d_imfft16x8x( ushort2* d_c, const float2* d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[3], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[threadIdx.y*176];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[20*slot+lane];
	float* spz=&sbase[20*slot+5*lane];
	d_c+=(patch_id<<7)+threadIdx.x;
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,16,)
	mH2Sx8(c,hc)
	mCALRF4(RF)
	mFFT8(c,i)
	mPERMUTE_S8_L4x2(spx,spy,c,20,80,4,0)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spy,spz,c,80,5,80,1,0)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spy,spx,c,80,4,20,0)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,16,)
}
__global__ void d_mfft16x16x( ushort2* d_c, int bat )
{											
	extern __shared__ float smem[];		
	float2 c[16], temp;	
	ushort2 hc[16];
	unsigned int slice_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<8)+threadIdx.x;	
	float* sst=&smem[272*threadIdx.y+threadIdx.x];			
	float* sld=&smem[272*threadIdx.y+17*threadIdx.x];		
	mLOAD16(hc,d_c,16,)
	mH2Sx16(c,hc)
	mFFT16(c,)							
	mPERMUTE(16,sst,sld,c,17,1,0)			
	mFFT16(c,)						
	mPERMUTE(16,sst,sld,c,17,1,0)
	mS2Hx16(hc,c)
	mSTORE16(d_c,hc,16,)						
}
__global__ void d_imfft16x16x( ushort2* d_c, int bat )
{							
	extern __shared__ float smem[];		
	float2 c[16], temp;		
	ushort2 hc[16];
	unsigned int slice_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<8)+threadIdx.x;	
	float* sst=&smem[272*threadIdx.y+threadIdx.x];			
	float* sld=&smem[272*threadIdx.y+17*threadIdx.x];		
	mLOAD16(hc,d_c,16,)
	mH2Sx16(c,hc)
	mFFT16(c,i);							
	mPERMUTE(16,sst,sld,c,17,1,0)			
	mFFT16(c,i)			
	mPERMUTE(16,sst,sld,c,17,1,0)
	mS2Hx16(hc,c)
	mSTORE16(d_c,hc,16,)						
}
__global__ void d_mfft16x32x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[640];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int tidx, lane, slot, p;
	tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<9)+tidx;
	slot=tidx>>2; lane=tidx&3; p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[80*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];
	RF[0]=d_RF[threadIdx.y];
	mLOAD8(hc,d_c,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spx,c,80,.x)	
	__syncthreads();
	c[0].x=spy[     0];
	c[1].x=spy[    16];
	c[2].x=spy[    36];
	c[3].x=spy[    52];
	c[4].x=spy[320+ 0];
	c[5].x=spy[320+16];
	c[6].x=spy[320+36];
	c[7].x=spy[320+52];		
	__syncthreads();
	mISTORE8(spx,c,80,.y)	
	__syncthreads();
	c[0].y=spy[     0];
	c[1].y=spy[    16];
	c[2].y=spy[    36];
	c[3].y=spy[    52];
	c[4].y=spy[320+ 0];
	c[5].y=spy[320+16];
	c[6].y=spy[320+36];
	c[7].y=spy[320+52];		
	__syncthreads();	
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE4x2(spx,spu,c,72,144,288,4,0xf)
	RF[0]=d_RF[lane<<1];
	mCALRF4(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,288,4,288,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,288,4,72,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,64,)
}
__global__ void d_imfft16x32x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[640];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int tidx, lane, slot, p;
	tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<9)+tidx;
	slot=tidx>>2; lane=tidx&3; p=36*(slot>>1)+16*(slot&1);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[80*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];
	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spx,c,80,.x)	
	__syncthreads();
	c[0].x=spy[     0];
	c[1].x=spy[    16];
	c[2].x=spy[    36];
	c[3].x=spy[    52];
	c[4].x=spy[320+ 0];
	c[5].x=spy[320+16];
	c[6].x=spy[320+36];
	c[7].x=spy[320+52];		
	__syncthreads();
	mISTORE8(spx,c,80,.y)	
	__syncthreads();
	c[0].y=spy[     0];
	c[1].y=spy[    16];
	c[2].y=spy[    36];
	c[3].y=spy[    52];
	c[4].y=spy[320+ 0];
	c[5].y=spy[320+16];
	c[6].y=spy[320+36];
	c[7].y=spy[320+52];		
	__syncthreads();	
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE4x2(spx,spu,c,72,144,288,4,0xf)
	RF[0]=d_RF[lane<<1];
	RF[0].y=-RF[0].y;
	mCALRF4(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,288,4,288,1,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spu,spx,c,288,4,72,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,64,)
}
__global__ void d_mfft16x64x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[1280];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<4)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	unsigned int p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];
	RF[0]=d_RF[threadIdx.y];
	mLOAD8(hc,d_c,128,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spx,c,144,.x) 
	__syncthreads();
	c[0].x=spy[    0];
	c[1].x=spy[   16];
	c[2].x=spy[   36];
	c[3].x=spy[   52];
	c[4].x=spy[72+ 0];
	c[5].x=spy[72+16];
	c[6].x=spy[72+36];
	c[7].x=spy[72+52];
	__syncthreads();
	mISTORE8(spx,c,144,.y) 
	__syncthreads();
	c[0].y=spy[    0];
	c[1].y=spy[   16];
	c[2].y=spy[   36];
	c[3].y=spy[   52];
	c[4].y=spy[72+ 0];
	c[5].y=spy[72+16];
	c[6].y=spy[72+36];
	c[7].y=spy[72+52];
	__syncthreads();
	mFFT8(c,)
	RF[0]=d_RF[lane<<2];
	mCALRF4(RF)
	mPERMUTE_S8_L4x2(spx,spu,c,144,576,4,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,576,4,576,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,576,4,144,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,128,)
}
__global__ void d_imfft16x64x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[1280];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<4)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	unsigned int p=36*(slot>>1)+((slot&1)<<4);
	float* spx=&smem[36*(slot>>3)+4*(slot&7)+lane];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[p+lane];
	float* spv=&smem[p+4*lane];

	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,128,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	RF[0]=d_RF[lane<<2];
	RF[0].y=-RF[0].y;
	mISTORE8(spx,c,144,.x) 
	__syncthreads();
	c[0].x=spy[    0];
	c[1].x=spy[   16];
	c[2].x=spy[   36];
	c[3].x=spy[   52];
	c[4].x=spy[72+ 0];
	c[5].x=spy[72+16];
	c[6].x=spy[72+36];
	c[7].x=spy[72+52];
	__syncthreads();
	mISTORE8(spx,c,144,.y) 
	__syncthreads();
	c[0].y=spy[    0];
	c[1].y=spy[   16];
	c[2].y=spy[   36];
	c[3].y=spy[   52];
	c[4].y=spy[72+ 0];
	c[5].y=spy[72+16];
	c[6].y=spy[72+36];
	c[7].y=spy[72+52];
	__syncthreads();	
	mFFT8(c,i)
	mCALRF4(RF)
	mPERMUTE_S8_L4x2(spx,spu,c,144,576,4,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spu,spv,c,576,4,576,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,576,4,144,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,128,)
}
__global__ void d_mfft16x128x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[2304];
	float2 c[16], RF[15], temp;
	ushort2 hc[16];
	unsigned int tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	float* spx=&smem[ 16*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[272*threadIdx.y+threadIdx.x];
	float* spv=&smem[17*tidx];
	RF[0]=d_RF[threadIdx.y];
	mLOAD16(hc,d_c,128,)
	mH2Sx16(c,hc)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(spx,spy,c,144,1152,16,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mPERMUTE_S8x2_L16(spu,spv,c,17,34,1,0xf)
	mFFT16(c,)
	mPERMUTE(16,spv,spu,c,1,17,0x7)
	mLOAD16(c,spu,17,.y)
	mS2Hx16(hc,c)
	mSTORE16(d_c,hc,128,)
}
__global__ void d_imfft16x128x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[2304];
	float2 c[16], RF[15], temp;
	ushort2 hc[16];
	unsigned int tidx=(threadIdx.y<<4)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	float* spx=&smem[ 16*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[272*threadIdx.y+threadIdx.x];
	float* spv=&smem[17*tidx];
	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD16(hc,d_c,128,)
	mH2Sx16(c,hc)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	mPERMUTE_S16_L8x2(spx,spy,c,144,1152,16,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mPERMUTE_S8x2_L16(spu,spv,c,17,34,1,0xf)
	mFFT16(c,i)
	mPERMUTE(16,spv,spu,c,1,17,0x7)
	mLOAD16(c,spu,17,.y)
	mS2Hx16(hc,c)
	mSTORE16(d_c,hc,128,)
}