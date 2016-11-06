#include "StdAfx.h"
#include "IspFunc.h"

IspFunc::IspFunc(void)
{
}

IspFunc::~IspFunc(void)
{
}
/*
	Ouput a edge detected bilevel image, 8-bits per pixel
	Input: gray scale image
	Sobel filter ----
	 1  2  1   1 0 -1
	 0  0  0   2 0 -2
	-1 -2 -1   1 0 -1
*/
void ispEdgeDetection(PImageInfo pInfo, LPBYTE pMask,int threshold)
{
	int i,j;
	BYTE* p1 = pInfo->pSrc;
	BYTE* p2 = p1 + pInfo->stride;
	BYTE* p3 = p2 + pInfo->stride;
	BYTE* t = pInfo->pOut;
 
	memset(t, 0, pInfo->width*pInfo->height); 
	t += pInfo->width;
	int x, y;
	int offset;
	for(i=1;i<pInfo->height-1; i++){
		offset = i* pInfo->stride +1;
		for (j=1; j< pInfo->width - 1; j++) {
			if(*(pMask + offset++ ) == 0)
				continue;

			x = p1[j-1]+2*p1[j] + p1[j+1] - p3[j-1] - 2*p3[j] - p3[j+1];
			y = p1[j-1]+2*p2[j-1] + p3[j-1] - p1[j+1] - 2*p2[j+1] - p3[j+1];
			if (abs(x) + abs(y) > threshold) t[j] = 0xff;
		
		}
		t+= pInfo->width;
		p1=p2; p2 = p3; p3 += pInfo->stride;
	}
	
}

/* dilation with block size (2*size+1) pixels */
inline bool IS_EDGE(BYTE* p, int stride)
{

	if(p[-1] ^ p[1] == 0xff) return true;
	if(p[-stride] ^ p[stride] == 0xff) return true;
	return false;
}
void ispDilation(PImageInfo pInfo, int size)
{
	int i,j,m;
	BYTE* p1 = pInfo->pOut;
	BYTE* p0 = pInfo->pOut+ pInfo->stride*size;
	BYTE* p2 = pInfo->pSrc + pInfo->stride*size;
	BYTE* pt;
	int d = 2*size+1; 
	memset(p1, 0, pInfo->width*pInfo->height); 
	for(i=size;i<pInfo->height-size-1; i++){

		for (j=size; j< pInfo->width - size -1; j++) {
			if(p2[j] == 0xff) p0[j] = 0xff;
			if(IS_EDGE(p2+j, pInfo->stride))
			{
				pt = p1 +j - size;
				for(m = 0; m< d; m++){
					memset(pt, 0xff, d);
					pt += pInfo->stride;
				}
			}
		
		}
		p0 += pInfo->stride;
		p1 += pInfo->stride;
		p2 += pInfo->stride;
	}
	
}