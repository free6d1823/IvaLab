// ChildView.cpp : implementation of the CMainView class
//

#include "stdafx.h"
#include "IvaLab.h"
#include "ChildView.h"
#include	"RGB15YUY2.h"
#include <Mmsystem.h>
#include "ImageWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SUBSAMPLE_X	2
#define SUBSAMPLE_Y	2

#define BLOCK_SIZE_X	4
#define BLOCK_SIZE_Y	4

#define EROSE_SIZE	4
// CMainView

CChildView::CChildView()
{
	m_sizeVideo.cx = 320;
	m_sizeVideo.cy = 240;
	m_nTotalFrames = 0;
	m_nStepFrames = 1;
	m_pDib = NULL;
	m_hTimer = NULL;
	m_pBuffer = NULL;
	m_pBackgndBuffer = NULL;
    m_nHeaderSize = 0;


	m_fZoomFactor = 1.0;
	m_bDrag = FALSE;
	m_bShowPixelInfo = FALSE;
	InitializeCriticalSection(&m_lock);
}

CChildView::~CChildView()
{
	CloseFile();
	DeleteCriticalSection(&m_lock);
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
ON_WM_ERASEBKGND()
	ON_COMMAND(ID_PLAY, OnPlay)
	ON_COMMAND(ID_BEGIN, OnBegin)
	ON_COMMAND(ID_BACK, OnBack)
	ON_COMMAND(ID_END, OnEnd)
	ON_COMMAND(ID_NEXT, OnNext)
	ON_COMMAND(ID_PAUSE, OnPause)
	ON_COMMAND(ID_PREV, OnPrev)
	ON_COMMAND(ID_STOP, OnStop)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_SHOWPIXELINFO, &CChildView::OnViewShowpixelinfo)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnNeedText )
	ON_COMMAND(ID_PROCESS_BACKGROUND, &CChildView::OnProcessBackground)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_BACKGROUND, &CChildView::OnUpdateProcessBackground)
	ON_COMMAND(ID_PROCESS_EXTRACTOBJECT, &CChildView::OnProcessExtractobject)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_EXTRACTOBJECT, &CChildView::OnUpdateProcessExtractobject)
	ON_COMMAND(ID_PROCESS_GRAY, &CChildView::OnProcessGray)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_GRAY, &CChildView::OnUpdateProcessGray)
END_MESSAGE_MAP()



// CMainView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}
BOOL CChildView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return TRUE;
	//CWnd::OnEraseBkgnd(pDC);
}

#define MASK_COLOR	0xff
void	MaskLine(LPBYTE pTar, int nByteToCopy, int nOffset, int nLines)
{
	
	for(int i=nLines;i>0;i--)
	{
		memset(pTar, MASK_COLOR, nByteToCopy);
		pTar += nOffset;
	}
}
void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if(m_pDib)
	{
		CDib*	pTargetDib;
		
		pTargetDib = m_pDib;
		RECT rcClient;
		GetClientRect(&rcClient);
		pTargetDib->StretchToDC(dc,m_rcViewPort, rcClient );
 
	}
	else
	{
		//draw background
		CRect rcClient;
		GetClientRect(&rcClient);
		dc.FillSolidRect(&rcClient,RGB(200,200,200));
	}
	CPen pen(PS_SOLID,1,RGB(0,0,255));
	dc.SelectObject(&pen);
	dc.MoveTo(0,m_sizeVideo.cy-1);
	dc.LineTo(m_sizeVideo.cx-1, m_sizeVideo.cy-1);
	dc.LineTo(m_sizeVideo.cx-1, 0);
	// Do not call CWnd::OnPaint() for painting messages
}
BOOL CChildView::SaveCurrentFrame(LPCTSTR szName) 
{
	if(!m_pDib)
		return FALSE;
	return m_pDib->WriteBmp(szName);
}
void CChildView::UpdateProperty()
{
	// TODO: Add your command handler code here
	if(m_hTimer)
	{
		timeKillEvent(m_hTimer);
		m_hTimer = NULL; 
	}
 

    //
    m_File.SetHeaderSize(m_nHeaderSize);
    m_File.SetProperty(m_sizeVideo, m_dwFourCC);
    m_nBytesPerFrame = m_File.BytesPerFrame();
    m_nTotalFrames = m_File.GetTotalFrames();

	//load first frame
	if(m_nTotalFrames<=0)
	{
		AfxMessageBox("The selected video format not support. Please assign again!!");
		return;
	}
	if(m_pBuffer)
		delete m_pBuffer;
	m_pBuffer = new BYTE[m_nBytesPerFrame];
	if(m_pDib)
		delete m_pDib;
	m_pDib = new CDib;
	if(!m_pDib->Create(m_sizeVideo.cx, m_sizeVideo.cy, 24, 0))
	{
		delete m_pDib;
		m_pDib = NULL;
		return;
	}
	ReadOneFrame();
	m_File.SeekToBegin();
	
	//zoom and position
	m_fZoomFactor = 1.0;
	m_rcViewPort.left = m_rcViewPort.top = 0;
	m_rcViewPort.right = m_sizeVideo.cx;
	m_rcViewPort.bottom = m_sizeVideo.cy;
	//
	m_ToolTipCtrl.AddTool(this, LPSTR_TEXTCALLBACK, &m_rcViewPort, (UINT)this);	
	InvalidateRect(NULL);


}
void CChildView::CloseFile()
{
	m_File.Close();

	if(m_pBuffer)
	{
		delete m_pBuffer;
		m_pBuffer = NULL;
	}
	if(m_pDib)
	{
		delete m_pDib;
		m_pDib = NULL;
	}
	if(m_pBackgndBuffer){
		delete m_pBackgndBuffer;
		m_pBackgndBuffer = NULL;
	}
	//
	
	m_szFilename.Empty();
}

