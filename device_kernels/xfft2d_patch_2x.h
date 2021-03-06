
#include"hfft.h"
#include"cuintrin.h"

__global__ void d_fft2x2x( float2* d_c, int bat )
{											
	float2 c[4], temp;						
	unsigned int slice_id=blockIdx.x*blockDim.x+threadIdx.x;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<2);	
	mLOAD4(c,d_c,1,)
	BFLYU10(c[0],c[1])
	BFLYU10(c[2],c[3])
	BFLYU10(c[0],c[2])
	BFLYU10(c[1],c[3])
	mISTORE4(d_c,c,1,)
}
__global__ void d_ifft2x2x( float2* d_c, int bat )
{											
	float2 c[4], temp;						
	unsigned int slice_id=blockIdx.x*blockDim.x+threadIdx.x;
	if(slice_id>=bat) return;
	d_c+=(slice_id<<2);	
	mLOAD4(c,d_c,1,)
	BFLYU10(c[0],c[1])
	BFLYU10(c[2],c[3])
	BFLYU10(c[0],c[2])
	BFLYU10(c[1],c[3])
	mSTORE4(d_c,c,1,)
}
__global__ void d_fft2x4x( float2* d_c, int bat )
{													
	float2 c[8], temp;						
	unsigned int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_c+=(tidx<<3)+threadIdx.x;
	mLOAD8(c,d_c,1,)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	BFLYU10(c[0],c[4])
	BFLYU10(c[2],c[6])
	BFLYU10(c[0],c[2])
	BFLYU01(c[4],c[6])
	BFLYU10(c[1],c[5])
	BFLYU10(c[3],c[7])
	BFLYU10(c[1],c[3])
	BFLYU01(c[5],c[7])
	d_c[0]=c[0];
	d_c[1]=c[1];
	d_c[2]=c[4];
	d_c[3]=c[5];
	d_c[4]=c[2];
	d_c[5]=c[3];
	d_c[6]=c[6];
	d_c[7]=c[7];
}
__global__ void d_ifft2x4x( float2* d_c, int bat )
{													
	float2 c[8], temp;						
	unsigned int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_c+=(tidx<<3)+threadIdx.x;
	mLOAD8(c,d_c,1,)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	BFLYU10(c[0],c[4])
	BFLYU10(c[2],c[6])
	BFLYU10(c[0],c[2])
	iBFLYU01(c[4],c[6])
	BFLYU10(c[1],c[5])
	BFLYU10(c[3],c[7])
	BFLYU10(c[1],c[3])
	iBFLYU01(c[5],c[7])
	d_c[0]=c[0];
	d_c[1]=c[1];
	d_c[2]=c[4];
	d_c[3]=c[5];
	d_c[4]=c[2];
	d_c[5]=c[3];
	d_c[6]=c[6];
	d_c[7]=c[7];
}
__global__ void d_fft2x8x( float2* d_c, int bat )
{													
	float2 c[16], temp;						
	unsigned int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_c+=(tidx<<4)+threadIdx.x;
	mLOAD16(c,d_c,1,)
	mFFT2(&c[ 0],)
	mFFT2(&c[ 2],)
	mFFT2(&c[ 4],)
	mFFT2(&c[ 6],)
	mFFT2(&c[ 8],)
	mFFT2(&c[10],)
	mFFT2(&c[12],)
	mFFT2(&c[14],)

	BFLYU10(c[ 0],c[ 8])
	BFLYU10(c[ 2],c[10])
	BFLYU10(c[ 4],c[12])
	BFLYU10(c[ 6],c[14])
	BFLYU10(c[ 0],c[ 4])
	BFLYU10(c[ 2],c[ 6])
	BFLYU01(c[ 8],c[12])
	BFLYU01(c[10],c[14])
	BFLYU10(c[ 0],c[ 2])
	BFLYU01(c[ 4],c[ 6])
	BFLYU(c[ 8],c[10], 0.707106781f,-0.707106781f);	
	BFLYU(c[12],c[14],-0.707106781f,-0.707106781f);

	BFLYU10(c[ 1],c[ 9])
	BFLYU10(c[ 3],c[11])
	BFLYU10(c[ 5],c[13])
	BFLYU10(c[ 7],c[15])
	BFLYU10(c[ 1],c[ 5])
	BFLYU10(c[ 3],c[ 7])
	BFLYU01(c[ 9],c[13])
	BFLYU01(c[11],c[15])
	BFLYU10(c[ 1],c[ 3])
	BFLYU01(c[ 5],c[ 7])
	BFLYU(c[ 9],c[11], 0.707106781f,-0.707106781f);	
	BFLYU(c[13],c[15],-0.707106781f,-0.707106781f);	

	d_c[ 0]=c[ 0];
	d_c[ 1]=c[ 1];
	d_c[ 2]=c[ 8];
	d_c[ 3]=c[ 9];
	d_c[ 4]=c[ 4];
	d_c[ 5]=c[ 5];
	d_c[ 6]=c[12];
	d_c[ 7]=c[13];
	d_c[ 8]=c[ 2];
	d_c[ 9]=c[ 3];
	d_c[10]=c[10];
	d_c[11]=c[11];
	d_c[12]=c[ 6];
	d_c[13]=c[ 7];
	d_c[14]=c[14];
	d_c[15]=c[15];
}
__global__ void d_ifft2x8x( float2* d_c, int bat )
{													
	float2 c[16], temp;						
	unsigned int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_c+=(tidx<<4)+threadIdx.x;
	mLOAD16(c,d_c,1,)
	mFFT2(&c[ 0],)
	mFFT2(&c[ 2],)
	mFFT2(&c[ 4],)
	mFFT2(&c[ 6],)
	mFFT2(&c[ 8],)
	mFFT2(&c[10],)
	mFFT2(&c[12],)
	mFFT2(&c[14],)

	BFLYU10(c[ 0],c[ 8])
	BFLYU10(c[ 2],c[10])
	BFLYU10(c[ 4],c[12])
	BFLYU10(c[ 6],c[14])
	BFLYU10(c[ 0],c[ 4])
	BFLYU10(c[ 2],c[ 6])
	iBFLYU01(c[ 8],c[12])
	iBFLYU01(c[10],c[14])
	BFLYU10(c[ 0],c[ 2])
	iBFLYU01(c[ 4],c[ 6])
	iBFLYU(c[ 8],c[10], 0.707106781f,-0.707106781f);	
	iBFLYU(c[12],c[14],-0.707106781f,-0.707106781f);

	BFLYU10(c[ 1],c[ 9])
	BFLYU10(c[ 3],c[11])
	BFLYU10(c[ 5],c[13])
	BFLYU10(c[ 7],c[15])
	BFLYU10(c[ 1],c[ 5])
	BFLYU10(c[ 3],c[ 7])
	iBFLYU01(c[ 9],c[13])
	iBFLYU01(c[11],c[15])
	BFLYU10(c[ 1],c[ 3])
	iBFLYU01(c[ 5],c[ 7])
	iBFLYU(c[ 9],c[11], 0.707106781f,-0.707106781f);	
	iBFLYU(c[13],c[15],-0.707106781f,-0.707106781f);	

	d_c[ 0]=c[ 0];
	d_c[ 1]=c[ 1];
	d_c[ 2]=c[ 8];
	d_c[ 3]=c[ 9];
	d_c[ 4]=c[ 4];
	d_c[ 5]=c[ 5];
	d_c[ 6]=c[12];
	d_c[ 7]=c[13];
	d_c[ 8]=c[ 2];
	d_c[ 9]=c[ 3];
	d_c[10]=c[10];
	d_c[11]=c[11];
	d_c[12]=c[ 6];
	d_c[13]=c[ 7];
	d_c[14]=c[14];
	d_c[15]=c[15];
}
