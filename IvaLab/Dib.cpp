#include "StdAfx.h"
#include "Dib.h"


CDib::CDib ()
	: m_pDibBits ( NULL ), m_pDibInfo ( NULL ), locked( false )
{
	m_pTempDib = NULL; //store old dibbits
	
	InitializeCriticalSection( &CriticalSection );
}
	
CDib::CDib (LPBITMAPINFO pDibInfo, void* pDibBits) : locked( false )
{
	ASSERT(pDibInfo);
		
	m_pDibInfo = pDibInfo; //header and color table
	m_pDibBits = pDibBits;
	m_pTempDib = NULL; //store old dibbits
	
	InitializeCriticalSection( &CriticalSection );
}

void CDib::AttachDibBits(LPBITMAPINFO pDibInfo, void* pDibBits)
{
	//return pDib back
	m_pDibBits = pDibBits;
	m_pDibInfo = pDibInfo;
}

void CDib::DetachDibBits()
{
	//return pDib back
	m_pDibBits = NULL;
	m_pDibInfo = NULL;
}

CDib::~CDib ( )
{
	Lock();
	if(m_pDibInfo) delete[] m_pDibInfo;
	if(m_pDibBits) delete[] m_pDibBits;

	if(m_pTempDib) delete[] m_pTempDib;

	Unlock();

	DeleteCriticalSection( &CriticalSection );
}

BOOL	CDib::Create( int width, int height, int depth, int colorUsed )
{
	BITMAPINFOHEADER *bmheader;
	int PaletteSize = 0;
	int cbHeaderSize;

	if ( colorUsed != 0 ) 
		PaletteSize = colorUsed;
	else if ( depth < 16 )
		PaletteSize = ( 1 << depth );

	cbHeaderSize = sizeof(BITMAPINFOHEADER) + PaletteSize * sizeof( RGBQUAD );
	if ( m_pDibInfo ) delete m_pDibInfo;
	m_pDibInfo	= (BITMAPINFO*) new char [ cbHeaderSize ];
	memset( m_pDibInfo, 0, cbHeaderSize );

	bmheader	= &( m_pDibInfo->bmiHeader );
	bmheader->biSize			= sizeof( BITMAPINFOHEADER ); 
	bmheader->biWidth			= width; 
	bmheader->biHeight			= height; 
	bmheader->biPlanes			= 1; 
	bmheader->biBitCount		= depth; 
	bmheader->biCompression		= BI_RGB; 
	bmheader->biSizeImage		= 0; 
	bmheader->biXPelsPerMeter	= 10000; 
	bmheader->biYPelsPerMeter	= 10000; 
	bmheader->biClrUsed			= PaletteSize; 
	bmheader->biClrImportant	= PaletteSize; 

	int	size = BytesPerLine() * height;
	if(m_pDibBits) delete[] m_pDibBits;
	m_pDibBits = (void*) new char[ size];
	memset( m_pDibBits, 0, size );
	bmheader->biSizeImage		= size; 

	//create gray pallete
	RGBQUAD* pPal = (RGBQUAD*) ((LPBYTE)m_pDibInfo + sizeof(BITMAPINFOHEADER));
	for(int i=0;i<PaletteSize;i++)
	{
		pPal->rgbBlue = pPal ->rgbGreen = pPal->rgbRed= i;
		pPal ++;

	}
	return TRUE;
}

CPalette* CDib::GetPalette()
{
	int i;
	CPalette *palette;
	LOGPALETTE *log;

	if ( !GetPaletteCount() ) return NULL;

	log = (LOGPALETTE*) new char [ sizeof(WORD)*2 + sizeof(PALETTEENTRY)*GetPaletteCount() ];
	log->palVersion = 0x300;
	log->palNumEntries = WORD(GetPaletteCount());

	for ( i = 0 ; i < log->palNumEntries ; i ++ )
	{
		log->palPalEntry[i].peRed = m_pDibInfo->bmiColors[i].rgbRed;
		log->palPalEntry[i].peGreen = m_pDibInfo->bmiColors[i].rgbGreen;
		log->palPalEntry[i].peBlue = m_pDibInfo->bmiColors[i].rgbBlue;
		log->palPalEntry[i].peFlags = NULL;
	}

	palette = new CPalette;
	palette->CreatePalette( log );
	delete[] log;
	return palette;
}