BOOL CChildView::OpenFile(LPCTSTR szName) 
{
	// TODO: Add your command handler code here
	if(m_hTimer)
	{
		timeKillEvent(m_hTimer);
		m_hTimer = NULL; 
	}
	EnterCriticalSection(&m_lock);

	CloseFile();

	m_szFilename = szName;

	LeaveCriticalSection(&m_lock);	

	if(!m_File.Open(m_szFilename))
	{
		AfxMessageBox("Failed to open file!!");
		return FALSE;
	}
    m_File.SetHeaderSize(m_nHeaderSize);
	m_dwFourCC = '024I'; //support I420 only
    m_File.SetProperty(m_sizeVideo,m_dwFourCC);
		//how many frames
    m_nTotalFrames = m_File.GetTotalFrames();
    m_nBytesPerFrame = m_File.BytesPerFrame();


	//load first frame
	if(m_nTotalFrames<=0)
	{
		AfxMessageBox("The selected video format not support. Please assign again!!");
		return FALSE;
	}
	ASSERT(m_pBuffer == NULL);
	m_pBuffer = new BYTE[m_nBytesPerFrame];
	m_pDib = new CDib;
	if(!m_pDib->Create(m_sizeVideo.cx, m_sizeVideo.cy, 24, 0))
	{
		delete m_pDib;
		m_pDib = NULL;
		return FALSE;
	}
	ReadOneFrame();
	
	//zoom and position
	m_fZoomFactor = 1.0;
	m_rcViewPort.left = m_rcViewPort.top = 0;
	m_rcViewPort.right = m_sizeVideo.cx;
	m_rcViewPort.bottom = m_sizeVideo.cy;
	//
	m_ToolTipCtrl.AddTool(this, LPSTR_TEXTCALLBACK, &m_rcViewPort, (UINT)this);
	InvalidateRect(NULL);

	return TRUE;
}
void CALLBACK FrameFunc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CChildView* pThis = (CChildView*) dwUser;
	pThis->PlayNextFame();
}

void CChildView::PlayNextFame()
{
    int nCurFrame = m_File.GetCurrentFrame();
    if(m_nStepFrames > 0)
	{
		if(nCurFrame<m_nTotalFrames)
		{
			nCurFrame += m_nStepFrames;
			if(nCurFrame == m_nTotalFrames)
			{
				OnStop();
			}
            else
            {            
			    m_File.Seek(nCurFrame);
			    ReadOneFrame();
            }
			InvalidateRect(NULL,FALSE);


		}

	}
	else //backward
	{
		if(nCurFrame>0)
		{
			nCurFrame+= m_nStepFrames;
			if(nCurFrame <= 0)
			{
				OnStop();
			}
            else
            {
                m_File.Seek(nCurFrame);
			    ReadOneFrame();
			    InvalidateRect(NULL,FALSE);
            }

		}

	}
}

