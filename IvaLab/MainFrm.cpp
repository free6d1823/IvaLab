// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "IvaLab.h"

#include "MainFrm.h"
#include "MediaProperty.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_OPEN, &CMainFrame::OnOpen)
	ON_UPDATE_COMMAND_UI(ID_OPEN, &CMainFrame::OnUpdateOpen)
	ON_COMMAND(ID_FILE_PROPERTIES, &CMainFrame::OnFileProperties)
	ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, &CMainFrame::OnUpdateFileProperties)
	ON_COMMAND(ID_VIEW_ZOOMIN, &CMainFrame::OnViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, &CMainFrame::OnUpdateViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, &CMainFrame::OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, &CMainFrame::OnUpdateViewZoomout)
	ON_COMMAND(ID_EDIT_SNAPSHOT, &CMainFrame::OnEditSnapshot)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SNAPSHOT, &CMainFrame::OnUpdateEditSnapshot)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FRAME, OnUpdateFrame)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_COLOR, OnUpdateColor)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SIZE, OnUpdateSize)
	ON_COMMAND(ID_FILE_CAPTUREFOLDER, &CMainFrame::OnFileCapturefolder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_FRAME,
	ID_INDICATOR_COLOR,
	ID_INDICATOR_SIZE,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 640, 480), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_CONTROLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



void CMainFrame::OnOpen()
{
	CString strFilter = "YUV|*.yuv|All|*.*||" ;
		
	CString strFileName = AfxGetApp()->GetProfileString("MediaProperty", "File", NULL);
	CFileDialog aFileDialog( TRUE, NULL, strFileName,
		                     OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
							 strFilter);
	
	int nID = aFileDialog.DoModal();
	if( nID == IDOK ){
		//CString szExt = aFileDialog.GetPathName();
		//szExt = szExt.Mid(szExt.ReverseFind('.')+1);

		MediaProperty	dlg;

		//dlg.m_szExt = szExt;

		if(dlg.DoModal()!= IDOK)
			return;
		m_wndView.m_nFrameRate = dlg.m_nFPS;
 

		SetClientWindow(dlg.m_sizeVideo.cx,dlg.m_sizeVideo.cy);
    	m_wndView.m_nHeaderSize  = dlg.m_nHeaderSize;

		if(m_wndView.OpenFile(aFileDialog.GetPathName()))
		{
			SetWindowText(aFileDialog.GetFileTitle());
		}
		strFileName = aFileDialog.GetPathName();
		AfxGetApp()->WriteProfileString("MediaProperty", "File", strFileName);

		m_wndStatusBar.UpdateWindow();
	}
}

void CMainFrame::SetClientWindow(int nWidth, int nHeight)
{
	m_wndView.m_sizeVideo.cx = nWidth;
	m_wndView.m_sizeVideo.cy = nHeight;
	CRect rcClient, rcWindow;
	GetWindowRect(&rcWindow);
	m_wndView.GetClientRect(&rcClient); //old client
	rcWindow.right += m_wndView.m_sizeVideo.cx - rcClient.Width();
	rcWindow.bottom += m_wndView.m_sizeVideo.cy - rcClient.Height();
//	rcWindow.InflateRect(1,1);
	MoveWindow(&rcWindow, TRUE);

	m_wndStatusBar.SetPaneInfo(0,indicators[0],SBPS_NOBORDERS,max(0,rcWindow.Width()-250));
	m_wndStatusBar.SetPaneInfo(1,indicators[1],SBPS_NORMAL,50);
	m_wndStatusBar.SetPaneInfo(2,indicators[2],SBPS_NORMAL,50);
	m_wndStatusBar.SetPaneInfo(3,indicators[3],SBPS_NORMAL,150);
}

void CMainFrame::OnUpdateOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_wndView.CanOpen());
}

void CMainFrame::OnFileProperties()
{
	MediaProperty	dlg;
	//dlg.m_szExt = szExt;

	if(dlg.DoModal()!= IDOK)
		return;
	m_wndView.m_sizeVideo = dlg.m_sizeVideo;
	m_wndView.m_nFrameRate = dlg.m_nFPS;
	//m_wndView.m_dwFourCC  = dlg.m_dwFourCC;
	m_wndView.m_nHeaderSize  = dlg.m_nHeaderSize;
	m_wndView.UpdateProperty();

	CRect rcClient, rcWindow;
	GetWindowRect(&rcWindow);
	m_wndView.GetClientRect(&rcClient); //old client
	rcWindow.right += m_wndView.m_sizeVideo.cx - rcClient.Width();
	rcWindow.bottom += m_wndView.m_sizeVideo.cy - rcClient.Height();
	rcWindow.InflateRect(1,1);
	MoveWindow(&rcWindow, TRUE);

}
void CMainFrame::OnUpdateFileProperties(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CMainFrame::OnViewZoomin()
{
	m_wndView.OnZoomIn();
}

void CMainFrame::OnUpdateViewZoomin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CMainFrame::OnViewZoomout()
{
	m_wndView.OnZoomOut();
}

void CMainFrame::OnUpdateViewZoomout(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CMainFrame::OnEditSnapshot()
{
	CTime tm = CTime::GetCurrentTime();
	CString szName = tm.Format("%Y%m%d_%H%M%S.bmp");
	m_wndView.SaveCurrentFrame(szName);
}

void CMainFrame::OnUpdateEditSnapshot(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}
void CMainFrame::OnUpdateFrame(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CString string;
	string.Format ("%d/%d", m_wndView.GetCurrentFrameIndex(), m_wndView.m_nTotalFrames);
	pCmdUI->Enable (TRUE);
	pCmdUI->SetText (string);

}void CMainFrame::OnUpdateColor(CCmdUI* pCmdUI) 
{
	CString string;
	string.Format ("%4.2f", m_wndView.m_fZoomFactor);
	pCmdUI->Enable (TRUE);
	pCmdUI->SetText (string);

}
void CMainFrame::OnUpdateSize(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CString string;
	string.Format ("%dx%d", m_wndView.m_sizeVideo.cx, m_wndView.m_sizeVideo.cy);
	pCmdUI->Enable (TRUE);
	pCmdUI->SetText (string);

}
void CMainFrame::OnFileCapturefolder()
{
	CString strFolder = AfxGetApp()->GetProfileString("Capture", "Folder", NULL);

    LPMALLOC pMalloc; //,pMalloc2;
    CString strDirectory;
    BROWSEINFO bi;
    /* Gets the Shell's default allocator */
    char pszBuffer[MAX_PATH];
	strcpy_s(pszBuffer, MAX_PATH, strFolder);
    LPITEMIDLIST pidl;
    // Get help on BROWSEINFO struct - it's got all the bit settings.
	bi.hwndOwner = this->m_hWnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = pszBuffer;
    bi.lpszTitle = _T("Select A Directory");
    bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    bi.lpfn = NULL;
    bi.lParam = 0;
    
    if (::SHGetMalloc(&pMalloc) == NOERROR)
    {
        if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
        {
            if (::SHGetPathFromIDList(pidl, pszBuffer))
            { 
                //strDirectory = pszBuffer;
				AfxGetApp()->WriteProfileString("Capture", "Folder", pszBuffer);
            }
            pMalloc->Free(pidl);
        }
        pMalloc->Release();
    }

}
