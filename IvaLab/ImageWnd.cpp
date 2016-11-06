// ImageWnd.cpp : implementation file
//

#include "stdafx.h"
#include "IvaLab.h"
#include "ImageWnd.h"


// ImageWnd

ImageWnd::ImageWnd()
{
	m_pDib = NULL;

    //reference pointer, don't delete
    m_pTarget = NULL;
    m_pRef = NULL;
    m_nCompareChannel = 0; //RGB or YUV channel, first channel = 0
    m_nOffset = 0;  //bytes to next pixel
    m_nPixelStep = 1;//bytes to next pixel
   	m_nBytesPerFrame = 0;

    m_pTable = m_Table + 256; //from -256 to +255
}

ImageWnd::~ImageWnd()
{
	if(m_pDib)
		delete m_pDib;
	m_pDib = NULL;
}


BEGIN_MESSAGE_MAP(ImageWnd, CWnd)
	//{{AFX_MSG_MAP(ImageWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
    ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnNeedText )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ImageWnd message handlers

void ImageWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
    RECT rcClient;
    RECT rcVideo = {0,0,m_sizeVideo.cx,m_sizeVideo.cy};
    
    GetClientRect(&rcClient);
    if(m_pDib)
		m_pDib->StretchToDC(dc,rcVideo, rcClient );
	if ( ! m_rcSelection.IsRectEmpty()) 
    {
        CBrush pen(RGB(255,0,0));
        dc.FrameRect(&m_rcSelection, &pen);
    }

	// Do not call CWnd::OnPaint() for painting messages
}

int ImageWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
    RECT rc;
    GetClientRect(&rc);
	m_ToolTipCtrl.Create(this, TTS_ALWAYSTIP);
    m_ToolTipCtrl.AddTool(this, LPSTR_TEXTCALLBACK, &rc, (UINT)this);
    m_ToolTipCtrl.Activate(TRUE);
    m_ToolTipCtrl.SetDelayTime(10);
//    m_ToolTipCtrl.EnableToolTips();

	return 0;
}
//click X button
void ImageWnd::OnClose() 
{
	CWnd::OnClose();
	//will call OnDestroy next
}
void ImageWnd::OnDestroy() 
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
	delete this;
}

void ImageWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void ImageWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL ImageWnd::Create(CWnd *pParent)
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	LPSTR lpszName = "ImageWnd::WNDCLASS";
	WNDCLASS wndcls;
	if (!::GetClassInfo(hInst, lpszName, &wndcls))
	{
		wndcls.style = CS_DBLCLKS|CS_CLASSDC;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);;
		wndcls.hCursor = 0;
		wndcls.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = lpszName;
		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
	if(!CWnd::CreateEx(
//					WS_EX_ACCEPTFILES|WS_EX_CONTEXTHELP,
					0,
					lpszName,
					"Image",
					WS_OVERLAPPED| WS_CAPTION|WS_SYSMENU, //stytle
					0, 0,320,240,
					pParent->m_hWnd, NULL, NULL))	
		return FALSE;
 
	return TRUE;
}


void ImageWnd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
    RECT rc;
	if(bShow)
    {
        //get position
        rc.left = AfxGetApp()->GetProfileInt("Diff", "x", 800);
        rc.top = AfxGetApp()->GetProfileInt("Diff", "y", 80);
    	SetWindowPos(&CWnd::wndTop, rc.left, rc.top, 0,0,SWP_NOSIZE);
    }
    else
    {
  		GetWindowRect(&rc);
		AfxGetApp()->WriteProfileInt("Diff", "x", rc.left);
		AfxGetApp()->WriteProfileInt("Diff", "y", rc.top);      
    }
}

