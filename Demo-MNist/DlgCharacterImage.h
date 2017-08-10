#if !defined(AFX_DLGCHARACTERIMAGE_H__5CE8866D_6C85_42BD_90E9_AD63905278C0__INCLUDED_)
#define AFX_DLGCHARACTERIMAGE_H__5CE8866D_6C85_42BD_90E9_AD63905278C0__INCLUDED_

#include "DlgResizeHelper.h"	// Added by ClassView
#include "WndCharImage.h"	// Added by ClassView

#include "MNistDoc.h"
#include "WndNeuronViewer.h"	// Added by ClassView

using namespace std;

#include <vector>

typedef std::vector< double >  VectorDoubles;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCharacterImage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCharacterImage dialog

class CDlgCharacterImage : public CDialog
{
// Construction
public:
	virtual void OnCancel();
	virtual void OnOK();
	void UpdateCharacterImageData( int& iNumImage );
	std::vector< VectorDoubles > m_NeuronOutputs;
	CMNistDoc* m_pDoc;
	CWndCharImage m_wndCharImage;
	CWndNeuronViewer m_wndNeuronViewer;
	DlgResizeHelper m_resizeHelper;
	CDlgCharacterImage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCharacterImage)
	enum { IDD = IDD_DIALOG_CHARACTER_IMAGE };
	CButton	m_ctlRadioWhichSet;
	CButton	m_ctlCheckDistortInputPattern;
	CEdit	m_ctlEditLabelValue;
	CEdit	m_ctlEditImageNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCharacterImage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCharacterImage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonGetImageData();
	afx_msg void OnButtonGetNextImageData();
	afx_msg void OnButtonGetPreviousImageData();
	afx_msg void OnButtonNnCalculate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCHARACTERIMAGE_H__5CE8866D_6C85_42BD_90E9_AD63905278C0__INCLUDED_)
