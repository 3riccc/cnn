// MNistView.h : interface of the CMNistView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MNISTVIEW_H__0EAB31AC_CDA9_4FAA_9C3C_C90A3CF4AAA1__INCLUDED_)
#define AFX_MNISTVIEW_H__0EAB31AC_CDA9_4FAA_9C3C_C90A3CF4AAA1__INCLUDED_

#include "DlgResizeHelper.h"	// Added by ClassView
#include "TabbedControlPanel.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMNistCntrItem;

class CMNistView : public CFormView
{
protected: // create from serialization only
	CMNistView();
	DECLARE_DYNCREATE(CMNistView)

public:
	//{{AFX_DATA(CMNistView)
	enum { IDD = IDD_MNIST_FORM };
	CTabbedControlPanel	m_ctlTabbedControlPanel;
	//}}AFX_DATA

// Attributes
public:
	CMNistDoc* GetDocument();
	// m_pSelection holds the selection to the current CMNistCntrItem.
	// For many applications, such a member variable isn't adequate to
	//  represent a selection, such as a multiple selection or a selection
	//  of objects that are not CMNistCntrItem objects.  This selection
	//  mechanism is provided just to help you get started.

	// TODO: replace this selection mechanism with one appropriate to your app.
	CMNistCntrItem* m_pSelection;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMNistView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL IsSelected(const CObject* pDocItem) const;// Container support
	//}}AFX_VIRTUAL

// Implementation
public:
	DlgResizeHelper m_resizeHelper;
	virtual ~CMNistView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMNistView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnInsertObject();
	afx_msg void OnCancelEditCntr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MNistView.cpp
inline CMNistDoc* CMNistView::GetDocument()
   { return (CMNistDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MNISTVIEW_H__0EAB31AC_CDA9_4FAA_9C3C_C90A3CF4AAA1__INCLUDED_)
