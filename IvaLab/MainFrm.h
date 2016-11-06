// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "ChildView.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CChildView    m_wndView;
public:
	void SetClientWindow(int nWidth, int nHeight);
// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpen();
	afx_msg void OnUpdateOpen(CCmdUI *pCmdUI);
	afx_msg void OnFileProperties();
	afx_msg void OnUpdateFileProperties(CCmdUI *pCmdUI);
	afx_msg void OnViewZoomin();
	afx_msg void OnUpdateViewZoomin(CCmdUI *pCmdUI);
	afx_msg void OnViewZoomout();
	afx_msg void OnUpdateViewZoomout(CCmdUI *pCmdUI);
	afx_msg void OnEditSnapshot();
	afx_msg void OnUpdateEditSnapshot(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFrame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateColor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSize(CCmdUI* pCmdUI);
};


