// DlgNeuralNet.cpp : implementation file
//

#include "stdafx.h"
#include "MNist.h"
#include "DlgNeuralNet.h"
#include "DlgBackpropParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgNeuralNet dialog


CDlgNeuralNet::CDlgNeuralNet(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgNeuralNet::IDD, pParent),
	m_pDoc( NULL )
{
	//{{AFX_DATA_INIT(CDlgNeuralNet)
	//}}AFX_DATA_INIT
}


void CDlgNeuralNet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgNeuralNet)
	DDX_Control(pDX, IDC_STATIC_LABEL_MSE, m_ctlStaticRunningMSE);
	DDX_Control(pDX, IDC_STATIC_LABEL_PATTERN_SEQ_NUM, m_ctlStaticPatternSequenceNum);
	DDX_Control(pDX, IDC_EDIT_EPOCH_INFO, m_ctlEditEpochInformation);
	DDX_Control(pDX, IDC_STATIC_EPOCHS_COMPLETED, m_ctlStaticEpochsCompleted);
	DDX_Control(pDX, IDC_PROGRESS_PATTERN_NUM, m_ctlProgressPatternNum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgNeuralNet, CDialog)
	//{{AFX_MSG_MAP(CDlgNeuralNet)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_STOP_BACKPROP, OnButtonStopBackpropagation)
	ON_BN_CLICKED(IDC_BUTTON_START_BACKPROP, OnButtonStartBackpropagation)
	ON_REGISTERED_MESSAGE( UWM_BACKPROPAGATION_NOTIFICATION, OnBackpropagationNotification )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgNeuralNet message handlers

BOOL CDlgNeuralNet::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ASSERT( m_pDoc != NULL );


	// create the graphic MSE viewer window, using static placeholder from the dialog template
	
	CRect rcPlace;
	CWnd* pPlaceholder = GetDlgItem( IDC_STATIC_GRAPHIC_MSE );
	
	if ( pPlaceholder != NULL )
	{
		pPlaceholder->GetWindowRect( &rcPlace );  // in screen coords
		::MapWindowPoints( NULL, m_hWnd, (POINT*)&rcPlace, 2 );  // map from screen to this window's coords
		
		m_wndGraphicMSE.CreateEx( WS_EX_STATICEDGE,  NULL, _T("GraphicMseViewer"), WS_CHILD|WS_VISIBLE, rcPlace, this, IDC_STATIC_GRAPHIC_MSE );
		
		// close placeholder window since it's no longer needed
		
		pPlaceholder->DestroyWindow();
	}
	
	
	// initialize resize helper
	
	m_resizeHelper.Init( m_hWnd );
	//	m_resizeHelper.Fix( IDC_EDIT1, DlgResizeHelper::kNoHFix /* DlgResizeHelper::kLeft */, DlgResizeHelper::kHeight );
	
	
	
	// ensure that thread-pertinent controls are hidden
	
	m_ctlProgressPatternNum.ShowWindow( SW_HIDE );
	m_ctlStaticPatternSequenceNum.ShowWindow( SW_HIDE );
	
	// initialize the range of the progress control
	
	m_ctlProgressPatternNum.SetRange32( 0, ::GetPreferences().m_nItemsTrainingImages );

	// initialize the recent MSE's

	m_dRecentMses.resize( 200, 0.0 );  // 200 sample running average


	// enlarge the default 32K depth of the edit control (remember to accommodate unicode builds)

	m_ctlEditEpochInformation.SetLimitText( 660000 );  	
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgNeuralNet::OnOK()
{
	// do nothing -- prevent the dialog from closing when user hits the "Enter key	
}

void CDlgNeuralNet::OnCancel()
{
	// do nothing -- prevent the dialog from closing when the user hits the ESC key
}



void CDlgNeuralNet::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	m_resizeHelper.OnSize();	
	
}


