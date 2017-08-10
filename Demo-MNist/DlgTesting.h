#if !defined(AFX_DLGTESTING_H__A84455E6_C51D_4AB0_BFA5_9E34939D4536__INCLUDED_)
#define AFX_DLGTESTING_H__A84455E6_C51D_4AB0_BFA5_9E34939D4536__INCLUDED_

#include "MNistDoc.h"
#include "DlgResizeHelper.h"	// Added by ClassView


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTesting.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgTesting dialog

class CDlgTesting : public CDialog
{
// Construction
public:
	virtual void OnCancel();
	virtual void OnOK();
	UINT m_iWhichImageSet;
	UINT m_iNumErrors;
	DWORD m_dwStarted;
	DlgResizeHelper m_resizeHelper;
	CMNistDoc* m_pDoc;
	CDlgTesting(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgTesting)
	enum { IDD = IDD_DIALOG_TESTING };
	CStatic	m_ctlStaticCurrentlyTesting;
	CProgressCtrl	m_ctlProgressTesting;
	CEdit	m_ctlEditTestResults;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTesting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgTesting)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonStartTesting();
	afx_msg void OnButtonStopTesting();
	afx_msg LRESULT OnTestingNotification(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTESTING_H__A84455E6_C51D_4AB0_BFA5_9E34939D4536__INCLUDED_)
