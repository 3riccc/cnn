// TabbedControlPanel.cpp : implementation file
//

#include "stdafx.h"
#include "MNist.h"
#include "TabbedControlPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabbedControlPanel

CTabbedControlPanel::CTabbedControlPanel() : m_pDoc( NULL )
{
}

CTabbedControlPanel::~CTabbedControlPanel()
{
}


BEGIN_MESSAGE_MAP(CTabbedControlPanel, CTabCtrl)
	//{{AFX_MSG_MAP(CTabbedControlPanel)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabbedControlPanel message handlers

int CTabbedControlPanel::CreateImageList(int iResImages, int cxWidth, int iResMasks /* =0 */ )
{
	// use this function to load bitmaps with greater than 16 colors
	// you must specify a black-and-white (monochrome) mask or your bitmaps will
	// appear as rectangles.  In the mask, "white" = transparent areas where the background shows,
	// and "black" = areas where the bitmaps show

	if ( m_ctlImageList.m_hImageList != NULL )
		::ImageList_Destroy( m_ctlImageList.Detach() );

	HBITMAP hBMColor = (HBITMAP) ::LoadImage( ::AfxGetInstanceHandle(), MAKEINTRESOURCE( iResImages ),
								IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	
	HBITMAP hBMMask = (HBITMAP) ::LoadImage( ::AfxGetInstanceHandle(), MAKEINTRESOURCE( iResMasks ),
								IMAGE_BITMAP, 0, 0, LR_MONOCHROME );

	BITMAP bm;
	::GetObject( hBMColor, sizeof( bm ), &bm );

	ASSERT( bm.bmWidth%cxWidth == 0 );	// the resource's width should be an integral multiple of the icon's width

	m_ctlImageList.Create( cxWidth, bm.bmHeight, ILC_COLOR32|ILC_MASK, 1, 1 );

	int iRet = ::ImageList_Add( m_ctlImageList.GetSafeHandle(), hBMColor, hBMMask );

	::DeleteObject( hBMColor );
	::DeleteObject( hBMMask );

///	int ccx, ccy;
///	::ImageList_GetIconSize( m_ctlImageList.GetSafeHandle(), &ccx, &ccy );
///	m_ImageSize = CSize( ccx, ccy );

	return ( iRet != -1 ) ? ( m_ctlImageList.GetImageCount() ) : ( 0 ) ;

}

void CTabbedControlPanel::PreSubclassWindow() 
{
	CTabCtrl::PreSubclassWindow();

	// modify style to include WS_CLIPCHILDREN

/////////	ModifyStyle( 0, WS_CLIPCHILDREN, 0 );

	// it turns out that we do not want this WS_CLIPCHILDREN style here.  Unless
	// ALL child windows in the chain on down have this style, maybe even including the parent 
	// dialog to this tab control, then NONE of the windows should have this style.  Turning it 
	// on here makes the currently-displayed child page disappear under certain conditions, 
	// like opening a large neural net file


	// image list has more than 16 colors (actually it's 24 bit)
	
	CreateImageList( IDB_BITMAP_TAB_CONTROL_ICONS, 17, 0 );
	SetImageList( &m_ctlImageList );


	// insert tab labels

	int ii = 0;

	InsertItem( ii, _T("Character Images"), ii++ );
	InsertItem( ii, _T("Neural Net"), ii++ );
	InsertItem( ii, _T("Testing"), ii++ );


	// create dialog pages

	// set document pointers

	ASSERT( m_pDoc != NULL );

	m_dlgCharacterImage.m_pDoc = m_pDoc;
	m_dlgNeuralNet.m_pDoc = m_pDoc;
	m_dlgTesting.m_pDoc = m_pDoc;

	// create windows windows

	CRect rc;
	GetWindowRect( &rc );
	AdjustRect( FALSE, &rc );
	ScreenToClient( &rc );
	
	m_dlgCharacterImage.Create( IDD_DIALOG_CHARACTER_IMAGE, this );
	m_dlgNeuralNet.Create( IDD_DIALOG_NEURAL_NET, this );
	m_dlgTesting.Create( IDD_DIALOG_TESTING, this );

	
	m_dlgCharacterImage.MoveWindow( &rc, FALSE );
	m_dlgNeuralNet.MoveWindow( &rc, FALSE );
	m_dlgTesting.MoveWindow( &rc, FALSE );


	ii = 0;
	m_arrHwnd[ii++] = m_dlgCharacterImage.m_hWnd;
	m_arrHwnd[ii++] = m_dlgNeuralNet.m_hWnd;
	m_arrHwnd[ii++] = m_dlgTesting.m_hWnd;

	m_iNumPages = ii;

	// show default window and hide others

	int defaultShow = 0;

	SetCurSel( defaultShow );
	m_iCurSelTab = defaultShow;

	for ( ii=0; ii<m_iNumPages; ++ii )
	{
		::ShowWindow( m_arrHwnd[ ii ], ( ii==defaultShow ? SW_SHOW : SW_HIDE ) );
	}


}

void CTabbedControlPanel::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int ii = GetCurSel();
	
	::ShowWindow( m_arrHwnd[ m_iCurSelTab ], SW_HIDE );
	::ShowWindow( m_arrHwnd[ ii ], SW_SHOW );
	m_iCurSelTab = ii;
	
	*pResult = 0;
}

void CTabbedControlPanel::OnSize(UINT nType, int cx, int cy) 
{
	CTabCtrl::OnSize(nType, cx, cy);
	
	CRect rc;
	GetWindowRect( &rc );
	AdjustRect( FALSE, &rc );
	ScreenToClient( &rc );	
	
	for ( int ii=0; ii<m_iNumPages; ++ii )
	{
		::MoveWindow( m_arrHwnd[ ii ], rc.left, rc.top, rc.Width(), rc.Height(), TRUE );
	}
}




