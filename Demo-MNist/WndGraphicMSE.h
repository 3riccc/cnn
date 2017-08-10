#if !defined(AFX_WNDGRAPHICMSE_H__68A13782_32AC_4A66_B46A_3D3475E7CA82__INCLUDED_)
#define AFX_WNDGRAPHICMSE_H__68A13782_32AC_4A66_B46A_3D3475E7CA82__INCLUDED_


// diable warning C4284: return type for 'std::deque<double,class std::allocator<double> >::const_iterator::operator ->'
// is 'const double *' (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)

#pragma warning( push )
#pragma warning( disable:4284 )

#include <deque>

using namespace std;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndGraphicMSE.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWndGraphicMSE window

class CWndGraphicMSE : public CWnd
{
// Construction
public:
	CWndGraphicMSE();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndGraphicMSE)
	//}}AFX_VIRTUAL

// Implementation
public:
	void AddNewestPoint( double newest );
	void EraseAllPoints();
	virtual ~CWndGraphicMSE();

	// Generated message map functions
protected:
	void Initialize();
	std::deque< double >  m_Points;  // holds 200 points, one MSE value per every 100 patterns
	//{{AFX_MSG(CWndGraphicMSE)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


#pragma warning( pop )


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_WNDGRAPHICMSE_H__68A13782_32AC_4A66_B46A_3D3475E7CA82__INCLUDED_)