void CDlgNeuralNet::OnButtonStartBackpropagation() 
{
	
	CDlgBackpropParameters dlg;

	dlg.m_cNumThreads = ::GetPreferences().m_cNumBackpropThreads;	
	dlg.m_InitialEta = ::GetPreferences().m_dInitialEtaLearningRate;
	dlg.m_MinimumEta = ::GetPreferences().m_dMinimumEtaLearningRate;
	dlg.m_EtaDecay = ::GetPreferences().m_dLearningRateDecay;
	dlg.m_AfterEvery = ::GetPreferences().m_nAfterEveryNBackprops;
	dlg.m_StartingPattern = 0;
	dlg.m_EstimatedCurrentMSE = 0.10;
	dlg.m_bDistortPatterns = TRUE;
	
	double eta = m_pDoc->GetCurrentEta();
	dlg.m_strInitialEtaMessage.Format( _T("Initial Learning Rate eta (currently, eta = %11.8f)"), eta );
	
	UINT curPattern = m_pDoc->GetCurrentTrainingPatternNumber();
	dlg.m_strStartingPatternNum.Format( _T("Starting Pattern Number (currently at %d)"), curPattern );
	
	int iRet = dlg.DoModal();
	
	if ( iRet == IDOK )
	{
		BOOL bRet = m_pDoc->StartBackpropagation( dlg.m_StartingPattern, dlg.m_cNumThreads,
			m_hWnd, dlg.m_InitialEta, dlg.m_MinimumEta, dlg.m_EtaDecay,	dlg.m_AfterEvery, 
			dlg.m_bDistortPatterns, dlg.m_EstimatedCurrentMSE );
		if ( bRet != FALSE )
		{
			m_ctlProgressPatternNum.ShowWindow( SW_SHOW );
			m_ctlStaticPatternSequenceNum.ShowWindow( SW_SHOW );

			m_ctlProgressPatternNum.SetPos( 0 );

			m_iEpochsCompleted = 0;
			m_iBackpropsPosted = 0;
			m_dMSE = 0.0;

			m_cMisrecognitions = 0;

			m_dwEpochStartTime = ::GetTickCount();

			CString str;
			str.Format( _T("%d Epochs completed "), m_iEpochsCompleted );
			m_ctlStaticEpochsCompleted.SetWindowText( str );

			m_wndGraphicMSE.EraseAllPoints();

						
			// write a "starting" message to the info window
			
			CWnd* pWnd = m_ctlEditEpochInformation.SetFocus();
			
			m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
			m_ctlEditEpochInformation.ReplaceSel( _T("Backpropagation started \r\n") );
			m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
			
			if ( pWnd != NULL )
				pWnd->SetFocus();
		}
	}
	
}


void CDlgNeuralNet::OnButtonStopBackpropagation() 
{
	
	m_ctlProgressPatternNum.ShowWindow( SW_HIDE );
	m_ctlStaticPatternSequenceNum.ShowWindow( SW_HIDE );
	
	m_pDoc->StopBackpropagation();
	
	// write a "stopped" message to the info window
	
	CWnd* pWnd = m_ctlEditEpochInformation.SetFocus();
	
	m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
	m_ctlEditEpochInformation.ReplaceSel( _T("\r\nBackpropagation stopped \r\n\r\n") );
	m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
	
	if ( pWnd != NULL )
		pWnd->SetFocus();
}


