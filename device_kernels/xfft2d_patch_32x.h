#include"hfft.h"
#include"cuintrin.h"

/*
32x2   : block( 8, n)
32x4   : block(16, n)
32x8   : block(32, n)
32x16  : block(16, 4)
32x32  : block(32, 4)
32x64  : block(32, 8)
32x128 : block(32, 8)
*/

__global__ void d_fft32x2x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[72*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	mLOAD2(&c[0],d_c+0*8,32,)
	mLOAD2(&c[2],d_c+1*8,32,)
	mLOAD2(&c[4],d_c+2*8,32,)
	mLOAD2(&c[6],d_c+3*8,32,)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*8,&c[0],36,.x)
	mISTORE2(spx+1*8,&c[2],36,.x)
	mISTORE2(spx+2*8,&c[4],36,.x)
	mISTORE2(spx+3*8,&c[6],36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],36,.y)
	mISTORE2(spx+1*8,&c[2],36,.y)
	mISTORE2(spx+2*8,&c[4],36,.y)
	mISTORE2(spx+3*8,&c[6],36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD2(&c[0],spx+0*8,36,.x)
	mLOAD2(&c[2],spx+1*8,36,.x)
	mLOAD2(&c[4],spx+2*8,36,.x)
	mLOAD2(&c[6],spx+3*8,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD2(&c[0],spx+0*8,36,.y)
	mLOAD2(&c[2],spx+1*8,36,.y)
	mLOAD2(&c[4],spx+2*8,36,.y)
	mLOAD2(&c[6],spx+3*8,36,.y)
	mSTORE2(d_c+0*8,&c[0],32,)
	mSTORE2(d_c+1*8,&c[2],32,)
	mSTORE2(d_c+2*8,&c[4],32,)
	mSTORE2(d_c+3*8,&c[6],32,)
}
__global__ void d_ifft32x2x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[72*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD2(&c[0],d_c+0*8,32,)
	mLOAD2(&c[2],d_c+1*8,32,)
	mLOAD2(&c[4],d_c+2*8,32,)
	mLOAD2(&c[6],d_c+3*8,32,)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*8,&c[0],36,.x)
	mISTORE2(spx+1*8,&c[2],36,.x)
	mISTORE2(spx+2*8,&c[4],36,.x)
	mISTORE2(spx+3*8,&c[6],36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],36,.y)
	mISTORE2(spx+1*8,&c[2],36,.y)
	mISTORE2(spx+2*8,&c[4],36,.y)
	mISTORE2(spx+3*8,&c[6],36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD2(&c[0],spx+0*8,36,.x)
	mLOAD2(&c[2],spx+1*8,36,.x)
	mLOAD2(&c[4],spx+2*8,36,.x)
	mLOAD2(&c[6],spx+3*8,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD2(&c[0],spx+0*8,36,.y)
	mLOAD2(&c[2],spx+1*8,36,.y)
	mLOAD2(&c[4],spx+2*8,36,.y)
	mLOAD2(&c[6],spx+3*8,36,.y)
	mSTORE2(d_c+0*8,&c[0],32,)
	mSTORE2(d_c+1*8,&c[2],32,)
	mSTORE2(d_c+2*8,&c[4],32,)
	mSTORE2(d_c+3*8,&c[6],32,)
}
__global__ void d_fft32x4x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[144*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	mLOAD4x2(c,d_c,16,32,)
	mCALRF8(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spx,c,16,36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE4x2(spx,c,16,36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD4x2(c,spx,16,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD4x2(c,spx,16,36,.y)
	mSTORE4x2(d_c,c,16,32)
}
__global__ void d_ifft32x4x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[144*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD4x2(c,d_c,16,32,)
	mCALRF8(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spx,c,16,36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE4x2(spx,c,16,36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD4x2(c,spx,16,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD4x2(c,spx,16,36,.y)
	mSTORE4x2(d_c,c,16,32)
}
__global__ void d_fft32x8x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* base=&smem[288*threadIdx.y];
	float* spx=&base[threadIdx.x];
	float* spy=&base[36*slot+lane];
	float* spz=&base[33*lane+4*slot];
	RF[0]=d_RF[lane];
	mLOAD8(c,d_c,32,)
	mCALRF8(RF)
	mFFT8(c,)
	mPERMUTE(8,spx,spy,c,36,4,0)	
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spz,c,33,132,1,0)	
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spy,spx,c,4,8,36,0)		
	mSTORE8(d_c,c,32,)
}
__global__ void d_ifft32x8x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* base=&smem[288*threadIdx.y];
	float* spx=&base[threadIdx.x];
	float* spy=&base[36*slot+lane];
	float* spz=&base[33*lane+4*slot];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,32,)
	mCALRF8(RF)
	mFFT8(c,i)
	mPERMUTE(8,spx,spy,c,36,4,0)		
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spz,c,33,132,1,0)	
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spy,spx,c,4,8,36,0)					
	mSTORE8(d_c,c,32,)
}
/*bank conflicts*/
__global__ void d_fft32x16x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*48];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<9)+(threadIdx.y<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[36*slot+4*lane];
	RF[0]=d_RF[threadIdx.y<<1];
	mLOAD4(&c[0],d_c   ,128,)
	mLOAD4(&c[4],d_c+16,128,)
	mCALRF4(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.x)	__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.y)	__syncthreads();
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	RF[0]=d_RF[lane];
	mCALRF8(RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD8(c,spu,4,.x)			__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD8(c,spu,4,.y)			__syncthreads();
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spu,c,4,.x)		__syncthreads();
	mLOAD4x2(c,spv,16,1,.x)		__syncthreads();
	mISTORE8(spu,c,4,.y)		__syncthreads();
	mLOAD4x2(c,spv,16,1,.y)		__syncthreads();
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spu,c,4,8,.x)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.x)	__syncthreads();
	mISTORE4x2(spu,c,4,8,.y)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.y)
	mSTORE4(d_c   ,&c[0],128,)
	mSTORE4(d_c+16,&c[4],128,)
}
__global__ void d_ifft32x16x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*48];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<9)+(threadIdx.y<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[36*slot+4*lane];
	RF[0]=d_RF[threadIdx.y<<1];
	RF[0].y=-RF[0].y;
	mLOAD4(&c[0],d_c   ,128,)
	mLOAD4(&c[4],d_c+16,128,)
	mCALRF4(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.x)	__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.y)	__syncthreads();
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mCALRF8(RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD8(c,spu,4,.x)			__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD8(c,spu,4,.y)			__syncthreads();
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spu,c,4,.x)	__syncthreads();
	mLOAD4x2(c,spv,16,1,.x)	__syncthreads();
	mISTORE8(spu,c,4,.y)	__syncthreads();
	mLOAD4x2(c,spv,16,1,.y)	__syncthreads();
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spu,c,4,8,.x)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.x)	__syncthreads();
	mISTORE4x2(spu,c,4,8,.y)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.y)
	mSTORE4(d_c   ,&c[0],128,)
	mSTORE4(d_c+16,&c[4],128,)
}
__global__ void d_fft32x32x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[32][33];
	float2 c[8], RF[7], temp;
	d_c+=((blockIdx.x<<10)+(threadIdx.y<<5)+threadIdx.x);
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[4*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][4*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD8(c,d_c,128,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,132,528,33,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spx,spu,c,132,264,4,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spu,spv,c,4,16,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,4,8,132,0x7)					
	mSTORE8(d_c,c,128,)
}
__global__ void d_ifft32x32x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[32][33];
	float2 c[8], RF[7], temp;
	d_c+=((blockIdx.x<<10)+(threadIdx.y<<5)+threadIdx.x);
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[4*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][4*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,128,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,132,528,33,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spx,spu,c,132,264,4,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spu,spv,c,4,16,1,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spu,spx,c,4,8,132,0x7)	
	mSTORE8(d_c,c,128,)
}
__global__ void d_fft32x64x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[64*36];
	float2 c[8], RF[7], temp;	
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD8(c,d_c,256,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)	
	RF[0]=d_RF[lane<<1];
	mPERMUTE(8,spx,spy,c,288,36,0xf)
	mFFT8(c,)	
	mCALRF8(RF)
	mPERMUTE(8,spy,spu,c,36,4,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spy,spv,c,33,132,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spy,spu,c,36,72,4,0x7)
	mSTORE8(d_c,c,256,)
}
__global__ void d_ifft32x64x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[64*36];
	float2 c[8], RF[7], temp;	
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,256,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)	
	RF[0]=d_RF[lane<<1];
	RF[0].y=-RF[0].y;
	mPERMUTE(8,spx,spy,c,288,36,0xf)
	mFFT8(c,i)	
	mCALRF8(RF)
	mPERMUTE(8,spy,spu,c,36,4,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spy,spv,c,33,132,1,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spy,spu,c,36,72,4,0x7)
	mSTORE8(d_c,c,256,)
}
__global__ void d_fft32x128x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[288*16];
	float2 c[16], RF[15], temp;	
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<12)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD16(c,d_c,256,)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)	
	RF[0]=d_RF[lane<<2];
	mPERMUTE_S16_L8x2(spx,spy,c,288,2304,36,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mCALRF8(RF)
	mPERMUTE8x2(spy,spu,c,2304,36,2304,4,0xf)

	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mISTORE8x2(spy,c,2304,33,.x)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.x)
	mLOAD4x2(&c[8],spv+2304,132,1,.x)	__syncthreads();
	mISTORE8x2(spy,c,2304,33,.y)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.y)
	mLOAD4x2(&c[8],spv+2304,132,1,.y)	__syncthreads();
	mFFT4(&c[ 0],)
	mFFT4(&c[ 4],)	
	mFFT4(&c[ 8],)
	mFFT4(&c[12],)
	mISTORE4x2(spy	   ,&c[0],36,72,.x)
	mISTORE4x2(spy+2304,&c[8],36,72,.x) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.x)			__syncthreads();
	mISTORE4x2(spy	   ,&c[0],36,72,.y)
	mISTORE4x2(spy+2304,&c[8],36,72,.y) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.y)
	mSTORE8x2(d_c,c,256,512,)
}
__global__ void d_ifft32x128x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[288*16];
	float2 c[16], RF[15], temp;	
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<12)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_c,256,)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)	
	RF[0]=d_RF[lane<<2];
	RF[0].y=-RF[0].y;
	mPERMUTE_S16_L8x2(spx,spy,c,288,2304,36,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mCALRF8(RF)
	mPERMUTE8x2(spy,spu,c,2304,36,2304,4,0xf)

	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mISTORE8x2(spy,c,2304,33,.x)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.x)
	mLOAD4x2(&c[8],spv+2304,132,1,.x)	__syncthreads();
	mISTORE8x2(spy,c,2304,33,.y)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.y)
	mLOAD4x2(&c[8],spv+2304,132,1,.y)	__syncthreads();
	mFFT4(&c[ 0],i)
	mFFT4(&c[ 4],i)	
	mFFT4(&c[ 8],i)
	mFFT4(&c[12],i)
	mISTORE4x2(spy	   ,&c[0],36,72,.x)
	mISTORE4x2(spy+2304,&c[8],36,72,.x) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.x)			__syncthreads();
	mISTORE4x2(spy	   ,&c[0],36,72,.y)
	mISTORE4x2(spy+2304,&c[8],36,72,.y) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.y)
	mSTORE8x2(d_c,c,256,512,)
}