void CChildView::OnPlay() 
{
	// TODO: Add your command handler code here
	if(!m_File.IsReady())
        return;
	
	if(m_hTimer)
		timeKillEvent(m_hTimer);
	m_hTimer = timeSetEvent(1000/m_nFrameRate, 1, FrameFunc, (DWORD)this, TIME_PERIODIC|TIME_CALLBACK_FUNCTION);

	m_nStepFrames = 1;
	m_File.Seek(m_File.GetCurrentFrame()+1);

}

void CChildView::OnBegin() 
{
    if(!m_File.IsReady())
        return;
	m_File.Seek(0);
	ReadOneFrame(); //
	m_File.Seek(0); //set the pointer to begin
	InvalidateRect(NULL, FALSE);
	
}

void CChildView::OnBack() 
{
    if(!m_File.IsReady())
        return;
	if(m_hTimer)
		timeKillEvent(m_hTimer);
	m_hTimer = timeSetEvent(1000/m_nFrameRate, 1, FrameFunc, (DWORD)this, TIME_PERIODIC|TIME_CALLBACK_FUNCTION);

    if(m_File.GetCurrentFrame() > 0){
		m_nStepFrames = -1;
		m_File.Seek(m_File.GetCurrentFrame()-1);
    }
}

void CChildView::OnEnd() 
{
    if(!m_File.IsReady())
        return;
	m_File.Seek(m_nTotalFrames-1);//show last pic
	ReadOneFrame();
	InvalidateRect(NULL, FALSE);
	
}

void CChildView::OnNext() 
{
    if(!m_File.IsReady())
        return;
	if(m_File.GetCurrentFrame() < m_nTotalFrames-1)
	{
		ReadOneFrame();
		InvalidateRect(NULL, FALSE);
	}
}

void CChildView::OnPause() 
{
    if(!m_File.IsReady())
        return;
	if(m_hTimer)
		timeKillEvent(m_hTimer);
	m_hTimer = NULL; 
}

void CChildView::OnPrev() 
{
	if(!m_File.IsReady())
        return;
	if(m_File.GetCurrentFrame() > 0)
	{
		m_File.Seek(m_File.GetCurrentFrame()-1);
		ReadOneFrame();
		InvalidateRect(NULL, FALSE);
	}
	
}

void CChildView::OnStop() 
{
    if(!m_File.IsReady())
        return;
	if(m_hTimer)
		timeKillEvent(m_hTimer);
	m_hTimer = NULL; 
}


BOOL CChildView::ReadOneFrame()
{
	ASSERT(m_pBuffer);
	ASSERT(m_pDib);

	if((UINT)m_nBytesPerFrame!=m_File.ReadOneFrame(m_pBuffer, &m_nBytesPerFrame))
		return FALSE;

    //notify parrent new frame
	ASSERT(g_pMainFrame);
    g_pMainFrame->SendMessage(WM_CHILD_NOTIFY,m_File.GetCurrentFrame(),(LPARAM)this);
 
 		
   
    //convert to DIB
	gColorInfo[m_File.GetColorIndex()].pConvFunc(m_pBuffer, (BYTE*)m_pDib->GetBits(),m_sizeVideo.cx,
				m_sizeVideo.cy);


	return TRUE;
}


void CChildView::OnZoomIn()
{
	int nWidth, nHeight;

	float newZoom = m_fZoomFactor + (float)0.25;
	if(	newZoom > 10)
		return;
	nWidth = (int)((float)m_sizeVideo.cx / newZoom); 
	if(nWidth < 16)
		return;
		
	nHeight = (int)((float)m_sizeVideo.cy / newZoom); 
	if(nHeight < 16)
		return;

	m_rcViewPort.right = m_rcViewPort.left + nWidth;
	m_rcViewPort.bottom = m_rcViewPort.top + nHeight;
	SetScrollBar();
	m_fZoomFactor = newZoom;
}

