// DlgBackpropParameters.cpp : implementation file
//

#include "stdafx.h"
#include "MNist.h"
#include "DlgBackpropParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBackpropParameters dialog


CDlgBackpropParameters::CDlgBackpropParameters(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgBackpropParameters::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgBackpropParameters)
	m_AfterEvery = 0;
	m_EtaDecay = 0.0;
	m_InitialEta = 0.0;
	m_MinimumEta = 0.0;
	m_strInitialEtaMessage = _T("");
	m_strStartingPatternNum = _T("");
	m_StartingPattern = 0;
	m_cNumThreads = 0;
	m_bDistortPatterns = FALSE;
	m_EstimatedCurrentMSE = 0.0;
	//}}AFX_DATA_INIT
}


void CDlgBackpropParameters::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBackpropParameters)
	DDX_Text(pDX, IDC_EDIT_EVERY_N_BACKPROPS, m_AfterEvery);
	DDV_MinMaxUInt(pDX, m_AfterEvery, 1, 4294967295);
	DDX_Text(pDX, IDC_EDIT_ETA_DECAY, m_EtaDecay);
	DDV_MinMaxDouble(pDX, m_EtaDecay, 0., 1.);
	DDX_Text(pDX, IDC_EDIT_INITIAL_ETA, m_InitialEta);
	DDV_MinMaxDouble(pDX, m_InitialEta, 0., 1.);
	DDX_Text(pDX, IDC_EDIT_MINIMUM_ETA, m_MinimumEta);
	DDV_MinMaxDouble(pDX, m_MinimumEta, 0., 1.);
	DDX_Text(pDX, IDC_STATIC_INITIAL_ETA, m_strInitialEtaMessage);
	DDX_Text(pDX, IDC_STATIC_STARTING_PATTERN_NUMBER, m_strStartingPatternNum);
	DDX_Text(pDX, IDC_EDIT_STARTING_PATTERN_NUM, m_StartingPattern);
	DDX_Text(pDX, IDC_EDIT_NUM_BACKPROP_THREADS, m_cNumThreads);
	DDV_MinMaxUInt(pDX, m_cNumThreads, 1, 1000000000);
	DDX_Check(pDX, IDC_CHECK_DISTORT_PATTERNS, m_bDistortPatterns);
	DDX_Text(pDX, IDC_EDIT_ESTIMATED_CURRENT_MSE, m_EstimatedCurrentMSE);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgBackpropParameters, CDialog)
	//{{AFX_MSG_MAP(CDlgBackpropParameters)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBackpropParameters message handlers


