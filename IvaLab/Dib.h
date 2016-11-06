#pragma once

#include <afxwin.h>

class CDib //: public CObject
{
	bool				locked;
	CRITICAL_SECTION	CriticalSection;
public:

	CDib (LPBITMAPINFO pDibInfo, void* pDibBits);
	CDib();
	~CDib ( );

	CDib*	Duplicate();
	void DetachDibBits();
	void AttachDibBits(LPBITMAPINFO pDibInfo, void* pDibBits);

	void	Lock()		{ EnterCriticalSection( &CriticalSection ); locked = true;  }
	void	Unlock()	{ locked = false; LeaveCriticalSection( &CriticalSection ); }
	bool	TryLock()	{ if ( locked ) return false; else { Lock(); return true; }}
	
	BOOL Create( int width, int height, int depth, int colorUsed = 0 );
	BOOL ReadBmp(LPCTSTR szFileName);
	BOOL WriteBmp(LPCTSTR szFileName);
	BOOL AdjustBrightness(int nFactor);

	void*		GetBits();
	inline BITMAPINFO* CDib::GetDibInfo() { return m_pDibInfo; }
	BOOL		GetColor( int index, COLORREF &color );
	BOOL		SetColor( int index, COLORREF color );

	int			GetBitsPerPixel ( );
	BOOL		GetDimension ( CSize& size );
    DWORD		Width();
    DWORD		Height();
	int			GetPaletteCount ();
	DWORD		BytesPerLine();
	CPalette*	GetPalette();

	int			StretchToDC ( CDC& dc, RECT& src, RECT& dst, DWORD rop = SRCCOPY );
	int			SetToDC ( CDC& dc, CRect& src, CPoint& dst );
	BOOL		CopyTo ( CBitmap& );

	BOOL		DoRead ( CFile& file );
	BOOL		DoWrite( CFile& file );

	long		GetBodySize();

	BOOL		ScaleUp(int nxScale, int nyScale);
	BOOL		ScaleDown(int nxScale, int nyScale);
protected:
	int			GetHeaderSize();
	void		InitDibInfo ( int , int = 0, int = 0 );

	BITMAPINFO*	m_pDibInfo;
	void*		m_pDibBits;
	void*		m_pTempDib;
};

inline int CDib::GetBitsPerPixel () { return ( !m_pDibInfo ) ? 0 : m_pDibInfo->bmiHeader.biBitCount; }
inline void* CDib::GetBits() { return m_pDibBits; }
inline DWORD CDib::Width() { return m_pDibInfo->bmiHeader.biWidth; }
inline DWORD CDib::Height() { return m_pDibInfo->bmiHeader.biHeight; }
inline long CDib::GetBodySize() { return BytesPerLine() * m_pDibInfo->bmiHeader.biHeight; }
inline DWORD CDib::BytesPerLine() 
{ 
	DWORD bytes_per_line;
	// fillup byte //
    bytes_per_line = ( m_pDibInfo->bmiHeader.biWidth * GetBitsPerPixel()+7)/8;
	// quad-byte alignment //
    bytes_per_line = ( bytes_per_line + 3 ) / 4;
    return bytes_per_line*4;
}

inline BOOL CDib::GetDimension ( CSize& size )
{
	if ( !m_pDibInfo ) return FALSE;	
	size.cx = m_pDibInfo->bmiHeader.biWidth;
	size.cy = m_pDibInfo->bmiHeader.biHeight;
	return TRUE;
}