void CChildView::OnZoomOut()
{
	int nWidth, nHeight;
	float newZoom = m_fZoomFactor - (float)0.25;

	if(	newZoom <1)
		return;
		m_fZoomFactor = 1;
	nWidth = (int)((float)m_sizeVideo.cx / newZoom); 
	if(nWidth > m_sizeVideo.cx)
		return;

	nHeight = (int)((float)m_sizeVideo.cy / newZoom); 
	if(nHeight > m_sizeVideo.cy)
		return;

	m_fZoomFactor = newZoom;
	m_rcViewPort.right = m_rcViewPort.left + nWidth;
	if(m_rcViewPort.right > m_sizeVideo.cx)
	{
		m_rcViewPort.right =  m_sizeVideo.cx;
		m_rcViewPort.left  = m_rcViewPort.right - nWidth;
	}
	m_rcViewPort.bottom = m_rcViewPort.top + nHeight;
	if(m_rcViewPort.bottom > m_sizeVideo.cy)
	{
		m_rcViewPort.bottom =  m_sizeVideo.cy;
		m_rcViewPort.top  = m_rcViewPort.bottom - nHeight;
	}

	if(m_fZoomFactor<= 1)
		ShowScrollBar(SB_BOTH, FALSE);
	else
		SetScrollBar();
}

//////////////////////////////////////////////////////////////////////////

void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nWidth = m_rcViewPort.bottom- m_rcViewPort.top;
	int nNewPos;
	switch(nSBCode)
	{
	case	SB_TOP: //   Scroll to top. 
		nNewPos = 0;
		break;
	case	SB_LINEUP: //Scroll one line up.
		nNewPos = m_rcViewPort.top -1;
		break;
	case	SB_LINEDOWN: //   Scroll right.
		nNewPos = m_rcViewPort.top +1;
		break;
	case	SB_PAGEUP: //   Scroll one page left.
		nNewPos = m_rcViewPort.top*2 - m_rcViewPort.bottom;
		break;
	case	SB_PAGEDOWN: //   Scroll one page down
		nNewPos = m_rcViewPort.bottom;
		break;
	case	SB_BOTTOM: //   Scroll to bottom.
		nNewPos = m_sizeVideo.cy;
		break;
	case	SB_THUMBPOSITION://   Scroll to absolute position. The current position is specified by the nPos parameter.
	case	SB_THUMBTRACK: //   Drag scroll box to specified position. The current position is specified by the nPos parameter
		nNewPos = nPos;
		break; 

	case	SB_ENDSCROLL:
	default:
		CWnd ::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	if(nNewPos < 0)
	{
		m_rcViewPort.top = 0;
		m_rcViewPort.bottom = nWidth;			
	}
	else 
	{
		m_rcViewPort.bottom = nNewPos +  nWidth;
		if(m_rcViewPort.bottom > m_sizeVideo.cy)
		{		
			m_rcViewPort.bottom = m_sizeVideo.cy;
			m_rcViewPort.top = m_rcViewPort.bottom - nWidth;
		}
		else
			m_rcViewPort.top = nNewPos;

	}

	SetScrollBar();
}

void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nWidth = m_rcViewPort.right- m_rcViewPort.left;
	int nNewPos;
	switch(nSBCode)
	{
	case	SB_LEFT:
		nNewPos = 0;
		break;
	case	SB_LINELEFT: //Scroll left.
		nNewPos = m_rcViewPort.left -1;
		break;
	case	SB_LINERIGHT: //   Scroll right.
		nNewPos = m_rcViewPort.left +1;
		break;
	case	SB_PAGELEFT: //   Scroll one page left.
		nNewPos = m_rcViewPort.left - nWidth;
		break;
	case	SB_PAGERIGHT: //   Scroll one page right.
		nNewPos = m_rcViewPort.right;
		break;
	case	SB_RIGHT: //   Scroll to far right.
		nNewPos = m_sizeVideo.cx;
		break;
	case	SB_THUMBPOSITION://   Scroll to absolute position. The current position is specified by the nPos parameter.
	case	SB_THUMBTRACK: //   Drag scroll box to specified position. The current position is specified by the nPos parameter
		nNewPos = nPos;
		break; 

	case	SB_ENDSCROLL:
	default:
		CWnd ::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	if(nNewPos < 0)
	{
		m_rcViewPort.left = 0;
		m_rcViewPort.right = nWidth;			
	}
	else 
	{
		m_rcViewPort.right = nNewPos +  nWidth;
		if(m_rcViewPort.right > m_sizeVideo.cx)
		{		
			m_rcViewPort.right = m_sizeVideo.cx;
			m_rcViewPort.left = m_rcViewPort.right - nWidth;
		}
		else
			m_rcViewPort.left = nNewPos;

	}
	SetScrollBar();
}

