
// HelloWindowsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HelloWindows.h"
#include "HelloWindowsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHelloWindowsDlg dialog
CHelloWindowsDlg* CHelloWindowsDlg::m_the_dialog = NULL;


CHelloWindowsDlg::CHelloWindowsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HelloWindows_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_the_dialog = this;
	m_color_bytes = 2;
	m_ui_width = m_ui_height = 0;
}

void CHelloWindowsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHelloWindowsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CHelloWindowsDlg message handlers

BOOL CHelloWindowsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	AfxBeginThread(CHelloWindowsDlg::ThreadHelloWindows, NULL);
	AfxBeginThread(CHelloWindowsDlg::ThreadRefreshUI, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHelloWindowsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHelloWindowsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHelloWindowsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CHelloWindowsDlg::ThreadHelloWindows(LPVOID pParam)
{
	startHelloWindows(calloc(1280 * 720, m_the_dialog->m_color_bytes), 1280, 720, m_the_dialog->m_color_bytes);
	return 0;
}

UINT CHelloWindowsDlg::ThreadRefreshUI(LPVOID pParam)
{
	CDC* pDC = m_the_dialog->GetDC();
	while (true)
	{
		Sleep(30);
		m_the_dialog->updateUI(pDC);
	}
}

void CHelloWindowsDlg::updateUI(CDC* pDC)
{
	void* data = NULL;
	CRect rcClient;
	GetClientRect(&rcClient);

	if (rcClient.EqualRect(m_block_rect) && m_ui_width != 0)
	{
		data = getUiOfHelloWindows(NULL, NULL);
		goto RENDER;
	}

	m_block_rect = rcClient;
	data = getUiOfHelloWindows(&m_ui_width, &m_ui_height, true);

	memset(&m_ui_bm_info, 0, sizeof(m_ui_bm_info));
	m_ui_bm_info.bmiHeader.biSize = sizeof(MYBITMAPINFO);
	m_ui_bm_info.bmiHeader.biWidth = m_ui_width;
	m_ui_bm_info.bmiHeader.biHeight = (0 - m_ui_height);//fix upside down. 
	m_ui_bm_info.bmiHeader.biBitCount = m_color_bytes * 8;
	m_ui_bm_info.bmiHeader.biPlanes = 1;
	m_ui_bm_info.bmiHeader.biSizeImage = m_ui_width * m_ui_height * m_color_bytes;

	switch (m_color_bytes)
	{
	case 2:
		m_ui_bm_info.bmiHeader.biCompression = BI_BITFIELDS;
		m_ui_bm_info.biRedMask = 0xF800;
		m_ui_bm_info.biGreenMask = 0x07E0;
		m_ui_bm_info.biBlueMask = 0x001F;
		break;
	case 4:
		m_ui_bm_info.bmiHeader.biCompression = BI_RGB;
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	m_memDC.DeleteDC();
	m_blockBmp.DeleteObject();

	m_memDC.CreateCompatibleDC(pDC);
	m_blockBmp.CreateCompatibleBitmap(pDC, m_block_rect.Width(), m_block_rect.Height());
	m_memDC.SelectObject(&m_blockBmp);
	::SetStretchBltMode(m_memDC, STRETCH_HALFTONE);

RENDER:
	if (!data)
	{
		return;
	}
	register int block_width = m_block_rect.Width();
	register int block_height = m_block_rect.Height();
	::StretchDIBits(m_memDC.m_hDC, 0, 0, block_width, block_height,
		0, 0, m_ui_width, m_ui_height, data, (BITMAPINFO*)&m_ui_bm_info, DIB_RGB_COLORS, SRCCOPY);
	pDC->BitBlt(m_block_rect.left, m_block_rect.top, block_width, block_height, &m_memDC, 0, 0, SRCCOPY);
}

void CHelloWindowsDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint guilitePos = pointMFC2GuiLite(point);
	sendTouch2HelloWindows(guilitePos.x, guilitePos.y, true);
}

void CHelloWindowsDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPoint guilitePos = pointMFC2GuiLite(point);
	sendTouch2HelloWindows(guilitePos.x, guilitePos.y, false);
}

CPoint CHelloWindowsDlg::pointMFC2GuiLite(CPoint point)
{
	if (!m_block_rect.PtInRect(point))
	{
		return CPoint(-1, -1);
	}
	return CPoint(m_ui_width * (point.x - m_block_rect.left) / m_block_rect.Width(), m_ui_height * (point.y - m_block_rect.top) / m_block_rect.Height());
}
