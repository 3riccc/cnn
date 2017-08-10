// WndCharImage.cpp : implementation file
//

#include "stdafx.h"
#include "MNist.h"
#include "WndCharImage.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWndCharImage

CWndCharImage::CWndCharImage() : m_bMagnifierVisible( FALSE )
{
}

CWndCharImage::~CWndCharImage()
{
}


BEGIN_MESSAGE_MAP(CWndCharImage, CWnd)
	//{{AFX_MSG_MAP(CWndCharImage)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE( WM_MOUSELEAVE, OnMouseLeave )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndCharImage message handlers

int CWndCharImage::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	// create the magnifier window, and make certain it's hidden

	BOOL bRet = m_wndMagnifier.CreateEx( NULL, AfxRegisterWndClass( CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW ),
		_T("CharacterMagnifier"), WS_CHILD, CRect(0,0,0,0), ::AfxGetMainWnd(), 0x1345 );

	ASSERT( bRet != FALSE );

	m_wndMagnifier.ShowWindow( SW_HIDE );

	
	// allocate memory for the DIB bitmap values, and create default for DDB bitmap

	CRect rcClient;

	GetClientRect( rcClient );

	m_cRows = rcClient.Height();
	m_cCols = rcClient.Width();

	ASSERT( m_cRows == g_cImageSize && m_cCols == g_cImageSize );

	m_cPixels = m_cRows * m_cCols;
	
	m_pValues = new COLORREF[ m_cPixels ];

	int ii;

	for ( ii=0; ii<m_cPixels; ++ii )
	{
		m_pValues[ii] = RGB_TO_BGRQUAD(ii*255/m_cPixels,ii*255/m_cPixels,ii*255/m_cPixels);
	}


	// create device dependent bitmap and store for future OnPaints

	CClientDC dc(this);

	m_bmDisplayedBitmap.CreateCompatibleBitmap( &dc, m_cCols, m_cRows );

	BITMAPINFO bmInfo;
	BITMAPINFOHEADER& bmInfoHeader = bmInfo.bmiHeader;

	::memset( &bmInfo, 0, sizeof(BITMAPINFO) );
	bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfoHeader.biWidth = m_cCols;
	bmInfoHeader.biHeight = -m_cRows;  // positive number for bottom-up DIB, negative for top-down
	bmInfoHeader.biPlanes = 1;
	bmInfoHeader.biBitCount = 32;
	bmInfoHeader.biCompression = BI_RGB;
	bmInfoHeader.biSizeImage = 0;
	bmInfoHeader.biXPelsPerMeter = 100;  // arbitrary value
	bmInfoHeader.biYPelsPerMeter = 100;  // arbitrary
	bmInfoHeader.biClrUsed = 0;
	bmInfoHeader.biClrImportant = 0;

	int iRet = ::SetDIBits( (HDC)dc, (HBITMAP)m_bmDisplayedBitmap, 0, m_cRows, (LPVOID)m_pValues, &bmInfo, DIB_RGB_COLORS );

	return 0;
}

void CWndCharImage::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here

	delete[] m_pValues;
	m_bmDisplayedBitmap.DeleteObject();

	
}

void CWndCharImage::BuildBitmapFromGrayValues(unsigned char *pGrayValues)
{
	ASSERT( ::IsWindow( this->m_hWnd ) );

	int ii, gray;

	for ( ii=0; ii<m_cPixels; ++ii )
	{
		gray = pGrayValues[ ii ];
		m_pValues[ii] = RGB_TO_BGRQUAD( gray, gray, gray );
	}

	// create device dependent bitmap and store for future OnPaints

	CClientDC dc(this);

	BITMAPINFO bmInfo;
	BITMAPINFOHEADER& bmInfoHeader = bmInfo.bmiHeader;

	::memset( &bmInfo, 0, sizeof(BITMAPINFO) );
	bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfoHeader.biWidth = m_cCols;
	bmInfoHeader.biHeight = -m_cRows;  // positive number for bottom-up DIB, negative for top-down
	bmInfoHeader.biPlanes = 1;
	bmInfoHeader.biBitCount = 32;
	bmInfoHeader.biCompression = BI_RGB;
	bmInfoHeader.biSizeImage = 0;
	bmInfoHeader.biXPelsPerMeter = 100;  // arbitrary value
	bmInfoHeader.biYPelsPerMeter = 100;  // arbitrary
	bmInfoHeader.biClrUsed = 0;
	bmInfoHeader.biClrImportant = 0;

	int iRet = ::SetDIBits( (HDC)dc, (HBITMAP)m_bmDisplayedBitmap, 0, m_cRows, (LPVOID)m_pValues, &bmInfo, DIB_RGB_COLORS );

}

void CWndCharImage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here

	// draw out the pre-stored DDB bitmap

	CDC memDC;
	memDC.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = memDC.SelectObject( &m_bmDisplayedBitmap );

	dc.BitBlt( 0, 0, m_cCols, m_cRows, &memDC, 0, 0, SRCCOPY );

	memDC.SelectObject( pOldBitmap );
	memDC.DeleteDC();
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CWndCharImage::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd::OnMouseMove(nFlags, point);

	// blt magnified image into m_wndMagnifier, and re-position and show it

	CRect rc;
	GetClientRect( rc );
	ClientToScreen( rc );
	::AfxGetMainWnd()->ScreenToClient( rc );

	int nSize = ::GetPreferences().m_nMagWindowSize * g_cImageSize;

	rc.OffsetRect( 5, -5 - nSize );
	rc.right = rc.left + nSize;
	rc.bottom = rc.top + nSize;

	m_wndMagnifier.SetWindowPos( &wndTopMost, rc.left, rc.top, nSize, nSize, 
		SWP_SHOWWINDOW | SWP_NOZORDER );
		
	CClientDC dc( &m_wndMagnifier );

	CDC memDC;
	memDC.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = memDC.SelectObject( &m_bmDisplayedBitmap );

	int divisor = ::GetPreferences().m_nMagWindowMagnification;
	if ( divisor < 1 ) divisor = 1;
	int delta = nSize/divisor;
	
	dc.StretchBlt( 0, 0, nSize, nSize, &memDC, point.x-delta/2, point.y-delta/2, delta, delta, SRCCOPY );

	memDC.SelectObject( pOldBitmap );

	// track mouse for mouse-leave message
	// note the complementary handler for OnMouseLeave based on _TrackMouseEvent

	TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof( tme );
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		
		_TrackMouseEvent( &tme );
	
}


afx_msg LRESULT CWndCharImage::OnMouseLeave(WPARAM, LPARAM)
{
	// hides magnification window as mouse leaves the window area

	m_wndMagnifier.ShowWindow( SW_HIDE );

	return (0L);
}