BOOL	CDib::GetColor( int index, COLORREF &color )
{
	int PaletteSize = 0;

	if ( m_pDibInfo->bmiHeader.biClrUsed != 0 ) 
		PaletteSize = m_pDibInfo->bmiHeader.biClrUsed;
	else if ( m_pDibInfo->bmiHeader.biBitCount < 16 )
		PaletteSize = ( 1 << m_pDibInfo->bmiHeader.biBitCount );

	if ( index < 0 || index >= PaletteSize )
		return FALSE;
	else {
		color = RGB( m_pDibInfo->bmiColors[ index].rgbRed,
					 m_pDibInfo->bmiColors[ index].rgbGreen,
					 m_pDibInfo->bmiColors[ index].rgbBlue );
		return TRUE;
		}
}

BOOL	CDib::SetColor( int index, COLORREF color )
{
	int PaletteSize = 0;

	if ( m_pDibInfo->bmiHeader.biClrUsed != 0 ) 
		PaletteSize = m_pDibInfo->bmiHeader.biClrUsed;
	else if ( m_pDibInfo->bmiHeader.biBitCount < 16 )
		PaletteSize = ( 1 << m_pDibInfo->bmiHeader.biBitCount );

	if ( index < 0 || index >= PaletteSize )
		return FALSE;
	else {
		m_pDibInfo->bmiColors[ index].rgbBlue		= GetBValue( color );
		m_pDibInfo->bmiColors[ index].rgbGreen		= GetGValue( color );
		m_pDibInfo->bmiColors[ index].rgbRed		= GetRValue( color );
		m_pDibInfo->bmiColors[ index].rgbReserved	= 0;
		return TRUE;
		}
}

BOOL CDib::CopyTo ( CBitmap& ddb )
{
	BOOL ret;
	if ( !m_pDibInfo ) return FALSE;
	// copy DIB to DDB //
	// generate DC of desktop window //
	CWnd DesktopWnd;
	DesktopWnd.Attach ( ::GetDesktopWindow() );
	CWindowDC dtpDC ( &DesktopWnd );
	// use SetDIBits to convert DIB to DDB //

	ret = ::SetDIBits( dtpDC.m_hDC,	// handle of device context
						HBITMAP (ddb),	// handle of bitmap
						0,	// starting scan line
						m_pDibInfo->bmiHeader.biHeight,	// number of scan lines
						m_pDibBits,	// array of bitmap bits
						m_pDibInfo,	// address of structure with bitmap data
						DIB_RGB_COLORS	// type of color indices to use
				 );
	DesktopWnd.Detach();
    return ret;
}

int CDib::SetToDC ( CDC& dc, CRect& src, CPoint& dst )
{
	if ( !m_pDibInfo ) return FALSE;

	return ::SetDIBitsToDevice(
				dc.m_hDC,	// handle of device context
				dst.x,	// x-coordinate of upper-left corner of dest. rect. 
				dst.y,	// y-coordinate of upper-left corner of dest. rect. 
				src.Width(),	// source rectangle width 
				src.Height(),	// source rectangle height 
				src.left,	// x-coordinate of lower-left corner of source rect. 
				src.top,	// y-coordinate of lower-left corner of source rect. 
				0,	// first scan line in array 
				m_pDibInfo->bmiHeader.biHeight,	// number of scan lines 
				m_pDibBits,	// address of array with DIB bits 
				m_pDibInfo,	// address of structure with bitmap info. 
				DIB_RGB_COLORS	// RGB or palette indices 
			);	
}

int CDib::StretchToDC (  CDC& dc, RECT& src, RECT& dst, DWORD rop)
{
	if ( !m_pDibInfo ) return FALSE;
	dc.SetStretchBltMode(STRETCH_DELETESCANS);
	return ::StretchDIBits (
				dc.m_hDC,	// handle of device context
				dst.left,	// x-coordinate of upper-left corner of dest. rect. 
				dst.top,	// y-coordinate of upper-left corner of dest. rect. 
				dst.right - dst.left,	// width of destination rectangle 
				dst.bottom - dst.top,	// height of destination rectangle 
				src.left,	// x-coordinate of lower-left corner of source rect. 
				Height() - src.bottom,	// y-coordinate of lower-left corner of source rect. 
				src.right - src.left,	// source rectangle width 
				src.bottom - src.top,	// source rectangle height 
				m_pDibBits,	// address of array with DIB bits 
				m_pDibInfo,	// address of structure with bitmap info. 
				DIB_RGB_COLORS,	// RGB or palette indices 
				rop		// raster operation code
			);	
}

int CDib::GetPaletteCount ()
{
	int PaletteSize = 0;
	if (!m_pDibInfo) return 0;
	switch ( m_pDibInfo->bmiHeader.biBitCount )
	{
		case 1:
			PaletteSize = 2;
			break;
		case 4:
			PaletteSize = 16;
			break;
		case 8:
			PaletteSize = 256;
			break;
	}
	return PaletteSize;
}


