#pragma once


// MediaProperty dialog

class MediaProperty : public CDialog
{
	DECLARE_DYNAMIC(MediaProperty)

public:
	MediaProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~MediaProperty();

// Dialog Data
	enum { IDD = IDD_MEDIAPROPERTY };
	//{{AFX_DATA(CMediaProperty)
		// image byte format
	CString m_szFormat;
	SIZE	m_sizeVideo;
	int		m_nFPS;
    int     m_nHeaderSize;
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
protected:

	// Generated message map functions
	//{{AFX_MSG(CMediaProperty)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
