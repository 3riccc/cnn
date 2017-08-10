#if !defined(AFX_DLGNEURALNET_H__BE2F91D6_7F72_401F_A01F_51ECBBA9E767__INCLUDED_)
#define AFX_DLGNEURALNET_H__BE2F91D6_7F72_401F_A01F_51ECBBA9E767__INCLUDED_

#include "DlgResizeHelper.h"	// Added by ClassView

#include "MNistDoc.h"
#include "WndGraphicMSE.h"

// diable warning C4284: return type for 'std::deque<double,class std::allocator<double> >::const_iterator::operator ->'
// is 'const double *' (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)

#pragma warning( push )
#pragma warning( disable:4284 )

#include <deque>

using namespace std;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgNeuralNet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgNeuralNet dialog

class CDlgNeuralNet : public CDialog
{
// Construction
public:
	virtual void OnCancel();
	virtual void OnOK();
	DWORD m_dwEpochStartTime;
	UINT m_iEpochsCompleted;
	UINT m_iBackpropsPosted;
	CMNistDoc* m_pDoc;
	DlgResizeHelper m_resizeHelper;
	std::deque< double > m_dRecentMses;
	double m_dMSE;
	UINT m_cMisrecognitions;
	CWndGraphicMSE m_wndGraphicMSE;
	CDlgNeuralNet(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgNeuralNet)
	enum { IDD = IDD_DIALOG_NEURAL_NET };
	CStatic	m_ctlStaticRunningMSE;
	CStatic	m_ctlStaticPatternSequenceNum;
	CEdit	m_ctlEditEpochInformation;
	CStatic	m_ctlStaticEpochsCompleted;
	CProgressCtrl	m_ctlProgressPatternNum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgNeuralNet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgNeuralNet)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonStopBackpropagation();
	afx_msg void OnButtonStartBackpropagation();
	afx_msg LRESULT OnBackpropagationNotification(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#pragma warning( pop )

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGNEURALNET_H__BE2F91D6_7F72_401F_A01F_51ECBBA9E767__INCLUDED_)