BOOL CDib::ReadBmp(LPCTSTR szFileName)
{
	BOOL ret;
	CFile* pFp = new CFile;
	if(!pFp->Open( szFileName, CFile::modeRead | CFile::typeBinary ))
	{
		delete pFp;
		return FALSE;
	}
	ret = DoRead ( *pFp );
	pFp->Close();
	delete pFp;
	return ret;
}

BOOL CDib::DoRead ( CFile& file )
{
	BITMAPFILEHEADER bmfileheader;
	BITMAPINFOHEADER bmheader;
	long size, headpos;
	int PaletteSize = 0;
	int ret, cbHeaderSize;

	headpos = file.GetPosition();
	ret = file.Read ( &bmfileheader, sizeof(BITMAPFILEHEADER) );
	if ( ( ret != sizeof(BITMAPFILEHEADER) ) || ( bmfileheader.bfType != 0x4d42) )
		return FALSE;
	ret = file.Read ( &bmheader, sizeof(BITMAPINFOHEADER) );
	if ( ret != sizeof(BITMAPINFOHEADER) ) 
		return FALSE;

	if ( bmheader.biClrUsed != 0 ) 
		PaletteSize = bmheader.biClrUsed;
	else if ( bmheader.biBitCount < 16 )
		PaletteSize = ( 1 << bmheader.biBitCount );

	cbHeaderSize = sizeof(BITMAPINFOHEADER) + PaletteSize*sizeof ( RGBQUAD );
	if(m_pDibInfo) delete m_pDibInfo;
	m_pDibInfo = (BITMAPINFO*) new char [ cbHeaderSize ];
	m_pDibInfo->bmiHeader = bmheader;

	if ( PaletteSize )
	{	
		ret = file.Read ( &(m_pDibInfo->bmiColors[0]), PaletteSize*sizeof ( RGBQUAD ) );
		if ( ret != int( PaletteSize*sizeof ( RGBQUAD ) ) )
		{
			delete[] m_pDibInfo;
			m_pDibInfo = NULL;
			return FALSE;
		}
	}
	size = BytesPerLine() * m_pDibInfo->bmiHeader.biHeight;
	if(m_pDibBits) delete[] m_pDibBits;
	m_pDibBits = (void*) new char[GetBodySize()];

	file.Seek ( headpos + bmfileheader.bfOffBits, CFile::begin );

	ret = file.Read ( m_pDibBits, size );
	if ( ret != int ( size ) )
	{
		delete[] m_pDibInfo;
		delete[] m_pDibBits;
		m_pDibInfo = NULL;
		m_pDibBits = NULL;
		return FALSE;
	}

	return TRUE;
}

BOOL CDib::WriteBmp(LPCTSTR szFileName)
{
	BOOL ret;
	CFile fp ( szFileName, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate );
	ret = DoWrite ( fp );
	fp.Close();
	return ret;
}

BOOL CDib::DoWrite( CFile& file )
{
	BITMAPFILEHEADER bmfileheader;
	long size, headpos;
	int PaletteSize = 0;
	int cbHeaderSize;

	bmfileheader.bfType			= 0x4d42;	// 'BM'
	bmfileheader.bfReserved1	= 0;
	bmfileheader.bfReserved2	= 0;

	headpos = file.GetPosition();
	file.Seek( sizeof(BITMAPFILEHEADER), CFile::current );

	if ( m_pDibInfo->bmiHeader.biClrUsed != 0 ) 
		PaletteSize = m_pDibInfo->bmiHeader.biClrUsed;
	else if ( m_pDibInfo->bmiHeader.biBitCount < 16 )
		PaletteSize = ( 1 << m_pDibInfo->bmiHeader.biBitCount );

	cbHeaderSize = sizeof(BITMAPINFOHEADER) + PaletteSize * sizeof( RGBQUAD );

	file.Write( m_pDibInfo, cbHeaderSize );

	bmfileheader.bfOffBits = file.GetPosition() - headpos;

	size = BytesPerLine() * m_pDibInfo->bmiHeader.biHeight;
	file.Write( m_pDibBits, size );

	bmfileheader.bfSize = cbHeaderSize + size + sizeof( bmfileheader );
	file.Seek( headpos, CFile::begin );
	file.Write( &bmfileheader, sizeof( bmfileheader ));

	return TRUE;
}

