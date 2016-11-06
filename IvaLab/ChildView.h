// ChildView.h : interface of the CMainView class
//


#pragma once
#include "Dib.h"
#include "ImageSource.h"
#include "Easytip.h"

#define STATUS_STOP	0
#define STATUS_PLAY	1
#define STATUS_PAUSE	2
// CMainView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	//return Y, and pass value to u,v
	unsigned char GetYuv(LONG x, LONG y, unsigned char * u,  unsigned char * v);
	void ShowYuvOnWnd(void* pYuv, int w, int h, int format);
	BOOL	CanOpen(){return (m_hTimer== NULL);	}
	BOOL OpenFile(LPCTSTR szName);
	void UpdateProperty();
	BOOL SaveCurrentFrame(LPCTSTR szName);
	void PlayNextFame();
	int GetCurrentFrameIndex(){return m_File.GetCurrentFrame();}
	CString	m_szFilename;
	SIZE	m_sizeVideo;
	SIZE	m_sizeResized;
	int		m_nFrameRate;
	DWORD	m_dwFourCC;
    int     m_nHeaderSize;
	int		m_nTotalFrames;
	int		m_nBytesPerFrame;

	CDib*	m_pDib;
	//zoom
	float	m_fZoomFactor;
	RECT	m_rcViewPort; //view range in the doc
	BOOL	m_bDrag;
	POINT	m_ptLast;
	BOOL	m_bShowPixelInfo;
	    Easytip    m_ToolTipCtrl;
private:
	CRITICAL_SECTION	m_lock;
	CImageSource	m_File;
	LPBYTE	m_pBuffer; //current image buffer
	LPBYTE	m_pBackgndBuffer; //background buffer
	LPBYTE  m_pResized; //resized buffer
	LPBYTE  m_pMask; //ROI buffer, 8-bit/pixel
	UINT	m_hTimer;
	int		m_nState; //0=stop, 1=play
	int		m_nStepFrames;
//	int		m_nCurFrame;
	int		m_nSpeed;	
	
	//}}AFX_VIRTUAL
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	LPBYTE GetCurrentFrame(int* pnLength);
	void CloseFile();
	void UpdateClientSize();
	BITMAPINFO* GetCurrentDibInfo();
	void* GetCurrentDibBits();
	BOOL SetCurrentFrame(int nFrameIndex);
	void SetScrollBar();
	BOOL DoShrinkYUY2();
	void OnZoomOut();
	void OnZoomIn();
	void DoDeInterlaceI420();
	void DoDeInterlaceYUY2();
	BOOL ReadOneFrame();
	virtual ~CChildView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPlay();
	afx_msg void OnBegin();
	afx_msg void OnBack();
	afx_msg void OnEnd();
	afx_msg void OnNext();
	afx_msg void OnPause();
	afx_msg void OnPrev();
	afx_msg void OnStop();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnNeedText(UINT id, NMHDR * pTTTStruct, LRESULT * pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewShowpixelinfo();
	afx_msg void OnProcessBackground();
	afx_msg void OnUpdateProcessBackground(CCmdUI *pCmdUI);
	afx_msg void OnProcessExtractobject();
	afx_msg void OnUpdateProcessExtractobject(CCmdUI *pCmdUI);
	afx_msg void OnProcessGray();
	afx_msg void OnUpdateProcessGray(CCmdUI *pCmdUI);
	afx_msg void OnProcessEdgedetection();
	afx_msg void OnUpdateProcessEdgedetection(CCmdUI *pCmdUI);
	afx_msg void OnProcessDilation();
	afx_msg void OnUpdateProcessDilation(CCmdUI *pCmdUI);
};

