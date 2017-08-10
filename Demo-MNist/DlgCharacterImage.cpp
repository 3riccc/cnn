// DlgCharacterImage.cpp : implementation file
//

#include "stdafx.h"
#include "MNist.h"
#include "DlgCharacterImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCharacterImage dialog


CDlgCharacterImage::CDlgCharacterImage(CWnd* pParent /*=NULL*/)
: CDialog(CDlgCharacterImage::IDD, pParent),
m_pDoc( NULL )
{
	//{{AFX_DATA_INIT(CDlgCharacterImage)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgCharacterImage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCharacterImage)
	DDX_Control(pDX, IDC_RADIO_TRAINING_SET, m_ctlRadioWhichSet);
	DDX_Control(pDX, IDC_CHECK_PATTERN_DISTORTION, m_ctlCheckDistortInputPattern);
	DDX_Control(pDX, IDC_EDIT_VALUE, m_ctlEditLabelValue);
	DDX_Control(pDX, IDC_EDIT_IMAGE_NUM, m_ctlEditImageNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCharacterImage, CDialog)
//{{AFX_MSG_MAP(CDlgCharacterImage)
ON_WM_SIZE()
ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGetImageData)
ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonGetNextImageData)
ON_BN_CLICKED(IDC_BUTTON_PREVIOUS, OnButtonGetPreviousImageData)
ON_BN_CLICKED(IDC_BUTTON_NN_CALCULATE, OnButtonNnCalculate)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCharacterImage message handlers

BOOL CDlgCharacterImage::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ASSERT( m_pDoc != NULL );
	
	
	
	// create the character image window, using static placeholder from the dialog template,
	// and set its size to the global size of the character image
	
	CRect rcPlace;
	CWnd* pPlaceholder = GetDlgItem( IDC_CHARACTER_IMAGE );
	if ( pPlaceholder != NULL )
	{
		pPlaceholder->GetWindowRect( &rcPlace );  // in screen coords
		::MapWindowPoints( NULL, m_hWnd, (POINT*)&rcPlace, 2 );  // map from screen to this window's coords
		
		rcPlace.right = rcPlace.left + g_cImageSize+2;  // +2 allows for the width of WS_EX_STATIC edge
		rcPlace.bottom = rcPlace.top + g_cImageSize+2;  // +2 allows for the width of WS_EX_STATIC edge
		
		m_wndCharImage.CreateEx( WS_EX_STATICEDGE,  NULL, _T("CharacterImage"), WS_CHILD|WS_VISIBLE, rcPlace, this, IDC_CHARACTER_IMAGE );
		
		// close placeholder window since it's no longer needed
		
		pPlaceholder->DestroyWindow();
	}
	
	
	// create the neuron viewer image window, using static placeholder from the dialog template
	
	pPlaceholder = GetDlgItem( IDC_NEURON_VIEWER );
	if ( pPlaceholder != NULL )
	{
		pPlaceholder->GetWindowRect( &rcPlace );  // in screen coords
		::MapWindowPoints( NULL, m_hWnd, (POINT*)&rcPlace, 2 );  // map from screen to this window's coords
		
		m_wndNeuronViewer.CreateEx( WS_EX_STATICEDGE,  NULL, _T("NeuronViewer"), WS_CHILD|WS_VISIBLE, rcPlace, this, IDC_NEURON_VIEWER );
		
		// close placeholder window since it's no longer needed
		
		pPlaceholder->DestroyWindow();
	}
	
	
	
	// initialize values of edit controls
	
	m_ctlEditImageNumber.SetWindowText( _T("0") );
	m_ctlEditLabelValue.SetWindowText( _T("0") );
	
	
	// initialize state of "Distort input pattern" check box
	
	m_ctlCheckDistortInputPattern.SetCheck( 1 );  // 1 == checked
	
	
	// initialize state of "Training Pattern/Testing Pattern" radio buttons
	
	m_ctlRadioWhichSet.SetCheck( 1 );  // 1 == checked (which implies that testing is un-checked
	
	
	
	// initialize resize helper
	
	m_resizeHelper.Init( m_hWnd );
	m_resizeHelper.Fix( IDC_CHARACTER_IMAGE, DlgResizeHelper::kLeftRight, DlgResizeHelper::kTopBottom );
	
	
	// clear m_NeuronOutputs
	
	m_NeuronOutputs.clear();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgCharacterImage::OnOK()
{
	// ordinarily do nothing, to prevent the dialog from closing when user hits the "Enter key
	// However, check if the user hit "return" after edting the pattern number window
	
	CWnd* pWnd = GetFocus();
	
	if ( pWnd->m_hWnd == m_ctlEditImageNumber.m_hWnd )
	{
		// current focus is the edit control that contains the index for the image number, and
		// user has just hit the "return" button.  We assume that he wants to calculate the neural
		// net, so we set the focus tot he Calculate button and call the corresponding
		// calculate functions
		
/////////////////////////////////////////////////
//////////  CODE REVIEW: There's no need to shift focus away from the edit.  This allows the user to 
		// continuously enter numbers and hit enter, without the need to re-click the edit window
		///////// CWnd* pAnotherWnd = GetDlgItem( IDC_BUTTON_NN_CALCULATE );
		///////// if ( pAnotherWnd != NULL )
		////////// 	pAnotherWnd->SetFocus();

		//////////// no point in SetSel here: the OnButtonNnCalculate function (which calls UpdateCharacterImageData)
		//////////// eventually sends WM_SETTEXT to the edit control, which un-selects all text.  
		//////////// We will SetSel right after the
		//////////// WM_SETTEXT, in the UpdateCharacterImageData function

		///////////// m_ctlEditImageNumber.SetSel( 0, -1 );  // select all text in control

		OnButtonNnCalculate();
	}
	
}


