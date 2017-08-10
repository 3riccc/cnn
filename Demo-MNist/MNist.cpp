// MNist.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MNist.h"

#include "SHLWAPI.H"	// for the path functions
#pragma comment( lib, "shlwapi.lib" )

#include "MainFrm.h"
#include "MNistDoc.h"
#include "MNistView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMNistApp

BEGIN_MESSAGE_MAP(CMNistApp, CWinApp)
	//{{AFX_MSG_MAP(CMNistApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMNistApp construction

CMNistApp::CMNistApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMNistApp object

CMNistApp theApp;


//////////////////////////////////////////////////////////////////////////
//
// global function to get a const reference to app's preferences

const CPreferences& GetPreferences()
{
	return theApp.m_Preferences;
}


/////////////////////////////////////////////////////////////////////
//
// SetThreadName -- a function to set the current thread's 8-character name
// so as to be able to distinguish between the threads during debug operations
//
// Usage: SetThreadName (-1, "MainThread");
// Must be called from the thread you're trying to name
// For example SetThreadName(-1, "1st Thread");
// Will truncate name to 8 characters
//
// code obtained from 
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vsdebug/html/vxtsksettingthreadname.asp
//


void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
	
	struct THREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} ;
	
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}



/////////////////////////////////////////////////////////////////////////////
// CMNistApp initialization

BOOL CMNistApp::InitInstance()
{
	// set main thread's name (useful for debugging etc)

	char str[] = "MAIN";  // must use chars, not TCHARs, for SetThreadname function
	SetThreadName( -1, str );

	// Seed the random-number generator with current time so that
    // the numbers will be different every time we run.
    
	srand( (unsigned)time( NULL ) );


	// socket initialization (it turns out that we don't need sockets, but it was easy to include
	// and no harm is done)

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries (same comment, we don't need these)

	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();


	// initialize path name for current module

	m_sModulePath.Empty();	
	::GetModuleFileName( NULL, m_sModulePath.GetBuffer(255), 255 );
	::PathMakePretty( m_sModulePath.GetBuffer(255) );
	::PathRemoveFileSpec( m_sModulePath.GetBuffer(255) );
	m_sModulePath.ReleaseBuffer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
////	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

////	LoadStdProfileSettings();  // Load standard INI file options (including MRU)


	// Use an .INI paradigm to load settings, rather than the registry
	// This makes for cleaner installs/uninstalls

	// free the string allocated by MFC at CWinApp startup

	free((void*)m_pszProfileName);
	
	// Next, change the name of the .INI file.
	// The CWinApp destructor will free the memory.
	
	CString tINI = m_sModulePath + _T("\\MNist.ini");
	
	m_pszProfileName=_tcsdup( tINI );

	// Finally, initialize preferences

	m_Preferences.ReadIniFile( this );



	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMNistDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMNistView));
	pDocTemplate->SetContainerInfo(IDR_CNTR_INPLACE);
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMNistApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMNistApp message handlers

