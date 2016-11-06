#pragma once


// ImageWnd
#include "dib.h"
#include "Easytip.h"
class ImageWnd : public CWnd
{
// Construction
public:
	ImageWnd();

// Attributes
public:
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ImageWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
private:
 
    CDib* m_pDib;
    LPBYTE  m_pRef;
    LPBYTE  m_pTarget;
    SIZE    m_sizeVideo;
    DWORD   m_dwColorSpace;
    int     m_nCompareChannel; //RGB or YUV channel, first channel = 0
    int     m_nPixelStep;//bytes to next pixel
    int     m_nOffset;  //bytes to the channel
	int		m_nBytesPerFrame;
    SIZE    m_sizeRange;//pixel range to compare
    CRect    m_rcSelection;
    RGBQUAD m_Table[512];
    LPRGBQUAD m_pTable;
    Easytip    m_ToolTipCtrl;
//    CToolTipCtrl    m_ToolTipCtrl;
// Implementation
public:
	void SetImage(CDib* pDib);
	int GetDiff(int x, int y);
	void ResizeWindow();
	void SelectChannel(int nChannel);
	void SetFrames(LPBYTE pRef, LPBYTE pTar, int nLength);
	void Calculate();
	void SetImageProperty(SIZE size, DWORD dwColor);
	BOOL Create(CWnd* pParent);
	virtual ~ImageWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(ImageWnd)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnNeedText(UINT id, NMHDR * pTTTStruct, LRESULT * pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