void CDlgCharacterImage::OnCancel()
{
	// do nothing, to override default behavior, which is to close the dialog when the ESC key is pressed
}



void CDlgCharacterImage::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	m_resizeHelper.OnSize();	
}

void CDlgCharacterImage::OnButtonGetImageData() 
{
	// gets image corresponding to value in edit control
	
	CString strNum;
	m_ctlEditImageNumber.GetWindowText( strNum );
	
	int iNum = _ttoi( strNum );
	
	UpdateCharacterImageData( iNum );
	
}

void CDlgCharacterImage::OnButtonGetNextImageData() 
{
	// increments the value of the edit control and gets corresponding image
	
	CString strNum;
	m_ctlEditImageNumber.GetWindowText( strNum );
	
	int iNum = _ttoi( strNum );
	++iNum;
	
	UpdateCharacterImageData( iNum );	
}

void CDlgCharacterImage::OnButtonGetPreviousImageData() 
{
	// decrements the value of the edit control and gets corresponding image
	
	CString strNum;
	m_ctlEditImageNumber.GetWindowText( strNum );
	
	int iNum = _ttoi( strNum );
	--iNum;
	
	UpdateCharacterImageData( iNum );		
}

void CDlgCharacterImage::UpdateCharacterImageData(int& iNumImage)
{
	// updates the viewing window that contains the character's image
	// and adjusts the value of the int iNumImage so that it returns a value
	// that lies inside a valid range of the selected image set (training or testing)
	
	unsigned char grayArray[ g_cImageSize * g_cImageSize ] = {0};
	int label = 0;
	
	// determine whether we are looking for the training patterns or the testing patterns
	
	BOOL bTraining = ( 1 == m_ctlRadioWhichSet.GetCheck() );  // 1 == checked
	
	if ( bTraining != FALSE )
	{
		// the training set has been selected
		// adjust numeric value of pattern so that it lies inside a valid range
		
		if ( iNumImage < 0 ) iNumImage = 0;
		if ( iNumImage >= ::GetPreferences().m_nItemsTrainingImages ) iNumImage = ::GetPreferences().m_nItemsTrainingImages - 1;
		
		// get gray-scale values for character image, and numeric value of its "answer"
		
		m_pDoc->GetTrainingPatternArrayValues( iNumImage, grayArray, &label, TRUE );
		
	}
	else
	{
		// testing set has been selected
		// adjust numeric value of pattern so that it lies inside a valid range
		
		if ( iNumImage < 0 ) iNumImage = 0;
		if ( iNumImage >= ::GetPreferences().m_nItemsTestingImages ) iNumImage = ::GetPreferences().m_nItemsTestingImages - 1;
		
		// get gray-scale values for character image, and numeric value of its "answer"
		
		m_pDoc->GetTestingPatternArrayValues( iNumImage, grayArray, &label, TRUE );
	}
	
	// update appearance of the dialog
	
	CString strNum;
	
	strNum.Format( _T("%i"), label );
	m_ctlEditLabelValue.SetWindowText( strNum );
	
	strNum.Format( _T("%i"), iNumImage );
	m_ctlEditImageNumber.SetWindowText( strNum );
	m_ctlEditImageNumber.SetSel( 0, -1 );  // select all text in control; will be highlighted only if edit also has focus

	
	m_wndCharImage.BuildBitmapFromGrayValues( grayArray );
	m_wndCharImage.Invalidate( );
	
}