void CDib::InitDibInfo( int BitsPerPixel, int w, int h )
{
	int i;
	int PaletteSize = 0, cbHeaderSize;

	switch ( BitsPerPixel )	
	{
		case 1:
			PaletteSize = 2;
			break;
		case 4:
			PaletteSize = 16;
			break;
		case 8:
			PaletteSize = 256;
			break;
		case 15:
		case 16:
			BitsPerPixel = 16;
			break;
	}

	cbHeaderSize = sizeof(BITMAPINFOHEADER) + PaletteSize * sizeof ( RGBQUAD );
	if ( m_pDibInfo ) delete[] m_pDibInfo;
	m_pDibInfo = (BITMAPINFO*) new char [ cbHeaderSize ]; /** [6] **/
	m_pDibInfo->bmiHeader.biSize = sizeof ( BITMAPINFOHEADER );
	m_pDibInfo->bmiHeader.biWidth = w;
	m_pDibInfo->bmiHeader.biHeight = h;
	m_pDibInfo->bmiHeader.biPlanes = 1;
	m_pDibInfo->bmiHeader.biBitCount = short(BitsPerPixel);
	m_pDibInfo->bmiHeader.biCompression = BI_RGB;
	m_pDibInfo->bmiHeader.biSizeImage = BytesPerLine() * m_pDibInfo->bmiHeader.biHeight;
	m_pDibInfo->bmiHeader.biXPelsPerMeter =
	m_pDibInfo->bmiHeader.biYPelsPerMeter = 120;
	m_pDibInfo->bmiHeader.biClrUsed = PaletteSize;
	m_pDibInfo->bmiHeader.biClrImportant = PaletteSize;

	if ( PaletteSize )
	{
		for ( i = 0 ; i < PaletteSize ; i ++ )
		{
			m_pDibInfo->bmiColors[i].rgbRed = 0;
			m_pDibInfo->bmiColors[i].rgbGreen = 0;
			m_pDibInfo->bmiColors[i].rgbBlue = 0;
			m_pDibInfo->bmiColors[i].rgbReserved = 0;
		}
	}
}

int CDib::GetHeaderSize()
{
	int PaletteSize;
	switch ( m_pDibInfo->bmiHeader.biBitCount )
	{
		case 1:
			PaletteSize = 2 * sizeof ( RGBQUAD ) + sizeof(BITMAPINFOHEADER);
			break;
		case 4:
			PaletteSize = 16 * sizeof ( RGBQUAD ) + sizeof(BITMAPINFOHEADER);
			break;
		case 8:
			PaletteSize = 256 * sizeof ( RGBQUAD ) + sizeof(BITMAPINFOHEADER);
			break;
		default:
			PaletteSize = sizeof(BITMAPINFOHEADER);
			break;
	}
	return PaletteSize;
}

#define CLIP255(x) min(255,max(0,x))
//nFactor = -128 ~ 128
BOOL CDib::AdjustBrightness(int nFactor)
{

	if(GetBitsPerPixel()!=24)
	{
		TRACE("Sorry not implement for this format!");
		return FALSE;
	}
	if(!m_pTempDib)
			m_pTempDib = (void*) new char[GetBodySize()];
	if(!m_pTempDib)
	{
		TRACE("out of memory\n");
		return FALSE;
	}

	DWORD i,j;
	LPBYTE pIndex = (LPBYTE)m_pTempDib;
	LPBYTE pLine = pIndex;

	for(i=0;i<Height();i++)
	{
		for(j=0;j<3*Width();j++)
		{
			*pIndex = CLIP255(*pIndex +nFactor);
			pIndex++;
		}

		pLine += BytesPerLine();
	}

	void* pTemp = m_pTempDib;
	m_pTempDib = m_pDibBits;
	m_pDibBits = pTemp;
	return TRUE;
}

