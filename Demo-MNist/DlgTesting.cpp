// DlgTesting.cpp : implementation file
//

#include "stdafx.h"
#include "mnist.h"
#include "DlgTesting.h"

#include "DlgTestingParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTesting dialog


CDlgTesting::CDlgTesting(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTesting::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTesting)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgTesting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTesting)
	DDX_Control(pDX, IDC_STATIC_CURRENT_PATTERN_NUM, m_ctlStaticCurrentlyTesting);
	DDX_Control(pDX, IDC_PROGRESS_TESTING, m_ctlProgressTesting);
	DDX_Control(pDX, IDC_EDIT_TEST_RESULTS, m_ctlEditTestResults);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTesting, CDialog)
	//{{AFX_MSG_MAP(CDlgTesting)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_START_TESTING, OnButtonStartTesting)
	ON_BN_CLICKED(IDC_BUTTON_STOP_TESTING, OnButtonStopTesting)
	ON_REGISTERED_MESSAGE( UWM_TESTING_NOTIFICATION, OnTestingNotification )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTesting message handlers

BOOL CDlgTesting::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ASSERT( m_pDoc != NULL );

	// initialize resize helper
	
	m_resizeHelper.Init( m_hWnd );

	
	// ensure that thread-pertinent controls are hidden
	
	m_ctlStaticCurrentlyTesting.ShowWindow( SW_HIDE );
	m_ctlProgressTesting.ShowWindow( SW_HIDE );

	
	// initialize the range of the progress control
	
	m_ctlProgressTesting.SetRange32( 0, ::GetPreferences().m_nItemsTestingImages );
	m_ctlProgressTesting.SetPos( 0 );


	// enlarge the default 32K depth of the edit control (remember to accommodate unicode builds)

	m_ctlEditTestResults.SetLimitText( 660000 );  // at 33 chars per line, this should accommodate 10000 unicode lines, i.e., the size of the entire testing set

	


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CDlgTesting::OnOK()
{
	// do nothing -- prevent the dialog from closing when user hits the "Enter key
}

void CDlgTesting::OnCancel()
{
	// do nothing -- prevent the dialog from closing when the user hits the ESC key
}


void CDlgTesting::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	m_resizeHelper.OnSize();	

}

void CDlgTesting::OnButtonStartTesting() 
{
	// initiates a testing session, to determine how well the neural net performs
	// on test data (as opposed to training data)

	CDlgTestingParameters dlg;

	dlg.m_iNumThreads = ::GetPreferences().m_cNumTestingThreads;
	dlg.m_bDistortPatterns = FALSE;
	dlg.m_iStartingPattern = 0;
	dlg.m_strStartingPatternNum.Format( _T("Starting Pattern Number (currently at %d)"), 
		m_pDoc->GetNextTestingPatternNumber() );
	dlg.m_iWhichImageSet = 1;  // default value of "1" is the "testing" set; "0" is the training set

	int iRet = dlg.DoModal();

	if ( iRet == IDOK )
	{
		BOOL bRet = m_pDoc->StartTesting( dlg.m_iStartingPattern, dlg.m_iNumThreads, m_hWnd, 
			dlg.m_bDistortPatterns, dlg.m_iWhichImageSet );
		
		if ( bRet != FALSE )
		{
			m_ctlStaticCurrentlyTesting.ShowWindow( SW_SHOW );
			m_ctlProgressTesting.ShowWindow( SW_SHOW );
			
			m_ctlProgressTesting.SetPos( 0 );
			m_dwStarted = ::GetTickCount();
			m_iNumErrors = 0;
			m_iWhichImageSet = dlg.m_iWhichImageSet;  // "1" is the "testing" set; "0" is the training set

			UINT iRange = (m_iWhichImageSet==0) ? (::GetPreferences().m_nItemsTrainingImages) : (::GetPreferences().m_nItemsTestingImages) ;  // "1" is the "testing" set; "0" is the training set
			m_ctlProgressTesting.SetRange32( 0, iRange );

			
			// write a "starting" message to the info window

			CString str;
			str.Format( _T("Testing started \r\n%s set selected\r\n\r\nList of incorrectly recognized patterns:\r\n"),
				( (m_iWhichImageSet==0) ? _T("Training") : _T("Testing") ) );
			
			CWnd* pWnd = m_ctlEditTestResults.SetFocus();
			
			m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
			m_ctlEditTestResults.ReplaceSel( str );
			m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
			
			if ( pWnd != NULL )
				pWnd->SetFocus();
		}
	}
}

