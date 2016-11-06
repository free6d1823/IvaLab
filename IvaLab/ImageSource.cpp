// ImageSource.cpp: implementation of the CImageSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



SUPPORT_COLOR	gColorInfo[] = 
{
	'2YUY', 16, YUV2RGB_Convertor::ConvertYUY2_24,
	'YVYU', 16, YUV2RGB_Convertor::ConvertUYVY_24,
	'3BGR', 24, YUV2RGB_Convertor::ConvertRGB_24,
	'3RGB', 24, YUV2RGB_Convertor::ConvertBGR_24,
	'024I', 12, YUV2RGB_Convertor::ConvertI420_24,
	'9UVY', 9, YUV2RGB_Convertor::ConvertYVU9_24,
	'224I', 16,YUV2RGB_Convertor::ConvertI422_24,
	'444I', 24, YUV2RGB_Convertor::ConvertI444_24,
	'444Y', 24, YUV2RGB_Convertor::ConvertY444_24,
	'5mgp', 12, YUV2RGB_Convertor::ConvertPGM5_24,
	'yarg', 8, YUV2RGB_Convertor::ConvertGray_8,
	'ABGR', 32, YUV2RGB_Convertor::ConvertRGBA_32,
};
CImageSource::CImageSource()
{
    m_pFile = NULL;
    m_nHeadLength = 0;
    m_bIsReady = FALSE;
    m_nSourceColorIndex = 0;
}

CImageSource::~CImageSource()
{
    Close();
}
void CImageSource::Close()
{
    if(m_pFile)
    {
        delete m_pFile;
        m_pFile = NULL;
    }
    m_bIsReady = FALSE;
	m_nTotalFrames = 0;
    m_nBytesPerFrame = 0;
}
BOOL CImageSource::Open(LPCTSTR szFilename)
{
    Close();
    m_pFile = new CFile;
	if(!m_pFile->Open(szFilename, CFile::modeRead|CFile::shareDenyNone))
    {
        delete m_pFile;
        m_pFile = NULL;
        return FALSE;
    }

    return TRUE;
}

void CImageSource::SeekToBegin()
{
	m_pFile->Seek(m_nHeadLength, CFile::begin);
    m_nCurFrame = 0;
}

void CImageSource::Seek(int nFrame)
{
    if(!m_pFile)
        return;
    if(nFrame < 0 || nFrame > m_nTotalFrames)
        return;

    m_pFile->Seek( m_nHeadLength + nFrame * m_nBytesPerFrame, CFile::begin);
    m_nCurFrame = nFrame;
        
}
//pnLength = input buffer length, 
//return bytes copied. If return 0, *pnLength = bytes need
UINT CImageSource::ReadOneFrame(LPBYTE pBuffer, int *pnLength)
{
    if(!m_pFile)
        return -1;
    if(*pnLength < m_nBytesPerFrame)//not enough
    {
        *pnLength = m_nBytesPerFrame;
        return 0;
    }
    int nRet = m_pFile->Read(pBuffer, m_nBytesPerFrame);
    if(nRet < m_nBytesPerFrame)
        return nRet;
    //sucessed
    m_nCurFrame ++;
    return nRet;
}

void CImageSource::SetProperty(SIZE szFrame, DWORD dwColor)
{
    if(!m_pFile)
        return;
	DWORD dwLength = m_pFile->GetLength();
	int nCnt = sizeof(gColorInfo)/sizeof(SUPPORT_COLOR);
	m_nTotalFrames = 0;
    int nSourceColorIndex = -1;
    for(int i=0;i<nCnt;i++)
	{
		if(dwColor == gColorInfo[i].dwFource)
		{
			nSourceColorIndex = i; 
		}
	}
    if(nSourceColorIndex < 0)
    {
        TRACE("Color space %X not support", dwColor);
        return;
    }
//    m_nHeadLength = 0;
//    if(dwColor == '5MGP')
//    {
//        if(ProcessPGMHeader())
//            m_nSourceColorIndex = 7;
//    }
//    else
    {
        m_sizeVideo = szFrame;
        m_nSourceColorIndex = nSourceColorIndex;
    }

	m_nBytesPerFrame =  m_sizeVideo.cx * m_sizeVideo.cy*gColorInfo[nSourceColorIndex].nBitsPerPixel / 8;
	m_nTotalFrames = (dwLength - m_nHeadLength)/m_nBytesPerFrame;
    m_bIsReady = TRUE;
    SeekToBegin();
}

BOOL CImageSource::ProcessPGMHeader()
{
    ASSERT(m_pFile);
    char    dummy[256];
    m_pFile->SeekToBegin();
    m_pFile->Read(dummy, 256);
    if ( (dummy[0] != 'P') || (dummy[1] != '5' ))
        return FALSE;
    char* token = strtok( dummy, "\n ");
    token = strtok( NULL, "\n "); //#XVID
    token = strtok( NULL, "\n "); //width
    m_sizeVideo.cx = atoi(token);
    token = strtok( NULL, "\n "); //height
    m_sizeVideo.cy = atoi(token);;//*2/3; 
    token = strtok( NULL, "\n "); //maxval
    int nMaxValue = atoi(token);;//*2/3; 
    if(nMaxValue != 255)
        return FALSE; //not support
    m_nHeadLength = token + strlen(token) + 1 - dummy;
    return TRUE;
}