afx_msg LRESULT CDlgNeuralNet::OnBackpropagationNotification(WPARAM wParam, LPARAM lParam)
{
	CString str;
	double currentMSE;

	if ( wParam == 1 )  
	{
		// lParam contains the number of the current pattern being back-propagated

		UINT pos = (UINT)lParam;
		str.Format( _T("Working on pattern number %d"), pos );
		
		m_ctlProgressPatternNum.SetPos( pos );
		m_ctlStaticPatternSequenceNum.SetWindowText( str );
		
		// check for completion of an epoch
		
		if ( pos == (::GetPreferences().m_nItemsTrainingImages - 1 ) )
		{
			// epoch has been completed.  Display interesting information
			
			m_iEpochsCompleted++;
			str.Format( ((m_iEpochsCompleted==1) ? _T("%d Epoch completed ") : _T("%d Epochs completed")),
				m_iEpochsCompleted );
			m_ctlStaticEpochsCompleted.SetWindowText( str );
			
			// calculate epoch statistics and append them to the end of the edit control
			
			DWORD currentTick = ::GetTickCount();
			double deltaSeconds = (double)( currentTick - m_dwEpochStartTime ) / 1000.0;
			m_dwEpochStartTime = currentTick;
			
			UINT divisor = m_iBackpropsPosted;
			if ( divisor <= 0 ) divisor = 10;  // arbitrary non-zero value
			double epochMSE = m_dMSE / divisor;
			m_dMSE = 0.0;
			m_iBackpropsPosted = 0.0;

			// update doc's estimate of current MSE.  Must use atomic compare-and-exchange, since other 
			// threads are using this value
				
			struct DOUBLE_UNION
			{
				union 
				{
					double dd;
					unsigned __int64 ullong;
				};
			};
			
			DOUBLE_UNION oldValue, newValue;

			oldValue.dd = m_pDoc->m_dEstimatedCurrentMSE;
			newValue.dd = epochMSE;
			while ( oldValue.ullong != _InterlockedCompareExchange64( (unsigned __int64*)( &(m_pDoc->m_dEstimatedCurrentMSE) ), 
					newValue.ullong, oldValue.ullong ) ) 
			{
				// another thread must have modified the MSE.  Obtain its new value, adjust it, and try again
				
				oldValue.dd = m_pDoc->m_dEstimatedCurrentMSE;
				newValue.dd = epochMSE;
			}


			UINT misRecognitions = m_cMisrecognitions;
			m_cMisrecognitions = 0;

			double eta = m_pDoc->GetCurrentEta();
			
			str.Format( _T("Epoch %2d: MSE = %10g\tMis-recognitions = %d\tLearning rate (eta) = %10g\tTime for completion = %.0f seconds \r\n"), 
				m_iEpochsCompleted - 1, epochMSE, misRecognitions, eta, deltaSeconds );
			
			CWnd* pWnd = m_ctlEditEpochInformation.SetFocus();
			
			m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
			m_ctlEditEpochInformation.ReplaceSel( str );
			m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
			
			if ( pWnd != NULL )
				pWnd->SetFocus();
		}
	}
	else if ( wParam == 2 )
	{
		// lParam contains a scaled numerical value indicating the Err_p for this current pattern

		UINT scaled = (UINT)lParam;
		double Err = ((double)(scaled))/2.0e8;  // arbitrary pre-agreed upon scale factor
		Err = Err * Err;  // accommodates the fact that we took the sqrt to improve scalability

		m_dRecentMses.pop_front();
		m_dRecentMses.push_back( Err );

		m_dMSE += Err;  // accumulate for use in displaying epoch statistics
		++m_iBackpropsPosted;

		currentMSE = 0.0;
		for ( int ii=0; ii<m_dRecentMses.size(); ++ii )
		{
			currentMSE += m_dRecentMses[ ii ];
		}

		currentMSE /= m_dRecentMses.size();

		str.Format( _T("Estimate of current MSE (200 sample running average) = %g"), currentMSE );
		m_ctlStaticRunningMSE.SetWindowText( str );


		// add to the graphic MSE viewer every 400 backprops (viewer holds 600 points, so 400x600=240000=4 epochs

		if ( ( m_iBackpropsPosted % 400 ) == 0 )
		{
			m_wndGraphicMSE.AddNewestPoint( currentMSE );
		}

	}
	else if ( wParam == 4 )
	{
		// related to calculation of the Hessian
		// lParam == 1L on commencement
		//        == 2L on an increment (such as every 50)
		//        == 4L on completion

		if ( lParam == 1L )
		{
			str.Format( _T( "Commencing calculation of Hessian" ) );
		}
		else if ( lParam == 2L )
		{
			str.Format( _T( " ." ) );
		}
		else if ( lParam == 4L )
		{
			str.Format( _T( " completed \r\n" ) );
		}

		
		CWnd* pWnd = m_ctlEditEpochInformation.SetFocus();
		
		m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
		m_ctlEditEpochInformation.ReplaceSel( str );
		m_ctlEditEpochInformation.SetSel( INT_MAX, INT_MAX );
		
		if ( pWnd != NULL )
			pWnd->SetFocus();
	}
	else if ( wParam == 8 )
	{
		// this message signifies that a pattern was mis-recognized, so update mis-recognition statistics

		m_cMisrecognitions++;
	}


	
	
	return 0L;
}



