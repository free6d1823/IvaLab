#pragma once

class IspFunc
{
public:
	IspFunc(void);
	~IspFunc(void);
};
typedef struct _ImgInfo{
	BYTE* pSrc; 
	int width; 
	int height; 
	int stride;
	BYTE* pOut;
}ImgInfor, *PImageInfo;
/*
	Ouput a edge detected bilevel image, 8-bits per pixel, in the pMask area
	Input: gray scale image
*/
void ispEdgeDetection(PImageInfo pInfo, LPBYTE pMask, int threshold);

/* dilation with block size (2*size+1) pixels   */
void ispDilation(PImageInfo pInfo, int size);
