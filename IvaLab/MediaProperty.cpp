// MediaProperty.cpp : implementation file
//

#include "stdafx.h"
#include "IvaLab.h"
#include "MediaProperty.h"


// MediaProperty dialog

IMPLEMENT_DYNAMIC(MediaProperty, CDialog)

MediaProperty::MediaProperty(CWnd* pParent /*=NULL*/)
	: CDialog(MediaProperty::IDD, pParent)
	, m_szFormat(_T(""))
{
	//{{AFX_DATA_INIT(CMediaProperty)
	m_sizeVideo.cx = 640;
	m_sizeVideo.cy = 480;
	m_nFPS = 15;
    m_nHeaderSize = 0;
	//}}AFX_DATA_INIT
}

MediaProperty::~MediaProperty()
{
}

void MediaProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMediaProperty)
	DDX_Text(pDX, IDC_EDIT1, m_szFormat);
	DDX_Text(pDX, IDC_EDIT2, m_sizeVideo.cx);
	DDX_Text(pDX, IDC_EDIT3, m_sizeVideo.cy);
	DDX_Text(pDX, IDC_EDIT4, m_nFPS);
	DDX_Text(pDX, IDC_EDIT5, m_nHeaderSize);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(MediaProperty, CDialog)
END_MESSAGE_MAP()


// MediaProperty message handlers
BOOL MediaProperty::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_szFormat = "YUV420 plan";

	m_sizeVideo.cx = AfxGetApp()->GetProfileInt("MediaProperty", "Width",640);
	m_sizeVideo.cy = AfxGetApp()->GetProfileInt("MediaProperty", "Height",480);
	m_nFPS = AfxGetApp()->GetProfileInt("MediaProperty", "FPS",15);
	m_nHeaderSize = AfxGetApp()->GetProfileInt("MediaProperty", "Header",0);
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MediaProperty::OnOK() 
{
	UpdateData(TRUE);
	AfxGetApp()->WriteProfileInt("MediaProperty", "Width",m_sizeVideo.cx);
	AfxGetApp()->WriteProfileInt("MediaProperty", "Height",m_sizeVideo.cy);
	AfxGetApp()->WriteProfileInt("MediaProperty", "FPS",m_nFPS);
	AfxGetApp()->WriteProfileInt("MediaProperty", "Header",m_nHeaderSize);

	CDialog::OnOK();
}