void CDlgCharacterImage::OnButtonNnCalculate() 
{
	// runs the current character image through the neural net
	
	// first, get image corresponding to value in edit control
	// note that this code is identical to OnButtonGetImageData,
	// which is deliberate because the user might have changed the content
	// of the image label, but which is somewhat redundant if he didn't
	
	CString strNum;
	m_ctlEditImageNumber.GetWindowText( strNum );
	
	int iNum = _ttoi( strNum );
	
	UpdateCharacterImageData( iNum );
	
	// now get image data, based on whether the training set or the testing set has been 
	// selected (this is the part that's redundant to the above call to UpdateCharacterImageData)
	
	unsigned char grayArray[ g_cImageSize * g_cImageSize ] = {0};
	int label = 0;
	
	BOOL bTraining = ( 1 == m_ctlRadioWhichSet.GetCheck() );  // 1 == checked
	
	if ( bTraining != FALSE )
	{
		// the training set has been selected
		// no need to adjust numeric value of the pattern since UpdateCharacterImageData() has 
		// already done this for us
		
		// get gray-scale values for character image, and numeric value of its "answer"
		
		m_pDoc->GetTrainingPatternArrayValues( iNum, grayArray, &label, TRUE );
		
	}
	else
	{
		// testing set has been selected
		
		// get gray-scale values for character image, and numeric value of its "answer"
		
		m_pDoc->GetTestingPatternArrayValues( iNum, grayArray, &label, TRUE );
	}
	
	// pad the values to 29x29, convert to double precision, and run through the neural net
	// This operation is timed and the result is displayed
	
	DWORD tick = ::GetTickCount();
	
	int ii, jj;
	double inputVector[841];
	
	for ( ii=0; ii<841; ++ii )
	{
		inputVector[ ii ] = 1.0;  // one is white, -one is black
	}
	
	// top row of inputVector is left as zero, left-most column is left as zero 
	
	for ( ii=0; ii<g_cImageSize; ++ii )
	{
		for ( jj=0; jj<g_cImageSize; ++jj )
		{
			inputVector[ 1 + jj + 29*(ii+1) ] = (double)((int)(unsigned char)grayArray[ jj + g_cImageSize*ii ])/128.0 - 1.0;  // one is white, -one is black
		}
	}
	
	// get state of "Distort input pattern check box
	
	BOOL bDistort = ( 1 == m_ctlCheckDistortInputPattern.GetCheck() );  // 1 == checked
	
	double outputVector[10] = {0.0};
	double targetOutputVector[10] = {0.0};
	
	
	// initialize target output vector (i.e., desired values)
	
	for ( ii=0; ii<10; ++ii )
	{
		targetOutputVector[ ii ] = -1.0;
	}
	
	if ( label > 9 ) label = 9;
	if ( label < 0 ) label = 0;
	
	targetOutputVector[ label ] = 1.0;
	
	
	// calculate neural net
	
	m_pDoc->CalculateNeuralNet( inputVector, 841, outputVector, 10, &m_NeuronOutputs, bDistort );
	
	DWORD diff = ::GetTickCount() - tick;
	
	
	// write numerical result (and time taken to get them) to results window
	
	CString strLine, strResult;
	double dTemp, sampleMse = 0.0;
	
	strResult.Format( _T("Results:\n") );
	
	for ( ii=0; ii<10; ii++ )
	{
		strLine.Format( _T(" %2i = %+6.3f \n"), ii, outputVector[ii] );
		strResult += strLine;
		
		dTemp = targetOutputVector[ ii ] - outputVector[ ii ];
		sampleMse += dTemp * dTemp;
		
	}
	
	sampleMse = 0.5 * sampleMse;
	strLine.Format( _T("\nPattern Error\n Ep = %g\n\n%i mSecs"), sampleMse, diff );
	strResult += strLine;
	CWnd* pWnd = GetDlgItem(IDC_STATIC_TIME);
	if ( pWnd != NULL )
		pWnd->SetWindowText( strResult );
	
	
	// update the view of the outputs of the neurons
	
	m_wndNeuronViewer.BuildBitmapFromNeuronOutputs( m_NeuronOutputs );
	m_wndNeuronViewer.Invalidate();
	
	
}