void CChildView::SetScrollBar()
{
	SCROLLINFO scro;
	scro.cbSize = sizeof(SCROLLINFO);
	scro.fMask = SIF_ALL;
	scro.nMin = 0;
	scro.nMax = m_sizeVideo.cx;
	scro.nPage = m_rcViewPort.right - m_rcViewPort.left;
	scro.nPos = m_rcViewPort.left;
	scro.nTrackPos = scro.nPos;
	ShowScrollBar(SB_BOTH, TRUE);
	SetScrollInfo(SB_HORZ, &scro, TRUE);
	scro.nMax = m_sizeVideo.cy;
	scro.nPage = m_rcViewPort.bottom - m_rcViewPort.top;
	scro.nPos = m_rcViewPort.top;
	scro.nTrackPos = scro.nPos;
	SetScrollInfo(SB_VERT, &scro, TRUE);

	InvalidateRect(NULL);
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pDib)
	{
	
		if(zDelta >0)
			OnZoomIn();
		else
			OnZoomOut();
	}
	return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pDib)
	{
	
		m_bDrag = TRUE;
		m_ptLast = point;
	}
	CWnd ::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_bDrag = FALSE;
	CWnd ::OnLButtonUp(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_bDrag)
	{
		HCURSOR hCursor;
		hCursor = AfxGetApp()->LoadCursor(IDC_MOVE);
		SetCursor(hCursor);
		
		POINT dt, newPt1, newPt2;
		dt.x = point.x - m_ptLast.x;
		dt.y = point.y - m_ptLast.y;
		newPt1.x = m_rcViewPort.left - dt.x;
		if(newPt1.x < 0)
			return;
		newPt1.y = m_rcViewPort.top - dt.y;
		if(newPt1.y < 0)
			return;
		newPt2.x = m_rcViewPort.right - dt.x;
		if(newPt2.x > m_sizeVideo.cx)
			return;
		newPt2.y = m_rcViewPort.bottom - dt.y;
		if(newPt2.y > m_sizeVideo.cy)
			return;
		 m_rcViewPort.left = newPt1.x;
		 m_rcViewPort.top = newPt1.y;
		 m_rcViewPort.right = newPt2.x;
		 m_rcViewPort.bottom = newPt2.y;
		 SetScrollBar();
		m_ptLast = point;
	}
 
	CWnd ::OnMouseMove(nFlags, point);
}
void CChildView::OnDestroy() 
{
	if(m_hTimer)
	{
		timeKillEvent(m_hTimer);
		m_hTimer = NULL; 
	}
	EnterCriticalSection(&m_lock);

	CWnd ::OnDestroy();
	LeaveCriticalSection(&m_lock);	
}


BOOL CChildView::SetCurrentFrame(int nFrameIndex)
{
	if(nFrameIndex < 0 || nFrameIndex>= m_nTotalFrames)
		return FALSE;
	m_File.Seek(nFrameIndex);
	ReadOneFrame();
	InvalidateRect(NULL, FALSE);

	return TRUE;
}


void* CChildView::GetCurrentDibBits()
{
	if(!m_pDib)
		return NULL;
	return m_pDib->GetBits();
}

BITMAPINFO* CChildView::GetCurrentDibInfo()
{
	if(!m_pDib)
		return NULL;
	return m_pDib->GetDibInfo();

}

void CChildView::UpdateClientSize()
{
	CRect rcClient, rcWindow;
	GetWindowRect(&rcWindow);
	GetClientRect(&rcClient); //old client
	rcWindow.right += m_sizeVideo.cx - rcClient.Width();
	rcWindow.bottom += m_sizeVideo.cy - rcClient.Height();
//	rcWindow.InflateRect(1,1);
	MoveWindow(&rcWindow, TRUE);

}


LPBYTE CChildView::GetCurrentFrame(int* pnLength)
{
    *pnLength = m_nBytesPerFrame;
    return m_pBuffer;
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd ::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    RECT rc;
    GetClientRect(&rc);
	m_ToolTipCtrl.Create(this, TTS_ALWAYSTIP);

    m_ToolTipCtrl.Activate(FALSE);
    m_ToolTipCtrl.SetDelayTime(10);
	
	return 0;
}


