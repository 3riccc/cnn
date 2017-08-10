// WndGraphicMSE.cpp : implementation file
//

#include "stdafx.h"
#include "mnist.h"
#include "WndGraphicMSE.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWndGraphicMSE

CWndGraphicMSE::CWndGraphicMSE()
{
}

CWndGraphicMSE::~CWndGraphicMSE()
{
}


BEGIN_MESSAGE_MAP(CWndGraphicMSE, CWnd)
	//{{AFX_MSG_MAP(CWndGraphicMSE)
	ON_WM_CREATE()
	ON_WM_NCDESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndGraphicMSE message handlers

int CWndGraphicMSE::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	Initialize();
	
	return 0;
}

void CWndGraphicMSE::OnNcDestroy() 
{
	CWnd::OnNcDestroy();
	
	// TODO: Add your message handler code here
	
}


void CWndGraphicMSE::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	// set mapping mode for the dc to anisotropic, so as to allow for re-sizings.
	// All coordinates in the logical space of the window are computed in an imaginary
	// space of 20 wide by 5 down (scaled by 100), and are automatically converted (by GDI) to the device
	// coordinates of the viewport, as defined by this window's client rectangle

	CRect rcClient;
	GetClientRect( &rcClient );

	dc.SetMapMode( MM_ANISOTROPIC );
	dc.SetWindowOrg( 0, 0 );
	dc.SetWindowExt( 2000, 500 );  // logical window is 2000 across x 500 down
	dc.SetViewportOrg( 0, 0);
	dc.SetViewportExt( rcClient.Width(), rcClient.Height() ); 

	// now draw, in stages:
	// - erase client area (probably could be done in OnEraseBkgnd)
	// - draw black graph area
	// - draw the actual graphic points
	// - draw horiz grid lines (dotted, since drawn on top of the graph points)
	// - draw vertical grid lines (dotted, since drawn on top of the graph points)
	// - draw boundary of graphic box
	// - draw horiz numeric labels
	// - draw vertical numeric labels
	// - label the graph as "MSE History"

	// erase the client area

	dc.SetBkMode( TRANSPARENT );  // we will be drawing on a black field, so this is important

	dc.DPtoLP( &rcClient );
	dc.FillSolidRect( &rcClient, ::GetSysColor( COLOR_BTNFACE ) );


	// draw black graph area

	CRect rcTemp( 100, 25, 1801, 401 );
	dc.FillSolidRect( &rcTemp, RGB( 0, 0, 0 ) );


	// graph the points

	DWORD crGraphic = RGB( 0, 255, 0 );  // bright green
	CPen penPoints( PS_SOLID, 1, crGraphic );
	CPen* pOldPen = dc.SelectObject( &penPoints );

	int ii = 0;
	BOOL bMovedYet = FALSE;
	double curVal = -99.0;
	int targetVPos = 0;
	int targetHPos = 0;

	for ( ii=0; ii<(int)m_Points.size(); ++ii )
	{
		curVal = m_Points[ ii ];
		targetVPos = (int)( -625.0 * curVal + 400.0 );  // (400 - 25) / (0-0.6) == -625.0
		targetHPos = (int)( 2.83806 * ii + 100 );  // (1800 - 100)/(599-0) == 2.83806

		if ( bMovedYet == FALSE )
		{
			// we still have not moved to the first legitimate point.  Check if we have a
			// legitimate point (i.e., one that's GTE zero), and if so then move there.

			if ( curVal >= 0.0 )
			{
				bMovedYet = TRUE;
				dc.MoveTo( targetHPos, targetVPos );
			}
		}
		else
		{
			// we have already encountered our first "move to", so issue a line to

			dc.LineTo( targetHPos, targetVPos );
		}
	}



	// draw horizontal grid lines (4 of them)

	DWORD crGrid = RGB( 32, 104, 32 );	// similar to forestgreen (whose exact value is RGB( 34, 139, 34 )); COLORREF value is 0x00206820; web value is #206820
	
	CPen penGrid( PS_DOT, 0, crGrid );
	dc.SelectObject( &penGrid );  // returns a pointer to penPoints, which we ignore

	CPoint ptStart( 100, 25 );
	CPoint ptEnd( 1801, 25 );  // 1801 instead of 1800, since LineTo draws "up to but not including the final point"

	for ( ii=0; ii<4; ++ii )
	{
		dc.MoveTo( ptStart );
		dc.LineTo( ptEnd );
		ptStart.Offset( 0, 125 );
		ptEnd.Offset( 0, 125 );
	}


	// draw vertical grid lines (5 of them)

	ptStart = CPoint( 100, 25 );
	ptEnd = CPoint( 100, 401 );  // must move one past, since LineTo draws up to, but not including, a point

	for ( ii=0; ii<5; ++ii )
	{
		dc.MoveTo( ptStart );
		dc.LineTo( ptEnd );
		ptStart.Offset( 425, 0 );
		ptEnd.Offset( 425, 0 );
	}


	// draw boundary of graphic box

	CPen penBoundary( PS_SOLID, 0, crGrid );  // same color as grid, but solid, not dotted
	dc.SelectObject( &penBoundary );  // returns a pointer to penGrid, which we ignore

	ptStart = CPoint( 100, 25 );  // top 
	ptEnd = CPoint( 1801, 25 );
	dc.MoveTo( ptStart );
	dc.LineTo( ptEnd );

	ptStart = CPoint( 100, 400 );  // bottom 
	ptEnd = CPoint( 1801, 400 );
	dc.MoveTo( ptStart );
	dc.LineTo( ptEnd );

	ptStart = CPoint( 100, 25 );  // left 
	ptEnd = CPoint( 100, 401 );
	dc.MoveTo( ptStart );
	dc.LineTo( ptEnd );

	ptStart = CPoint( 1800, 25 );  // right 
	ptEnd = CPoint( 1800, 401 );
	dc.MoveTo( ptStart );
	dc.LineTo( ptEnd );


	// draw horizontal numeric labels (5 of them)

	DWORD crText = RGB( 0, 0, 0 );
	dc.SetTextColor( crText );

	LOGFONT lf = {0};
	lf.lfHeight = 95;  // logical units, so fill up the height of the rect to 95%
	lf.lfWidth = 38;  // 5 characters max in space of 200 wide rectangle, so "50" should have worked ??
	lf.lfWeight = FW_LIGHT;  // FW_REGULAR is too heavy for a font this small
	lf.lfPitchAndFamily = FF_SWISS;
	_stprintf( lf.lfFaceName, _T("Arial") );

	CFont font;
	font.CreateFontIndirect( &lf );

	CFont* pOldFont = dc.SelectObject( &font );

	TCHAR hValues[][10] = {
		_T( "-240k" ),
		_T( "-180k" ),
		_T( "-120k" ),
		_T( "-60k" ),
		_T( "0" )
    };

	rcTemp = CRect( 0, 400, 200, 499 );

	for ( ii=0; ii<5; ++ii )
	{
		dc.DrawText( hValues[ ii ], -1, &rcTemp, DT_BOTTOM | DT_SINGLELINE | DT_CENTER | DT_VCENTER );
		rcTemp.OffsetRect( 425, 0 );
	}


	// draw vertical numeric labels (4 of them)

	TCHAR vValues[][10] = {
		_T( ".60" ),
		_T( ".40" ),
		_T( ".20" ),
		_T( ".00" )
    };

	rcTemp = CRect( 1800, 0, 1999, 100 );

	for ( ii=0; ii<4; ++ii )
	{
		dc.DrawText( vValues[ ii ], -1, &rcTemp, DT_BOTTOM | DT_SINGLELINE | DT_CENTER | DT_VCENTER );
		rcTemp.OffsetRect( 0, 125 );
	}


	// label the graph as "MSE History" at the upper left-hand side

	dc.SetTextColor( RGB( 38, 132, 38 ) );	// similar to forestgreen (whose exact value is RGB( 34, 139, 34 )); COLORREF value is 0x00268426; web value is #268426

	rcTemp = CRect( 125, 50, 610, 150 );

	dc.DrawText( _T("MSE History"), -1, &rcTemp, DT_BOTTOM | DT_SINGLELINE | DT_CENTER | DT_VCENTER );


	// clean up dc

	dc.SelectObject( pOldPen );
	dc.SelectObject( pOldFont );








}


void CWndGraphicMSE::EraseAllPoints()
{
	Initialize();
}

void CWndGraphicMSE::Initialize()
{
	// initialize m_Points.  This variable holds 600 points, one MSE value per every 400 patterns,
	// for a graph which encompasses the last 240000 patterns (4 epochs)

	m_Points.clear();
	m_Points.resize( 600, -99.0 );  // a negative value signifies a value that should not be graphed
}

void CWndGraphicMSE::AddNewestPoint( double newest )
{
	// pop (and lose) the first value, and push the newest value at the end.  Then invalidate
	// so as to re-draw the window

	m_Points.pop_front();
	m_Points.push_back( newest );

	Invalidate();
}

