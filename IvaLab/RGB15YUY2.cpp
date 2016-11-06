
//#ifndef		__BORLANDC__
#include	<stdafx.h>
//#else
//#include	<windows.h>
//#endif

#include	"RGB15YUY2.h"

/////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////

YUV2RGB_Convertor		YUV2RGB;	
/////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////

#define	crv	104597
#define	cbu	132201
#define	cgu	25675
#define	cgv	53279
long	crv_tab[256];
long	cbu_tab[256];
long	cgu_tab[256];
long	cgv_tab[256];
long	tab_76309[256];
BYTE	clp[1024];			//for clip in CCIR601

YUV2RGB_Convertor::YUV2RGB_Convertor()
{
	int i;
	for ( i = 0; i < 256; i++ ) {
		crv_tab[ i]	= ( i - 128 ) * crv;
		cbu_tab[ i]	= ( i - 128 ) * cbu;
		cgu_tab[ i]	= ( i - 128 ) * cgu;
		cgv_tab[ i]	= ( i - 128 ) * cgv;
		tab_76309[ i] = 76309 * ( i - 16 );
		}
	 
	for ( i = 0; i < 384; i++ )
		clp[i] = 0;
	for ( ; i < 256 + 384; i++ )
		clp[ i] = i - 384;
	for ( ; i < 256 + 2 * 384; i++ )
		clp[ i] = 255;
}

/////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////

YUV2RGB_Convertor::~YUV2RGB_Convertor()
{
}

/////////////////////////////////////////////////////////////////////////
// Convert UYVY to RGB
/////////////////////////////////////////////////////////////////////////
RYCERR  YUV2RGB_Convertor::ConvertUYVY_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pU	= src;
	LPBYTE	pY	= src+1;
	LPBYTE	pV	= src+2;

	dst		+= width * height * 3; //last line

	for ( int i1 = height; i1 > 0; i1-- ) {
		dst -= 3 * width;
		LPBYTE	rgb = dst;
		for ( int i2 = width / 2; i2 > 0; i2-- ) {
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];

			for ( int i = 2; i > 0; i-- ) {
				int	y = tab_76309[ *pY];	
				*rgb++ = clp[ 384+(( y + c4 ) >> 16 )]; //b
				*rgb++ = clp[ 384+(( y - c2 - c3 ) >> 16)]; //g
				*rgb++ = clp[ 384+(( y + c1 ) >> 16 )];  //r
				pY += 2;
				}
			pU += 4;
			pV += 4;
			}
		}
	return RYCERR_OK;

}
/////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////

RYCERR  YUV2RGB_Convertor::ConvertYUY2_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY	= src;
	LPBYTE	pU	= src + 1;
	LPBYTE	pV	= src + 3;

	dst		+= width * height * 3; //last line

	for ( int i1 = height; i1 > 0; i1-- ) {
		dst -= 3 * width;
		LPBYTE	rgb = dst;
		for ( int i2 = width / 2; i2 > 0; i2-- ) {
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];

			for ( int i = 2; i > 0; i-- ) {
				int	y = tab_76309[ *pY];	
				*rgb++ = clp[ 384+(( y + c4 ) >> 16 )]; //b
				*rgb++ = clp[ 384+(( y - c2 - c3 ) >> 16)]; //g
				*rgb++ = clp[ 384+(( y + c1 ) >> 16 )];  //r
				pY += 2;
				}
			pU += 4;
			pV += 4;
			}
		}
	return RYCERR_OK;
}

RYCERR	YUV2RGB_Convertor::ConvertYVU9_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE	pY2 = pY1 + width;
	LPBYTE	pY3 = pY2 + width;
	LPBYTE	pY4 = pY3 + width;

	LPBYTE	pV	= src + width * height;
	LPBYTE	pU	= pV  + width * height/16;
	LPBYTE  rgb1;
	LPBYTE  rgb2;
	LPBYTE  rgb3;
	LPBYTE  rgb4;

	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height/4; i1 > 0; i1-- ) 
	{
		dst -= nBpR*4;
		rgb4 = dst;
		rgb3 = rgb4 + nBpR;
		rgb2 = rgb3 + nBpR;
		rgb1 = rgb2 + nBpR;
		
		for ( int i2 = width/4; i2 > 0; i2-- ) 
		{
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];
			//Y11
			int	y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++; 			
			//Y12
			y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			//Y13
			y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			//Y14
			y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;

			//line 2
			//y21
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			//y22
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			//y23
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			//y24
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			//line 3
			//y31
			y = tab_76309[ *pY3];	
			*rgb3++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb3++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb3++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY3 ++;
			//y32
			y = tab_76309[ *pY3];	
			*rgb3++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb3++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb3++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY3 ++;
			//y33
			y = tab_76309[ *pY3];	
			*rgb3++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb3++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb3++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY3 ++;
			//y34
			y = tab_76309[ *pY3];	
			*rgb3++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb3++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb3++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY3 ++;
			//line 4
			//y41
			y = tab_76309[ *pY4];	
			*rgb4++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb4++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb4++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY4 ++;
			//y42
			y = tab_76309[ *pY4];	
			*rgb4++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb4++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb4++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY4 ++;
			//y43
			y = tab_76309[ *pY4];	
			*rgb4++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb4++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb4++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY4 ++;
			//y34
			y = tab_76309[ *pY4];	
			*rgb4++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb4++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb4++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY4 ++;			
			pU++;
			pV++;
		}

		pY1 += (width*3);
		pY2 = pY1+width;
		pY3 = pY2+width;
		pY4 = pY3+width;

	}
	return RYCERR_OK;
}//ConvertYVU9_24