BOOL CChildView::OnNeedText(UINT id, NMHDR * pTTTStruct, LRESULT * pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pTTTStruct;
//    if (pTTT->uFlags & TTF_IDISHWND)
    if((CChildView*)pTTTStruct->idFrom == this)
    {
        // idFrom is actually the HWND of the tool
//        nID = ::GetDlgCtrlID((HWND)nID);
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(&pt);
        CRect rc;
        GetClientRect(&rc);
        if(rc.PtInRect(pt))
        {   
			unsigned char y,u,v;
			y = GetYuv(pt.x, pt.y, &u, &v);
            wsprintf(pTTT->lpszText, "(%d,%d)=[%d,%d,%d]", pt.x, pt.y, y,u,v);
            pTTT->hinst = NULL;
			* pResult = 1;
        }
        return(TRUE); //don't continue
    }
    return(FALSE);

}
 
void CChildView::OnViewShowpixelinfo()
{
	m_bShowPixelInfo = 1 - m_bShowPixelInfo;

	if(m_bShowPixelInfo){
	   m_ToolTipCtrl.Activate(TRUE);
	}else
		m_ToolTipCtrl.Activate(FALSE);

}
BOOL CChildView::PreTranslateMessage(MSG* pMsg) 
{
    m_ToolTipCtrl.RelayEvent(pMsg);
    
	return CWnd::PreTranslateMessage(pMsg);
}

//return Y, and pass value to u,v
unsigned char CChildView::GetYuv(LONG x, LONG y, unsigned char * u,  unsigned char * v)
{
	if(!m_pBuffer)
		return 0;
	if( x>= m_sizeVideo.cx || y >m_sizeVideo.cy)
		return 0;
	unsigned char value = m_pBuffer[y* m_sizeVideo.cx + x];
	unsigned char*  uplan = m_pBuffer + m_sizeVideo.cx * m_sizeVideo.cy;
	unsigned char*  vplan = uplan + m_sizeVideo.cx * m_sizeVideo.cy/4;

	*u = uplan[y* m_sizeVideo.cx/4 + x/2];
	*v = vplan[y* m_sizeVideo.cx/4 + x/2];
	return value;
}

void CChildView::ShowYuvOnWnd(void* pYuv, int w, int h, int format)
{
	CDib* pDib = new CDib;
	if(!pDib->Create(w, h, 24, 0))
	{
		delete pDib;
		return;
	}
	int index;
	switch(format){
	case '024I': 
		index = 4;
		break;
	case 'yarg':
	default:
		index =10;
		break;
	}
    //convert to DIB
	gColorInfo[index].pConvFunc((LPBYTE) pYuv, (BYTE*)pDib->GetBits(), w, h);
	ImageWnd* pImgWnd = new ImageWnd();
	if( !pImgWnd->Create(this)){
		delete pDib;	
		return;
	}
	pImgWnd->SetImage(pDib);
	pImgWnd->ShowWindow(SW_SHOW );
}

void CChildView::OnProcessGray()
{
	UINT len = m_sizeVideo.cx * m_sizeVideo.cy;
	BYTE* pBuffer = (BYTE*) malloc(len);
	memcpy(pBuffer, m_pBuffer, len);
 
	/// Object ectraction end
	ShowYuvOnWnd(pBuffer, m_sizeVideo.cx, m_sizeVideo.cy, 'yarg');
	free (pBuffer);
}

void CChildView::OnUpdateProcessGray(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pBuffer != NULL);
}

#define CLIP255(x) ((( (x<256)?x:255)>0)?x:0)

void SubsampleY(LPBYTE pDst, LPBYTE pSrc, int cx, int cy, int dx, int dy)
{
	int i,j;
	int m,n;
	int blk;
	LPBYTE pLineDst;
	LPBYTE pLineSrc;
	LPBYTE p;
	for(i=0;i<cy;i+= dy){
		pLineSrc = pSrc + i*cx;
		pLineDst = pDst + (i/dy)*(cx/dx);
		//start of a line 
		for(j=0; j<cx; j+= dx){
			/////////within a block
			blk = 0;
			p = pLineSrc;
			for(m=0;m<dy;m++){
				for(n=0;n<dx; n++){
					blk +=p[n]; 
				}
				p += cx; //next line of block
			}
			/////////within a block
			*pLineDst++ = (BYTE) (blk/(m*n));
			pLineSrc += dx;
		}
		//end of a line
	}
}