void CDlgTesting::OnButtonStopTesting() 
{
	m_ctlStaticCurrentlyTesting.ShowWindow( SW_HIDE );
	m_ctlProgressTesting.ShowWindow( SW_HIDE );

	m_pDoc->StopTesting();

	DWORD current = ::GetTickCount();
	double elapsed = (double)( current - m_dwStarted ) / 1000.0;
	CString str;

	str.Format( _T("\r\nTesting stopped \r\nErrors found = %d \r\nElapsed Time = %.0f seconds \r\n\r\n"), 
		m_iNumErrors, elapsed );

	CWnd* pWnd = m_ctlEditTestResults.SetFocus();
	
	m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
	m_ctlEditTestResults.ReplaceSel( str );
	m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
	
	if ( pWnd != NULL )
		pWnd->SetFocus();
	
}

afx_msg LRESULT CDlgTesting::OnTestingNotification(WPARAM wParam, LPARAM lParam)
{	
	CString str;

	if ( wParam == 1 )
	{
		// lParam contains the number of the current pattern being tested

		UINT pos = (UINT)lParam;
		str.Format( _T("Currently testing pattern number %d"), pos );
		
		m_ctlProgressTesting.SetPos( pos );
		m_ctlStaticCurrentlyTesting.SetWindowText( str );

		// check if all testing patterns have been completed

		UINT iDone = (m_iWhichImageSet==0) ? (::GetPreferences().m_nItemsTrainingImages) : (::GetPreferences().m_nItemsTestingImages) ;  // "1" is the "testing" set; "0" is the training set
		if ( pos == ( iDone - 1 ) )
		{
			// testing has been completed
			// Post a message to simulate a button-click on "Stop Testing"
			// We post the message (rather than send it) so as to get closer to the end of
			// the message queue, bearing in mind that the testing threads are continuously
			// posting messages here

			PostMessage( WM_COMMAND, MAKEWPARAM( /*low*/ (WORD)( 0xFFFF & IDC_BUTTON_STOP_TESTING ),
				/*high*/ (WORD)( 0xFFFF & BN_CLICKED ) ) );
		}
	}
	else if ( wParam == 2 )
	{
		// this message is posted when a tested pattern is recognized incorrectly.
		// lParam contains a coded bit pattern, as follows:
		//
		//  0          1          2         3
		//  0123456 7890123 456789012345678901
		// |  act  |  tar  |    pattern num   |
		//
		// where act == actual output of the neural net, and tar == target
		// this gives 2^7 = 128 possible outputs (only 10 are needed here... future expansion??)
		// and 2^18 = 262144 possible pattern numbers ( only 10000 are needed here )

		UINT code = (UINT)lParam;

		UINT actual     = ( code & 0xFE000000 ) >> 25;
		UINT target     = ( code & 0x01FC0000 ) >> 18;
		UINT patternNum = ( code & 0x0003FFFF );

		++m_iNumErrors;

		str.Format( _T("%3d : Pattern %3d\tActual Value = %3d\tRecognized Value = %3d \r\n"), 
			m_iNumErrors, patternNum, target, actual );

		CWnd* pWnd = m_ctlEditTestResults.SetFocus();
		
		m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
		m_ctlEditTestResults.ReplaceSel( str );
		m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
		
		if ( pWnd != NULL )
			pWnd->SetFocus();
	}
	else if ( wParam == 4)
	{
		// lParam contains a scaled numerical value that codes the overall MSE Err of all
		// the tested patterns

		UINT scaled = (UINT)lParam;
		double dTotalMSE = ((double)(scaled))/2.0e8;  // arbitrary pre-agreed upon scale factor
		dTotalMSE = dTotalMSE * dTotalMSE;  // accommodates the fact that we took the sqrt to improve scalability

		CString str;

		str.Format( _T("\r\nTotal MSE for all patterns = %g \r\n"), dTotalMSE );

		CWnd* pWnd = m_ctlEditTestResults.SetFocus();
		
		m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
		m_ctlEditTestResults.ReplaceSel( str );
		m_ctlEditTestResults.SetSel( INT_MAX, INT_MAX );
		
		if ( pWnd != NULL )
			pWnd->SetFocus();

	}


	return 0L;
}