//2003.11.28
BOOL CDib::ScaleUp(int nxScale, int nyScale)
{
	int nNewWidth,nNewHeight,nOldBpR,nNewBpR;
	int i,j,k,m;
	LPBYTE	pBuffer;

	BITMAPINFOHEADER *bmheader = &( m_pDibInfo->bmiHeader );
	
	nNewWidth = nxScale * bmheader->biWidth;
	nNewHeight = nyScale * bmheader->biHeight;
	nOldBpR = BytesPerLine();
	nNewBpR = nxScale * nOldBpR;
	
	pBuffer = new BYTE[nNewBpR*nNewHeight];
	if(!pBuffer)
		return FALSE;
	LPBYTE pLineSrc = (LPBYTE)m_pDibBits;
	LPBYTE pLineDes = pBuffer;
	LPBYTE pIndexSrc;
	LPBYTE pIndexDes;
	ASSERT(m_pDibBits);

	switch(m_pDibInfo->bmiHeader.biBitCount)
	{
	case 8:
		for( i=0;i<bmheader->biHeight;i++)
		{
			pIndexSrc = pLineSrc;

			for ( m = 0; m <nyScale; m++)	//duplicate line
			{
				pIndexDes = pLineDes;
				for( j=0; j<bmheader->biWidth; j++)
				{
					for ( k=0; k<nxScale;k++) //repeat same pixel
					{
						*pIndexDes ++ = *pIndexSrc;
					}
					pIndexSrc ++;
				}
				pLineDes += nNewBpR;

			}
			pLineSrc += nOldBpR;
		}
		break;
	case 24:
		for( i=0;i<bmheader->biHeight;i++)
		{
			pIndexSrc = pLineSrc;

			for ( m = 0; m <nyScale; m++)	//duplicate line
			{
				pIndexDes = pLineDes;
				for( j=0; j<bmheader->biWidth; j++)
				{
					for ( k=0; k<nxScale;k++) //repeat same pixel
					{
						*pIndexDes ++ = *pIndexSrc;  //B
						*pIndexDes ++ = *(pIndexSrc+1);  //G
						*pIndexDes ++ = *(pIndexSrc+2);  //R
					}
					pIndexSrc +=3;
				}
				pLineDes += nNewBpR;

			}
			pLineSrc += nOldBpR;
		} //case 24
		break;
	default:
		TRACE("not implement yet\n");
		return FALSE;
	}



	//finish duplicate
	Lock();
	delete [] m_pDibBits;
	Unlock();
	m_pDibBits = pBuffer;
	bmheader->biHeight = nNewHeight;
	bmheader->biWidth  = nNewWidth;
	bmheader->biSizeImage *= (nxScale * nyScale);

	return TRUE;

}

//2003.11.28
BOOL CDib::ScaleDown(int nxScale, int nyScale)
{
	int nNewWidth,nNewHeight,nOldBpR,nNewBpR;
	int i,j;
	LPBYTE	pBuffer;


	ASSERT(m_pDibBits);

	BITMAPINFOHEADER *bmheader = &( m_pDibInfo->bmiHeader );
	
	nNewWidth =  bmheader->biWidth / nxScale;
	nNewHeight = bmheader->biHeight/nyScale;
	nOldBpR = BytesPerLine();
	nNewBpR = nOldBpR /nxScale;
	
	pBuffer = new BYTE[nNewBpR*nNewHeight];
	if(!pBuffer)
		return FALSE;
	LPBYTE pLineSrc = (LPBYTE)m_pDibBits;
	LPBYTE pLineDes = pBuffer;
	LPBYTE pIndexSrc;
	LPBYTE pIndexDes;

	switch(m_pDibInfo->bmiHeader.biBitCount)
	{
	case 8:
		for( i=0;i<nNewHeight;i++)
		{
			pIndexSrc = pLineSrc;
			pIndexDes = pLineDes;
			for( j=0; j<nNewWidth; j++)
			{
				*pIndexDes ++ = *pIndexSrc;
				pIndexSrc += nxScale;
			}
			pLineDes += nNewBpR;
			pLineSrc += nOldBpR * nyScale;
		}
		break;
	case 24:
		for( i=0;i<nNewHeight;i++)
		{
			pIndexSrc = pLineSrc;
			pIndexDes = pLineDes;
			for( j=0; j<nNewWidth; j++)
			{
				*pIndexDes ++ = *pIndexSrc;		//R
				*pIndexDes ++ = *(pIndexSrc+1);
				*pIndexDes ++ = *(pIndexSrc+2);
				pIndexSrc += nxScale*3;
			}
			pLineDes += nNewBpR;
			pLineSrc += nOldBpR * nyScale;
		}
		break;
	default:
		return FALSE;
	}

	//finish duplicate
	Lock();
	delete [] m_pDibBits;
	Unlock();
	m_pDibBits = pBuffer;
	bmheader->biHeight = nNewHeight;
	bmheader->biWidth  = nNewWidth;
	bmheader->biSizeImage /= (nxScale * nyScale);

	return TRUE;

}

CDib*	CDib::Duplicate()
{
	int cbHeaderSize = sizeof(BITMAPINFOHEADER) + GetPaletteCount() * sizeof( RGBQUAD );
	BITMAPINFO* pInfo = (BITMAPINFO*) new char [ cbHeaderSize ];
	void* pBits = new char [GetBodySize()];
	if(!pBits)
	{
		delete pInfo;
		return NULL; //out of memory
	}
	memcpy(pInfo, m_pDibInfo,cbHeaderSize);
	memcpy(pBits, m_pDibBits,GetBodySize());
	CDib* pDib = new CDib;
	pDib->AttachDibBits(pInfo, pBits);
	return pDib;
}


