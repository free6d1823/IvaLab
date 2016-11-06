#include "StdAfx.h"
#include "Easytip.h"

// Easytip.cpp : implementation file
//
 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Easytip
#define ET_BKCOLOR  0x00ccffff
#define ET_TXCOLOR  0x00330000
#define WAIT_TIME 50
#define SHOW_TIME 2000
#define WIDTH_OFFSET 8
#define HEIGHT_OFFSET 12
#define TOP_OFFSET 24
#define TEXT_X 4
#define TEXT_Y 6
#define TIME_INTERVAL   10
#define TIMER_ID        10023
#define DELAY_MOVE      2
enum TipStatus
{
	TS_OFF = 0,
    TS_ON,
    TS_WAIT,
	TS_SHOW,
};
Easytip::Easytip()
{
    m_pOwner = NULL;
    m_nDelay = WAIT_TIME;
    m_nState = TS_OFF;
    m_nShowTime = SHOW_TIME;
    m_nDelayMove = 0;
}

Easytip::~Easytip()
{
}


BEGIN_MESSAGE_MAP(Easytip, CWnd)
	//{{AFX_MSG_MAP(Easytip)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Easytip message handlers

void Easytip::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
    if(nIDEvent != TIMER_ID)
        return;
	m_nTick += TIME_INTERVAL;

    if(m_nState == TS_SHOW)
    {
        if(m_nTick > m_nShowTime)
        {
            ShowWindow(SW_HIDE);
            KillTimer(TIMER_ID);
            m_nState = TS_ON;
            m_nTick = 0;
        }
    }
    else if(m_nState == TS_WAIT)
    {
        if(m_nTick > m_nDelay)
        {
			if(ShowTip())
	            m_nState = TS_SHOW;
            m_nTick = 0;
            m_nDelayMove = DELAY_MOVE;
        }
    }
	CWnd::OnTimer(nIDEvent);
}

int Easytip::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_font.CreateFont(14,0,0,0,400,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
         DEFAULT_QUALITY,DEFAULT_PITCH|FF_MODERN,"Arial");

    return 0;
}

void Easytip::OnDestroy() 
{
    Activate(FALSE);
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void Easytip::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
    CRect rc;
    GetClientRect(&rc);
	rc.DeflateRect(3,2);

    dc.SelectObject(&m_font);
    dc.SetTextColor(ET_TXCOLOR);
    dc.SetBkColor(ET_BKCOLOR);
	dc.DrawText(m_szText, &rc, DT_LEFT);
	// Do not call CWnd::OnPaint() for painting messages
}

void Easytip::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
    //point = parent coord
    if(m_nDelayMove > 0)
    {
        m_nDelayMove --;
        return;
    }
    if(m_nState == TS_WAIT )
    {
        if(m_rcTool.PtInRect(point))
        {
            m_nTick = 0; //reset
			m_ptLast = point;
        }
        else
        {
           SetWindowPos(&CWnd::wndTop, 0,0,0,0, 
                SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
            KillTimer(TIMER_ID);
            m_nState = TS_ON;
        }
    }
    else if(m_nState == TS_SHOW)
    {
        TRACE("Mouse move don't show. tick = %d\n",m_nTick);
        //if pt in rect, hide
        KillTimer(TIMER_ID);
        m_nState = TS_ON;
//        ShowWindow(SW_HIDE);
        SetWindowPos(&CWnd::wndTop, 0,0,0,0, 
                SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
        //else count to hide
        m_nTick = 0;
    }
    else if(m_nState == TS_ON)
    {
        //if pt in tool, start timer
        if(m_rcTool.PtInRect(point))
        {
            TRACE("Mouse in tool, tick = %d\n",m_nTick);
            SetTimer(TIMER_ID, TIME_INTERVAL, NULL);
            m_nState = TS_WAIT;
            m_nTick = 0; //reset
        }
    }
	//(m_nState == TS_OFF) don't care
    
//	CWnd::OnMouseMove(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////
BOOL Easytip::Create( CWnd* pParentWnd, DWORD dwStyle)
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	LPSTR lpszName = "Easytip::WNDCLASS";
	WNDCLASS wndcls;
	if (!::GetClassInfo(hInst, lpszName, &wndcls))
	{
		wndcls.style = CS_DBLCLKS|CS_CLASSDC;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = 0;
		wndcls.hbrBackground = CreateSolidBrush(ET_BKCOLOR);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = lpszName;
		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
	if(!CWnd::CreateEx(
					WS_EX_TOOLWINDOW,
					lpszName,
					"",
					WS_POPUP,  //stytle
					0, 0,150,20,
					pParentWnd->m_hWnd, NULL, NULL))	
		return FALSE;

	return TRUE;    
}

BOOL Easytip::AddTool( CWnd* pWnd, LPCTSTR lpszText, LPCRECT lpRectTool, UINT nIDTool)
{
    
    if(lpszText != LPSTR_TEXTCALLBACK)
            m_szText = lpszText;
    m_rcTool = *lpRectTool; //client coord
    m_nIDTool = nIDTool;
    m_pOwner = pWnd;
    return TRUE;


}
void Easytip::Activate( BOOL bActivate )
{
    if(bActivate){
        if(m_nState == TS_OFF)
        {
           m_nState = TS_ON;
        }
    }
    else
    {
        if(m_nState != TS_OFF)
        {
            if(m_nState > TS_ON)
                KillTimer(TIMER_ID);
           m_nState = TS_OFF;
		   if(IsWindowVisible())
			ShowWindow(SW_HIDE);
        }      
    }
}
void Easytip::RelayEvent( LPMSG lpMsg )
{
	if(lpMsg->message == WM_MOUSEMOVE)
    SendMessage(lpMsg->message, lpMsg->wParam, lpMsg->lParam);
}

BOOL Easytip::ShowTip()
{
	if(!m_pOwner)
		return FALSE;
	LPTOOLTIPTEXT pTip = new TOOLTIPTEXT;
	memset(pTip, 0, sizeof(TOOLTIPTEXT));
	NMHDR * pHd = (NMHDR *)pTip;
	pHd->code = TTN_NEEDTEXT;
	pHd->idFrom = m_nIDTool;
	pHd->hwndFrom = m_hWnd;
	pTip->lpszText = pTip->szText;
	CPoint pos = CPoint(25,25);
	if(m_pOwner->SendMessage(WM_NOTIFY,0, (LPARAM)pHd))
	{
		m_szText = pTip->lpszText;
		pos += m_ptLast;
		m_pOwner->ClientToScreen(&pos);
		SetWindowPos(&CWnd::wndTop, pos.x,pos.y,0,0, 
			SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOACTIVATE);
		delete pTip;
		return TRUE;
	}

	delete pTip;
	return FALSE;

}
