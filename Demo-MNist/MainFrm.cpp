// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MNist.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
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
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
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

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


			
	// Add "About..." menu item to system menu.
	
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUT_SYS_MENU & 0xFFF0) == IDM_ABOUT_SYS_MENU);
	ASSERT(IDM_ABOUT_SYS_MENU < 0xF000);
	
	CMenu* pSysMenu = this->GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUT_SYS_MENU, _T("About MNist ...") );
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

/////////
	// omit the "doc title" from the caption bar 

/////	cs.style &= ~FWS_ADDTOTITLE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL APIENTRY CMainFrame::AboutBoxDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Basic dialog procedure used to display the About... dialog box
	// The procedure simply closes when the OK button or esc is entered

    switch ( message ) 
	{
	case WM_INITDIALOG:   
		return TRUE;
		
	case WM_COMMAND:  
		switch ( wParam ) 
		{           
		case IDOK:       
			EndDialog( hwndDlg, TRUE );      
			return TRUE;      
			
		case IDCANCEL:      
			EndDialog( hwndDlg, FALSE );  
			return TRUE;      
		}           
		break;    
	}    
	return FALSE; 
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	// display "About" dialog, when selected from the system menu

	if ((nID & 0xFFF0) == IDM_ABOUT_SYS_MENU)
	{
		::DialogBox( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDD_ABOUTBOX ),
			GetSafeHwnd(), AboutBoxDialogProc );
	}
	else
	{
		CFrameWnd::OnSysCommand(nID, lParam);
	}	

}


