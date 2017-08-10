#if !defined(AFX_TABBEDCONTROLPANEL_H__5FB6892E_DE24_40F0_9133_110EF8D6E649__INCLUDED_)
#define AFX_TABBEDCONTROLPANEL_H__5FB6892E_DE24_40F0_9133_110EF8D6E649__INCLUDED_

#include "DlgNeuralNet.h"	// Added by ClassView
#include "DlgCharacterImage.h"	// Added by ClassView

#include "MNistDoc.h"
#include "DlgTesting.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabbedControlPanel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabbedControlPanel window

class CTabbedControlPanel : public CTabCtrl
{
// Construction
public:
	CTabbedControlPanel();
		
	HWND m_arrHwnd[10];  // up to ten tabbed pages (arbitrary)
	UINT m_iCurSelTab;
	int m_iNumPages;

	CImageList m_ctlImageList;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabbedControlPanel)
	public:
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	CDlgTesting m_dlgTesting;
	CMNistDoc* m_pDoc;
	CDlgCharacterImage m_dlgCharacterImage;
	CDlgNeuralNet m_dlgNeuralNet;

	int CreateImageList( int iResImages, int cxWidth, int iResMasks=0 );
	virtual ~CTabbedControlPanel();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTabbedControlPanel)
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABBEDCONTROLPANEL_H__5FB6892E_DE24_40F0_9133_110EF8D6E649__INCLUDED_)
