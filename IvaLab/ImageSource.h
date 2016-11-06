// ImageSource.h: interface for the CImageSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGESOURCE_H__6344DC23_7902_4503_B1B4_994E55D3C0AC__INCLUDED_)
#define AFX_IMAGESOURCE_H__6344DC23_7902_4503_B1B4_994E55D3C0AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include	"RGB15YUY2.h"

typedef struct _SupportColorFormat{
	DWORD	dwFource;
	int		nBitsPerPixel;
	ConvertRGBFunc	pConvFunc; //convert dwFource to RGB24
}SUPPORT_COLOR;
extern SUPPORT_COLOR	gColorInfo[];

class CImageSource  
{
public:
	BOOL ProcessPGMHeader();
    void SetHeaderSize(int nLength){m_nHeadLength = nLength; }
	void SetProperty(SIZE szFrame, DWORD dwColor);
	UINT ReadOneFrame(LPBYTE pBuffer, int* pnLength);
	void Seek(int nFrame);
	void SeekToBegin();
	BOOL Open(LPCTSTR szFilename);
    void Close();
	CImageSource();
	virtual ~CImageSource();
//property
inline    BOOL IsReady(){return m_bIsReady;}
inline    int GetCurrentFrame(){return m_nCurFrame-1;  }//顯示frame
inline    int GetTotalFrames(){return m_nTotalFrames;}
inline    int BytesPerFrame(){return m_nBytesPerFrame; }
inline      int GetColorIndex(){return m_nSourceColorIndex;}
private:
    BOOL    m_bIsReady;
    CFile*   m_pFile;
    int     m_nHeadLength;
    SIZE    m_sizeVideo;
    int     m_nBytesPerFrame;
//    DWORD   m_dwFourCC;
	int		m_nCurFrame; //將要 display 的frame ID
	int		m_nSourceColorIndex;
	int		m_nTotalFrames;


};

#endif // !defined(AFX_IMAGESOURCE_H__6344DC23_7902_4503_B1B4_994E55D3C0AC__INCLUDED_)