void CChildView::OnProcessBackground()
{
	if( !m_File.IsReady())
		return;
	UINT len = m_File.BytesPerFrame();
	int nCurrentFrame = m_File.GetCurrentFrame();
	BYTE* pBufferTemp = (BYTE*) malloc(len);

	BYTE* pBaseBuffer = (BYTE*) malloc(m_sizeVideo.cx * m_sizeVideo.cy/(SUBSAMPLE_X*SUBSAMPLE_Y) );
	BYTE* pBuffer = (BYTE*) malloc(m_sizeVideo.cx * m_sizeVideo.cy/(SUBSAMPLE_X*SUBSAMPLE_Y) );
	m_File.Seek(44); //use 44-frame as bkgnd
	if(len != m_File.ReadOneFrame(pBufferTemp, (int*) &len) ){
		free( pBuffer);	
		return;
	}
	SubsampleY(pBaseBuffer, pBufferTemp, m_sizeVideo.cx, m_sizeVideo.cy, SUBSAMPLE_X, SUBSAMPLE_Y);
	m_File.Seek(0);
	for (int i=0; i< m_File.GetTotalFrames(); i++){
	
		ASSERT (len == m_File.ReadOneFrame(pBufferTemp, (int*) &len) );
		SubsampleY(pBuffer, pBufferTemp, m_sizeVideo.cx, m_sizeVideo.cy, SUBSAMPLE_X, SUBSAMPLE_Y);

		for(int j=0; j< m_sizeVideo.cx * m_sizeVideo.cy/(SUBSAMPLE_X*SUBSAMPLE_Y); j++){
			int p = (int) pBaseBuffer[j];
			int q = (int)pBuffer[j];
			if ( abs(p-q) < 10)
				pBaseBuffer[j] = CLIP255((p*i +q)/(i+1));
			else
				pBaseBuffer[j] = p;//CLIP255( (p*99 + q*1)/100);
		}
	
	}

	////
	ShowYuvOnWnd(pBaseBuffer, m_sizeVideo.cx/SUBSAMPLE_X, m_sizeVideo.cy/SUBSAMPLE_Y, 'yarg');
	
	m_File.Seek(nCurrentFrame);
	/////////////////
	if(pBufferTemp)
		free(pBufferTemp);
	if(pBuffer)
		free(pBuffer);
	if(m_pBackgndBuffer)
		free(m_pBackgndBuffer);
	m_pBackgndBuffer = pBaseBuffer;
}

void CChildView::OnUpdateProcessBackground(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nTotalFrames > 0);
}

void CChildView::OnProcessExtractobject()
{
	int len = m_sizeVideo.cx * m_sizeVideo.cy/(SUBSAMPLE_X*SUBSAMPLE_Y);
	BYTE* pBuffer = (BYTE*) malloc(len);
	BYTE* pMask = (BYTE*) malloc(len);

	SubsampleY(pBuffer, m_pBuffer, m_sizeVideo.cx, m_sizeVideo.cy, SUBSAMPLE_X, SUBSAMPLE_Y);
	/// Object ectraction start
	int i;
	for (i=0; i<len; i++){
		pMask[i] = ( abs(pBuffer[i] - m_pBackgndBuffer[i]) < 60)?0:0xff;
		if( abs(pBuffer[i] - m_pBackgndBuffer[i]) < 60)
			pBuffer[i] = 0;
	
	}
	/// Object ectraction end
	ShowYuvOnWnd(pBuffer, m_sizeVideo.cx/SUBSAMPLE_X, m_sizeVideo.cy/SUBSAMPLE_Y, 'yarg');
	ShowYuvOnWnd(pMask, m_sizeVideo.cx/SUBSAMPLE_X, m_sizeVideo.cy/SUBSAMPLE_Y, 'yarg');

	free (pBuffer);
	free(pMask);
}

void CChildView::OnUpdateProcessExtractobject(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pBackgndBuffer != NULL);
}