//======================================================================================================================================================

__global__ void d_mfft32x2x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[72*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	mLOAD2(&hc[0],d_c+0*8,32,)
	mLOAD2(&hc[2],d_c+1*8,32,)
	mLOAD2(&hc[4],d_c+2*8,32,)
	mLOAD2(&hc[6],d_c+3*8,32,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*8,&c[0],36,.x)
	mISTORE2(spx+1*8,&c[2],36,.x)
	mISTORE2(spx+2*8,&c[4],36,.x)
	mISTORE2(spx+3*8,&c[6],36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],36,.y)
	mISTORE2(spx+1*8,&c[2],36,.y)
	mISTORE2(spx+2*8,&c[4],36,.y)
	mISTORE2(spx+3*8,&c[6],36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD2(&c[0],spx+0*8,36,.x)
	mLOAD2(&c[2],spx+1*8,36,.x)
	mLOAD2(&c[4],spx+2*8,36,.x)
	mLOAD2(&c[6],spx+3*8,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD2(&c[0],spx+0*8,36,.y)
	mLOAD2(&c[2],spx+1*8,36,.y)
	mLOAD2(&c[4],spx+2*8,36,.y)
	mLOAD2(&c[6],spx+3*8,36,.y)
	mS2Hx8(hc,c)
	mSTORE2(d_c+0*8,&hc[0],32,)
	mSTORE2(d_c+1*8,&hc[2],32,)
	mSTORE2(d_c+2*8,&hc[4],32,)
	mSTORE2(d_c+3*8,&hc[6],32,)
}
__global__ void d_imfft32x2x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[72*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD2(&hc[0],d_c+0*8,32,)
	mLOAD2(&hc[2],d_c+1*8,32,)
	mLOAD2(&hc[4],d_c+2*8,32,)
	mLOAD2(&hc[6],d_c+3*8,32,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*8,&c[0],36,.x)
	mISTORE2(spx+1*8,&c[2],36,.x)
	mISTORE2(spx+2*8,&c[4],36,.x)
	mISTORE2(spx+3*8,&c[6],36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE2(spx+0*8,&c[0],36,.y)
	mISTORE2(spx+1*8,&c[2],36,.y)
	mISTORE2(spx+2*8,&c[4],36,.y)
	mISTORE2(spx+3*8,&c[6],36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD2(&c[0],spx+0*8,36,.x)
	mLOAD2(&c[2],spx+1*8,36,.x)
	mLOAD2(&c[4],spx+2*8,36,.x)
	mLOAD2(&c[6],spx+3*8,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD2(&c[0],spx+0*8,36,.y)
	mLOAD2(&c[2],spx+1*8,36,.y)
	mLOAD2(&c[4],spx+2*8,36,.y)
	mLOAD2(&c[6],spx+3*8,36,.y)
	mS2Hx8(hc,c)
	mSTORE2(d_c+0*8,&hc[0],32,)
	mSTORE2(d_c+1*8,&hc[2],32,)
	mSTORE2(d_c+2*8,&hc[4],32,)
	mSTORE2(d_c+3*8,&hc[6],32,)
}
__global__ void d_mfft32x4x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[144*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	mLOAD4x2(hc,d_c,16,32,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spx,c,16,36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE4x2(spx,c,16,36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD4x2(c,spx,16,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD4x2(c,spx,16,36,.y)
	mS2Hx8(hc,c)
	mSTORE4x2(d_c,hc,16,32)
}
__global__ void d_imfft32x4x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	short2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* sbase=&smem[144*threadIdx.y];
	float* spx=&sbase[threadIdx.x];
	float* spy=&sbase[36*slot+lane];
	float* spz=&sbase[36*slot+4*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD4x2(hc,d_c,16,32,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spx,c,16,36,.x)
	mLOAD8(c,spy,4,.x)
	mISTORE4x2(spx,c,16,36,.y)
	mLOAD8(c,spy,4,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spy,c,4,.x)
	mLOAD4x2(c,spz,16,1,.x)
	mISTORE8(spy,c,4,.y)
	mLOAD4x2(c,spz,16,1,.y)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spy,c,4,8,.x)
	mLOAD4x2(c,spx,16,36,.x)
	mISTORE4x2(spy,c,4,8,.y)
	mLOAD4x2(c,spx,16,36,.y)
	mS2Hx8(hc,c)
	mSTORE4x2(d_c,hc,16,32)
}
__global__ void d_mfft32x8x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* base=&smem[288*threadIdx.y];
	float* spx=&base[threadIdx.x];
	float* spy=&base[36*slot+lane];
	float* spz=&base[33*lane+4*slot];
	RF[0]=d_RF[lane];
	mLOAD8(hc,d_c,32,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mPERMUTE(8,spx,spy,c,36,4,0)	
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spz,c,33,132,1,0)	
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spy,spx,c,4,8,36,0)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,32,)
}
__global__ void d_imfft32x8x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=threadIdx.x>>2;
	float* base=&smem[288*threadIdx.y];
	float* spx=&base[threadIdx.x];
	float* spy=&base[36*slot+lane];
	float* spz=&base[33*lane+4*slot];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,32,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mPERMUTE(8,spx,spy,c,36,4,0)		
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spz,c,33,132,1,0)	
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spy,spx,c,4,8,36,0)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,32,)
}
__global__ void d_mfft32x16x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*48];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<9)+(threadIdx.y<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[36*slot+4*lane];
	RF[0]=d_RF[threadIdx.y<<1];
	mLOAD4(&hc[0],d_c   ,128,)
	mLOAD4(&hc[4],d_c+16,128,)
	mH2Sx8(c,hc)
	mCALRF4(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.x)	__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.y)	__syncthreads();
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	RF[0]=d_RF[lane];
	mCALRF8(RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD8(c,spu,4,.x)			__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD8(c,spu,4,.y)			__syncthreads();
	mFFT8(c,)
	mHMRF8(c,RF)
	mISTORE8(spu,c,4,.x)		__syncthreads();
	mLOAD4x2(c,spv,16,1,.x)		__syncthreads();
	mISTORE8(spu,c,4,.y)		__syncthreads();
	mLOAD4x2(c,spv,16,1,.y)		__syncthreads();
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4x2(spu,c,4,8,.x)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.x)	__syncthreads();
	mISTORE4x2(spu,c,4,8,.y)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.y)
	mS2Hx8(hc,c)
	mSTORE4(d_c   ,&hc[0],128,)
	mSTORE4(d_c+16,&hc[4],128,)
}
__global__ void d_imfft32x16x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*48];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<9)+(threadIdx.y<<5)+threadIdx.x;
	unsigned int lane=threadIdx.x&3;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>2);
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[144*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[36*slot+4*lane];
	RF[0]=d_RF[threadIdx.y<<1];
	RF[0].y=-RF[0].y;
	mLOAD4(&hc[0],d_c   ,128,)
	mLOAD4(&hc[4],d_c+16,128,)
	mH2Sx8(c,hc)
	mCALRF4(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.x)	__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD4x2(c,spy,16,36,.y)	__syncthreads();
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mCALRF8(RF)
	mISTORE4x2(spx,c,16,144,.x)	__syncthreads();
	mLOAD8(c,spu,4,.x)			__syncthreads();
	mISTORE4x2(spx,c,16,144,.y)	__syncthreads();
	mLOAD8(c,spu,4,.y)			__syncthreads();
	mFFT8(c,i)
	mHMRF8(c,RF)
	mISTORE8(spu,c,4,.x)	__syncthreads();
	mLOAD4x2(c,spv,16,1,.x)	__syncthreads();
	mISTORE8(spu,c,4,.y)	__syncthreads();
	mLOAD4x2(c,spv,16,1,.y)	__syncthreads();
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4x2(spu,c,4,8,.x)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.x)	__syncthreads();
	mISTORE4x2(spu,c,4,8,.y)	__syncthreads();
	mLOAD4x2(c,spx,16,144,.y)
	mS2Hx8(hc,c)
	mSTORE4(d_c   ,&hc[0],128,)
	mSTORE4(d_c+16,&hc[4],128,)
}
__global__ void d_mfft32x32x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[32][33];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=((blockIdx.x<<10)+(threadIdx.y<<5)+threadIdx.x);
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[4*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][4*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD8(hc,d_c,128,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,132,528,33,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spx,spu,c,132,264,4,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spu,spv,c,4,16,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spu,spx,c,4,8,132,0x7)	
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,128,)
}
__global__ void d_imfft32x32x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[32][33];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=((blockIdx.x<<10)+(threadIdx.y<<5)+threadIdx.x);
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[4*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][4*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,128,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,132,528,33,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spx,spu,c,132,264,4,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spu,spv,c,4,16,1,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spu,spx,c,4,8,132,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,128,)
}
__global__ void d_mfft32x64x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[64*36];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD8(hc,d_c,256,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)	
	RF[0]=d_RF[lane<<1];
	mPERMUTE(8,spx,spy,c,288,36,0xf)
	mFFT8(c,)	
	mCALRF8(RF)
	mPERMUTE(8,spy,spu,c,36,4,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spy,spv,c,33,132,1,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spy,spu,c,36,72,4,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,256,)
}
__global__ void d_imfft32x64x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[64*36];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<11)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,256,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)	
	RF[0]=d_RF[lane<<1];
	RF[0].y=-RF[0].y;
	mPERMUTE(8,spx,spy,c,288,36,0xf)
	mFFT8(c,i)	
	mCALRF8(RF)
	mPERMUTE(8,spy,spu,c,36,4,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spy,spv,c,33,132,1,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spy,spu,c,36,72,4,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,256,)
}
__global__ void d_mfft32x128x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[288*16];
	float2 c[16], RF[15], temp;	
	ushort2 hc[16];
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<12)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD16(hc,d_c,256,)
	mH2Sx16(c,hc)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)	
	RF[0]=d_RF[lane<<2];
	mPERMUTE_S16_L8x2(spx,spy,c,288,2304,36,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mCALRF8(RF)
	mPERMUTE8x2(spy,spu,c,2304,36,2304,4,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mISTORE8x2(spy,c,2304,33,.x)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.x)
	mLOAD4x2(&c[8],spv+2304,132,1,.x)	__syncthreads();
	mISTORE8x2(spy,c,2304,33,.y)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.y)
	mLOAD4x2(&c[8],spv+2304,132,1,.y)	__syncthreads();
	mFFT4(&c[ 0],)
	mFFT4(&c[ 4],)	
	mFFT4(&c[ 8],)
	mFFT4(&c[12],)
	mISTORE4x2(spy	   ,&c[0],36,72,.x)
	mISTORE4x2(spy+2304,&c[8],36,72,.x) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.x)			__syncthreads();
	mISTORE4x2(spy	   ,&c[0],36,72,.y)
	mISTORE4x2(spy+2304,&c[8],36,72,.y) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.y)
	mS2Hx16(hc,c)
	mSTORE8x2(d_c,hc,256,512,)
}
__global__ void d_imfft32x128x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[288*16];
	float2 c[16], RF[15], temp;	
	ushort2 hc[16];
	unsigned int tidx=(threadIdx.y<<5)+threadIdx.x;
	d_c+=(blockIdx.x<<12)+tidx;
	unsigned int lane=tidx&3;
	unsigned int slot=tidx>>2;
	float* spx=&smem[ 36*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[36*slot+lane];
	float* spv=&smem[288*threadIdx.y+33*lane+4*(slot&7)];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD16(hc,d_c,256,)
	mH2Sx16(c,hc)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)	
	RF[0]=d_RF[lane<<2];
	RF[0].y=-RF[0].y;
	mPERMUTE_S16_L8x2(spx,spy,c,288,2304,36,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mCALRF8(RF)
	mPERMUTE8x2(spy,spu,c,2304,36,2304,4,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mISTORE8x2(spy,c,2304,33,.x)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.x)
	mLOAD4x2(&c[8],spv+2304,132,1,.x)	__syncthreads();
	mISTORE8x2(spy,c,2304,33,.y)		__syncthreads();
	mLOAD4x2(&c[0],spv	   ,132,1,.y)
	mLOAD4x2(&c[8],spv+2304,132,1,.y)	__syncthreads();
	mFFT4(&c[ 0],i)
	mFFT4(&c[ 4],i)	
	mFFT4(&c[ 8],i)
	mFFT4(&c[12],i)
	mISTORE4x2(spy	   ,&c[0],36,72,.x)
	mISTORE4x2(spy+2304,&c[8],36,72,.x) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.x)			__syncthreads();
	mISTORE4x2(spy	   ,&c[0],36,72,.y)
	mISTORE4x2(spy+2304,&c[8],36,72,.y) __syncthreads();
	mLOAD8x2(c,spu,2304,4,.y)
	mS2Hx16(hc,c)
	mSTORE8x2(d_c,hc,256,512,)
}