void ImageWnd::SelectChannel(int nChannel)
{
    m_nCompareChannel = nChannel;
    if(m_dwColorSpace == '2YUY')
    {
        if(nChannel == 1)
        {
            m_nOffset = 1;//U
            m_nPixelStep = 4;
            m_sizeRange.cx = m_sizeVideo.cx/2;
            m_sizeRange.cy = m_sizeVideo.cy;

        }
        else if(nChannel == 2) 
        {
            m_nOffset = 3;//V
            m_nPixelStep = 4;
            m_sizeRange.cx = m_sizeVideo.cx/2;
            m_sizeRange.cy = m_sizeVideo.cy;
        }
        else//(nChannel == 0)
        {
            m_nOffset = 0;//Y
            m_nPixelStep = 2;
            m_sizeRange.cx = m_sizeVideo.cx;
            m_sizeRange.cy = m_sizeVideo.cy;
        }
    }
    else if(m_dwColorSpace == '024I')
    {
        if(nChannel == 1)
        {
            m_nOffset = m_sizeVideo.cx * m_sizeVideo.cy;//U
            m_sizeRange.cx = m_sizeVideo.cx/2;
            m_sizeRange.cy = m_sizeVideo.cy/2;
        }
        else if(nChannel == 2) 
        {
            m_nOffset = m_sizeVideo.cx * m_sizeVideo.cy *5/4;//V
            m_sizeRange.cx = m_sizeVideo.cx/2;
            m_sizeRange.cy = m_sizeVideo.cy/2;
        }
        else//(nChannel == 0)
        {
            m_nOffset = 0;//Y
            m_sizeRange.cx = m_sizeVideo.cx;
            m_sizeRange.cy = m_sizeVideo.cy;

        }
        m_nPixelStep = 1;

    }
    else if(m_dwColorSpace == '024I')
    {
        m_nOffset = 0;//Y
        m_sizeRange.cx = m_sizeVideo.cx;
        m_sizeRange.cy = m_sizeVideo.cy;
        m_nPixelStep = 1;
    }
    else if(m_dwColorSpace == 'ABGR')
    {
        m_nOffset = 0;//Y
        m_sizeRange.cx = m_sizeVideo.cx;
        m_sizeRange.cy = m_sizeVideo.cy;
        m_nPixelStep = 4;
    }
    else //compare one by one
    {
        m_nOffset = 0;//Y
        m_sizeRange.cx = m_sizeVideo.cx;
        m_sizeRange.cy = m_sizeVideo.cy;
        m_nPixelStep = 1;
        
    }
}

void ImageWnd::SetImage(CDib* pDib)
{
    	if(m_pDib)
		    delete m_pDib;
		m_pDib = pDib;
		m_sizeVideo.cx = pDib->Width();
		m_sizeVideo.cy = pDib->Height();
		ResizeWindow();
		RECT rc = {0,0,m_sizeVideo.cx, m_sizeVideo.cy};
	   m_ToolTipCtrl.AddTool(this, LPSTR_TEXTCALLBACK, &rc, (UINT)this);
}
void ImageWnd::SetImageProperty(SIZE size, DWORD dwColor)
{
    if(dwColor != m_dwColorSpace || size.cx != m_sizeVideo.cx || size.cy != m_sizeVideo.cy)
    {
    	if(m_pDib)
		    delete m_pDib;
	    m_pDib = new CDib;
	    if(!m_pDib->Create(size.cx, size.cy, 24, 0))
	    {
		    delete m_pDib;
		    m_pDib = NULL;
		    return;
	    }
        m_dwColorSpace = dwColor;
        m_sizeVideo = size;
        ResizeWindow();
		RECT rc = {0,0,size.cx, size.cy};
	   m_ToolTipCtrl.AddTool(this, LPSTR_TEXTCALLBACK, &rc, (UINT)this);
    }
    SelectChannel(0);

}
void ImageWnd::SetFrames(LPBYTE pRef, LPBYTE pTar, int nLength)
{
    m_pRef = pRef;
    m_pTarget = pTar;
    Calculate();
}
//return RGB at point (x,y)
DWORD ImageWnd::GetColor(DWORD x, DWORD y)
{
    if(!m_pDib)
        return 0;
	if(x>=m_pDib->Width() || y >= m_pDib->Height())
        return 0;
	BYTE* p1 = (BYTE*)m_pDib->GetBits();
	BYTE* p = (BYTE*)m_pDib->GetBits() + x*m_pDib->GetBitsPerPixel()/8 + (m_pDib->Height()-y-1)* m_pDib->BytesPerLine();
	DWORD color = 0;
	color = p[0] | (p[1]<<8) | (p[2]<<16);//0xrrggbb
    return color;
}

