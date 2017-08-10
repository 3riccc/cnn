#if !defined(AFX_DLGTESTINGPARAMETERS_H__CE4E9B7C_B3AE_49BD_B733_79C10E8C531A__INCLUDED_)
#define AFX_DLGTESTINGPARAMETERS_H__CE4E9B7C_B3AE_49BD_B733_79C10E8C531A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTestingParameters.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgTestingParameters dialog

class CDlgTestingParameters : public CDialog
{
// Construction
public:
	CDlgTestingParameters(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgTestingParameters)
	enum { IDD = IDD_DIALOG_TESTING_PARAMETERS };
	BOOL	m_bDistortPatterns;
	UINT	m_iNumThreads;
	UINT	m_iStartingPattern;
	CString	m_strStartingPatternNum;
	int 	m_iWhichImageSet;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTestingParameters)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgTestingParameters)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTESTINGPARAMETERS_H__CE4E9B7C_B3AE_49BD_B733_79C10E8C531A__INCLUDED_)
