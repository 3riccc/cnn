#if !defined(AFX_WNDCHARIMAGE_H__63D5AC58_3F10_4828_B6F8_3CDD90A27350__INCLUDED_)
#define AFX_WNDCHARIMAGE_H__63D5AC58_3F10_4828_B6F8_3CDD90A27350__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndCharImage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWndCharImage window

class CWndCharImage : public CWnd
{
// Construction
public:
	CWndCharImage();

	CBitmap m_bmDisplayedBitmap;
	int m_cRows;
	int m_cCols;
	int m_cPixels;
	DWORD* m_pValues;  // actually stores a gbr quad

	inline DWORD& At( DWORD* p, int row, int col )  // zero-based indices, starting at bottom-left
		{ int location = row * m_cCols + col;
		  ASSERT( location>=0 && location<m_cPixels && row<m_cRows && row>=0 && col<m_cCols && col>=0 );
		  return p[ location ];
		}

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndCharImage)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL m_bMagnifierVisible;
	CWnd m_wndMagnifier;
	void BuildBitmapFromGrayValues(unsigned char* pGrayValues );
	virtual ~CWndCharImage();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndCharImage)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDCHARIMAGE_H__63D5AC58_3F10_4828_B6F8_3CDD90A27350__INCLUDED_)