void ImageWnd::Calculate()
{
    int i,j;
    int diff;
    LPBYTE pRef = m_pRef + m_nOffset;
    LPBYTE pTar = m_pTarget + m_nOffset;
    int nBpL = m_pDib->BytesPerLine();
    LPBYTE pLine = (LPBYTE)m_pDib->GetBits() + //last line
        (m_pDib->Height()-1)*nBpL;
    LPBYTE pPixel;
    for(i=0;i<m_sizeRange.cy;i++)
    {
        pPixel = pLine;
        for(j=0;j<m_sizeRange.cx;j++)
        {
            diff = *pTar - *pRef; //-255~255
            *pPixel++ = m_pTable[diff].rgbBlue;
            *pPixel++ = m_pTable[diff].rgbGreen;
            *pPixel++ = m_pTable[diff].rgbRed;
            pTar += m_nPixelStep;
            pRef += m_nPixelStep;
        }
        pLine -= nBpL;
    }

    InvalidateRect(NULL,FALSE);
}


void ImageWnd::ResizeWindow()
{
	CRect rcClient, rcWindow;
	GetWindowRect(&rcWindow);
	GetClientRect(&rcClient); //old client
	rcWindow.right += m_sizeVideo.cx - rcClient.Width();
	rcWindow.bottom += m_sizeVideo.cy - rcClient.Height();
//	rcWindow.InflateRect(1,1);
	MoveWindow(&rcWindow, TRUE);

}

void ImageWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	/*
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MAPPING));
    int nSel = AfxGetApp()->GetProfileInt("Diff", "map", 1);

	menu.GetMenuItemCount();
	CMenu* pMenuPopup = menu.GetSubMenu(0);
    for(int i=0;i<4;i++)
        pMenuPopup->CheckMenuItem(i, (nSel==i)?MF_CHECKED| MF_BYPOSITION :MF_UNCHECKED|MF_BYPOSITION);
	pMenuPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
							   point.x,
							   point.y,
							   this);
	*/
}



BOOL ImageWnd::PreTranslateMessage(MSG* pMsg) 
{
    m_ToolTipCtrl.RelayEvent(pMsg);
    
	return CWnd::PreTranslateMessage(pMsg);
}


BOOL ImageWnd::OnNeedText(UINT id, NMHDR * pTTTStruct, LRESULT * pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pTTTStruct;
//    if (pTTT->uFlags & TTF_IDISHWND)
    if((ImageWnd*)pTTTStruct->idFrom == this)
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
			DWORD color = GetColor(pt.x, pt.y);
            wsprintf(pTTT->lpszText, "(%d,%d)=[%d,%d,%d]", pt.x, pt.y, (color>>16)&0xff, (color >>8 )&0xff, color&0xff);
            pTTT->hinst = NULL;
			* pResult = 1;
        }
        return(TRUE); //don't continue
    }
    return(FALSE);

}


POINT   m_ptStart;
POINT   m_ptEnd;

void ImageWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if ( GetCapture() == this ) {
//		CDC	*dc = GetDC();
		if ( ! m_rcSelection.IsRectEmpty()) 
//				dc->DrawFocusRect( &m_rcSelection );			
                InvalidateRect(&m_rcSelection, FALSE);
    
        m_ptEnd = point;
	    CRect	rc(m_ptStart, m_ptEnd);
	    rc.NormalizeRect();
	    GetClientRect( &m_rcSelection );
	    m_rcSelection.IntersectRect( &m_rcSelection, &rc );
		if ( ! m_rcSelection.IsRectEmpty()) {
//			dc->DrawFocusRect( &m_rcSelection );
            InvalidateRect(&m_rcSelection, FALSE);

		}
//		ReleaseDC(dc);
    }
//	CWnd::OnMouseMove(nFlags, point);
}

void ImageWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( GetCapture() == this ) {
        OnMouseMove(nFlags, point);
		HCURSOR hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		ReleaseCapture();
		SetCursor(hCursor);
        
		memset(&m_ptStart,0,sizeof(m_ptStart));
		memset(&m_ptEnd,0,sizeof(m_ptEnd));	
	}	
	CWnd::OnLButtonUp(nFlags, point);
}

void ImageWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	HCURSOR hCursor = AfxGetApp()->LoadStandardCursor(IDC_CROSS);
	SetCursor(hCursor);
	SetCapture();
    //clear last rect
//	CDC	*dc = GetDC();
	if ( ! m_rcSelection.IsRectEmpty()) 
//			dc->DrawFocusRect( &m_rcSelection );			
        InvalidateRect(&m_rcSelection, FALSE);
//	ReleaseDC(dc);

	m_ptStart = m_ptEnd = point;
	m_rcSelection.SetRectEmpty();
	
	CWnd::OnLButtonDown(nFlags, point);
}
