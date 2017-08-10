#if !defined(AFX_WNDNEURONVIEWER_H__27EBD600_E125_472D_A8C0_0869EE1B3618__INCLUDED_)
#define AFX_WNDNEURONVIEWER_H__27EBD600_E125_472D_A8C0_0869EE1B3618__INCLUDED_

#include <vector>

using namespace std;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndNeuronViewer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWndNeuronViewer window

class CWndNeuronViewer : public CWnd
{
// Construction
public:
	CWndNeuronViewer();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndNeuronViewer)
	//}}AFX_VIRTUAL

// Implementation
public:

	CBitmap m_bmDisplayedBitmap;
	DWORD* m_pValues;  // actually stores a gbr quad
	int m_cRows;
	int m_cCols;
	int m_cPixels;
	inline DWORD& At( DWORD* p, int row, int col )  // zero-based indices, starting at bottom-left
		{ int location = row * m_cCols + col;
		  ASSERT( location>=0 && location<m_cPixels && row<m_cRows && row>=0 && col<m_cCols && col>=0 );
		  return p[ location ];
		}

	CWnd m_wndMagnifier;
	virtual ~CWndNeuronViewer();

	void BuildBitmapFromNeuronOutputs( std::vector< std::vector< double > >& neuronOutputs );
	void DrawOutputBox( UINT left, UINT top, UINT clientWidth, UINT clientHeight, std::vector< double >::iterator& it );
	void DrawOutputBox( UINT left, UINT top, UINT clientWidth, UINT clientHeight, DWORD* pArray, int count );


	// Generated message map functions
protected:
	//{{AFX_MSG(CWndNeuronViewer)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDNEURONVIEWER_H__27EBD600_E125_472D_A8C0_0869EE1B3618__INCLUDED_)
