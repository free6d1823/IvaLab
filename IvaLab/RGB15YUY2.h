#ifndef	_RGB15YUY2_H
#define	_RGB15YUY2_H

#include	<stdio.h>

typedef	enum	{ 
		RYCERR_OK, 
		RYCERR_OUT_OF_MEMORY, 
		RYCERR_TABLE_NOT_FOUND, 
		RYCERR_WIDTH_NOT_EVEN,
		RYCERR_INVALID_BUFFER,
		RYCERR_INVALID_FORMAT,
		RYCERR_ALL
} RYCERR;

typedef RYCERR (*ConvertRGBFunc) ( LPBYTE src, LPBYTE dst, int width, int height );

class	YUV2RGB_Convertor	{
public:
	YUV2RGB_Convertor();
	~YUV2RGB_Convertor();
static RYCERR  ConvertUYVY_24( LPBYTE src, LPBYTE dst, int width, int height );
static 		RYCERR	ConvertYVU9_24( LPBYTE src, LPBYTE dst, int width, int height );
static 		RYCERR	ConvertYUY2_24( LPBYTE src, LPBYTE dst, int width, int height );
static 		RYCERR	ConvertI420_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertI422_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertRGB_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertBGR_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertI444_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertY444_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertGray_8( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertPGM5_24( LPBYTE src, LPBYTE dst, int width, int height );
static 	RYCERR	ConvertRGBA_32( LPBYTE src, LPBYTE dst, int width, int height );


};

extern	YUV2RGB_Convertor		YUV2RGB;	

#endif	//	_RGB15YUY2_H