//Plan mode
// Y0Y1Y2Y3Y4Y5Y6Y7
// Y Y Y Y Y Y Y Y
// Y8Y9YAYBYCYDYEYF
// Y Y Y Y Y Y Y Y
// U0U2U4U6v0v2v4v6
// U9UAUCUEv9vAvCvE
RYCERR	YUV2RGB_Convertor::ConvertPGM5_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE	pY2 = pY1 + width;
	LPBYTE	pU	= src + width * height;
    int halfwidth = width/2;
	LPBYTE	pV	= pU  + halfwidth;
	LPBYTE  rgb1;
	LPBYTE  rgb2;
	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height/2; i1 > 0; i1-- ) 
	{
		dst -= nBpR*2;
		rgb2 = dst;
		rgb1 = dst + nBpR;
		
		for ( int i2 = width/2; i2 > 0; i2-- ) 
		{
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];
			int	y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			//line 2
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			//p3
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			
			pU++;
			pV++;
		}
        pU += halfwidth;
        pV += halfwidth;
		pY1 += width;
		pY2 = pY1+width;

	}
	return RYCERR_OK;    
}

RYCERR	YUV2RGB_Convertor::ConvertI420_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE	pY2 = pY1 + width;
	LPBYTE	pU	= src + width * height;
	LPBYTE	pV	= pU  + width * height/4;
	LPBYTE  rgb1;
	LPBYTE  rgb2;
	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height/2; i1 > 0; i1-- ) 
	{
		dst -= nBpR*2;
		rgb2 = dst;
		rgb1 = dst + nBpR;
		
		for ( int i2 = width/2; i2 > 0; i2-- ) 
		{
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];
			int	y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			//line 2
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			//p3
			y = tab_76309[ *pY2];	
			*rgb2++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb2++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb2++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY2 ++;
			
			pU++;
			pV++;
		}

		pY1 += width;
		pY2 = pY1+width;

	}
	return RYCERR_OK;
}

/////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////


RYCERR	YUV2RGB_Convertor::ConvertI422_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE	pU	= pY1 + width * height;
	LPBYTE	pV	= pU  + width * height/2;
	LPBYTE  rgb1;
	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height; i1 > 0; i1-- ) 
	{
		dst -= nBpR;
		rgb1 = dst;
		for ( int i2 = width/2; i2 > 0; i2-- ) 
		{
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];
			int	y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			y = tab_76309[ *pY1]; 
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			
			pU++;
			pV++;
		}

	}
	return RYCERR_OK;
}
RYCERR	YUV2RGB_Convertor::ConvertRGB_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	memcpy(dst, src, width*height*3);
	return RYCERR_OK;

}

//RGB bottom-down
RYCERR	YUV2RGB_Convertor::ConvertBGR_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	int nBpR = width*3;
	dst += width * height * 3;
    for(int i=0;i<height; i++)
    {
        dst -= nBpR;
        for(int j=0; j<width; j++)
        {
            *dst = *(src+2);
            *(dst+1) = *(src+1);
            *(dst+2) = *(src);
            dst += 3;
            src += 3;
        }
        dst -= nBpR;
    }
	return RYCERR_OK;

}
RYCERR	YUV2RGB_Convertor::ConvertI444_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE	pU	= src + width * height;
	LPBYTE	pV	= pU  + width * height;
	LPBYTE  rgb1;
	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height; i1 > 0; i1-- ) 
	{
		dst -= nBpR;
		rgb1 = dst;
		
		for ( int i2 = width; i2 > 0; i2-- ) 
		{
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];
			int	y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1++;
			
			pU++;
			pV++;
		}


	}
	return RYCERR_OK;
}

RYCERR	YUV2RGB_Convertor::ConvertY444_24( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE	pU	= src + 1;
	LPBYTE	pV	= pU  + 2;
	LPBYTE  rgb1;
	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height; i1 > 0; i1-- ) 
	{
		dst -= nBpR;
		rgb1 = dst;
		
		for ( int i2 = width; i2 > 0; i2-- ) 
		{
			int	u = *pU;
			int	v = *pV;

			int	c1 = crv_tab[ v];
			int	c2 = cgu_tab[ u];
			int	c3 = cgv_tab[ v];
			int	c4 = cbu_tab[ u];
			int	y = tab_76309[ *pY1];
			*rgb1++ = clp[ 384+(( y + c4 ) >> 16 )];
			*rgb1++ = clp[ 384+(( y - c2 - c3 ) >> 16)];
			*rgb1++ = clp[ 384+(( y + c1 ) >> 16 )];  
			pY1+=3;
			
			pU+=3;
			pV+=3;
		}


	}
	return RYCERR_OK;
}
//
RYCERR	YUV2RGB_Convertor::ConvertGray_8( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE  rgb1;
	int nBpR = width*3;
	dst += nBpR * height;
	for ( int i1 = height; i1 > 0; i1-- ) 
	{
		dst -= nBpR;
		rgb1 = dst;
		
		for ( int i2 = width; i2 > 0; i2-- ) 
		{
            *rgb1++ = *src;
            *rgb1++ = *src;
            *rgb1++ = *src;
            src++;
        }
    }
    
    return RYCERR_OK;
}
//convert RGBA to Bitmap BGR24
RYCERR	YUV2RGB_Convertor::ConvertRGBA_32( LPBYTE src, LPBYTE dst, int width, int height )
{
	LPBYTE	pY1 = src;
	LPBYTE  rgb1;
	int nBpR = width*3;
	dst += width * height * 3;
	for ( int i1 = height; i1 > 0; i1-- ) 
	{
		dst -= nBpR;
		rgb1 = dst;
		
		for ( int i2 = width; i2 > 0; i2-- ) 
		{
            *rgb1++ = *(src+2);//B
            *rgb1++ = *(src+1);//G
            *rgb1++ = *src;//R
            src+=4;
        }
    }
    
    return RYCERR_OK;
}
