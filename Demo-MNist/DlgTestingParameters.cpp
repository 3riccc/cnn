// DlgTestingParameters.cpp : implementation file
//

#include "stdafx.h"
#include "mnist.h"
#include "DlgTestingParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTestingParameters dialog


CDlgTestingParameters::CDlgTestingParameters(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTestingParameters::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTestingParameters)
	m_bDistortPatterns = FALSE;
	m_iNumThreads = 0;
	m_iStartingPattern = 0;
	m_strStartingPatternNum = _T("");
	m_iWhichImageSet = -1;
	//}}AFX_DATA_INIT
}


void CDlgTestingParameters::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTestingParameters)
	DDX_Check(pDX, IDC_CHECK_DISTORT_PATTERNS, m_bDistortPatterns);
	DDX_Text(pDX, IDC_EDIT_NUM_TESTING_THREADS, m_iNumThreads);
	DDV_MinMaxUInt(pDX, m_iNumThreads, 0, 50);
	DDX_Text(pDX, IDC_EDIT_STARTING_PATTERN_NUM, m_iStartingPattern);
	DDV_MinMaxUInt(pDX, m_iStartingPattern, 0, 9999);
	DDX_Text(pDX, IDC_STATIC_STARTING_PATTERN_NUMBER, m_strStartingPatternNum);
	DDX_Radio(pDX, IDC_RADIO_TRAINING_SET, m_iWhichImageSet);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTestingParameters, CDialog)
	//{{AFX_MSG_MAP(CDlgTestingParameters)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTestingParameters message handlers
