// Preferences.cpp: implementation of the CPreferences class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MNist.h"
#include "Preferences.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPreferences::CPreferences() : m_pMainApp( NULL )
{
	
}

CPreferences::~CPreferences()
{
	
}

void CPreferences::ReadIniFile(CWinApp *pApp)
{
	ASSERT( m_pMainApp==NULL  &&  pApp!=NULL );
	m_pMainApp = pApp;
	
	// confirm that INI file exists, and write a default one from resources if it does not
	
	WIN32_FIND_DATA  fd;
	HANDLE hFile = ::FindFirstFile( m_pMainApp->m_pszProfileName, &fd );
	
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		// file must not exist --  get default ini file from resources and 
		// write it to ..\YourAppName.ini (which is stored in m_pMainApp->m_pszProfileName)
		
		CFile f;
		CFileException fe;
		if ( f.Open( (LPCTSTR) m_pMainApp->m_pszProfileName, 
			CFile::modeCreate | CFile::modeWrite, &fe ) ) // note: didn't specify modeNoTruncate, so length is zero'd
		{
			HINSTANCE hInst = m_pMainApp->m_hInstance;
			HRSRC hINI = ::FindResource(hInst,
				MAKEINTRESOURCE( IDR_TEXT_DEFAULT_INI ),
				_T("TEXT") );
			
			DWORD size = ::SizeofResource(hInst, hINI);
			HGLOBAL hGlobal = ::LoadResource(hInst, hINI);
			LPVOID t = ::LockResource(hGlobal);
			f.Write( t, size );
			f.Close();
			::FreeResource(hGlobal);
		}
		else
		{		
			::MessageBox( NULL,	
				_T("Error encountered while trying to write default INI file\n")
				_T("Hard-coded baseline parameters will be used instead"),
				_T("INI File Creation Failed"),
				MB_OK | MB_ICONWARNING);
		}
		
	}
	
	::FindClose( hFile );
	
#if 0
	// set default values

	m_nMagicTrainingLabels = 0x00000801;
	m_nMagicTrainingImages = 0x00000803;

	m_nItemsTrainingLabels = 60000;
	m_nItemsTrainingImages = 60000;

	m_nMagicTestingLabels = 0x00000801;
	m_nMagicTestingImages = 0x00000803;

	m_nItemsTestingLabels = 10000;
	m_nItemsTestingImages = 10000;

	m_nRowsImages = g_cImageSize;
	m_nColsImages = g_cImageSize;

	m_nMagWindowSize = 5;
	m_nMagWindowMagnification = 8;

	m_dInitialEtaLearningRate = 0.001;
	m_dLearningRateDecay = 0.794328235;  // 0.794328235 = 0.001 down to 0.00001 in 20 epochs 
	m_dMinimumEtaLearningRate = 0.00001;
	m_nAfterEveryNBackprops = 60000;
	m_cNumBackpropThreads = 2;

	m_cNumTestingThreads = 1;
	
	// parameters for controlling distortions of input image

	m_dMaxScaling = 15.0;  // like 20.0 for 20%
	m_dMaxRotation = 15.0;  // like 20.0 for 20 degrees
	m_dElasticSigma = 8.0;  // higher numbers are more smooth and less distorted; Simard uses 4.0
	m_dElasticScaling = 0.5;  // higher numbers amplify the distortions; Simard uses 34 (sic, maybe 0.34 ??)

	// for limiting the step size in backpropagation, since we are using second order
	// "Stochastic Diagonal Levenberg-Marquardt" update algorithm.  See Yann LeCun 1998
	// "Gradient-Based Learning Applied to Document Recognition" at page 41

	m_dMicronLimitParameter = 0.10;  // since we divide by this, update can never be more than 10x current eta
	m_nNumHessianPatterns = 500;  // number of patterns used to calculate the diagonal Hessian

#endif
	// now read values from the ini file

	CString tSection;

	// Neural Network parameters

	tSection = _T( "Neural Network Parameters" );

	Get( tSection, _T( "Initial learning rate (eta)" ),			m_dInitialEtaLearningRate );
	Get( tSection, _T( "Minimum learning rate (eta)" ),			m_dMinimumEtaLearningRate );
	Get( tSection, _T( "Rate of decay for learning rate (eta)" ), m_dLearningRateDecay );
	Get( tSection, _T( "Decay rate is applied after this number of backprops" ), m_nAfterEveryNBackprops );
	Get( tSection, _T( "Number of backprop threads" ),			m_cNumBackpropThreads );
	Get( tSection, _T( "Number of testing threads" ),			m_cNumTestingThreads );
	Get( tSection, _T( "Number of patterns used to calculate Hessian" ),	m_nNumHessianPatterns );
	Get( tSection, _T( "Limiting divisor (micron) for learning rate amplification (like 0.10 for 10x limit)" ),	m_dMicronLimitParameter );


	// Neural Network Viewer parameters

	tSection = _T( "Neural Net Viewer Parameters" );

	Get( tSection, _T( "Size of magnification window" ),			m_nMagWindowSize );
	Get( tSection, _T( "Magnification factor for magnification window" ), m_nMagWindowMagnification );


	// MNIST data collection parameters

	tSection = _T( "MNIST Database Parameters" );

	Get( tSection, _T( "Training images magic number" ),	m_nMagicTrainingImages );
	Get( tSection, _T( "Training images item count" ),		m_nItemsTrainingImages );
	Get( tSection, _T( "Training labels magic number" ),	m_nMagicTrainingLabels );
	Get( tSection, _T( "Training labels item count" ),		m_nItemsTrainingLabels );

	Get( tSection, _T( "Testing images magic number" ),		m_nMagicTestingImages );
	Get( tSection, _T( "Testing images item count" ),		m_nItemsTestingImages );
	Get( tSection, _T( "Testing labels magic number" ),		m_nMagicTestingLabels );
	Get( tSection, _T( "Testing labels item count" ),		m_nItemsTestingLabels );

	// these two are basically ignored

	UINT uiCount = g_cImageSize;
	Get( tSection, _T( "Rows per image" ),			uiCount );
	m_nRowsImages = uiCount;

	uiCount = g_cImageSize;
	Get( tSection, _T( "Columns per image" ),		uiCount );
	m_nColsImages = uiCount;


	// parameters for controlling pattern distortion during backpropagation

	tSection = _T( "Parameters for Controlling Pattern Distortion During Backpropagation" );
	
	Get( tSection, _T( "Maximum scale factor change (percent, like 20.0 for 20%)" ),		m_dMaxScaling );
	Get( tSection, _T( "Maximum rotational change (degrees, like 20.0 for 20 degrees)" ),	m_dMaxRotation );
	Get( tSection, _T( "Sigma for elastic distortions (higher numbers are more smooth and less distorted; Simard uses 4.0)" ), m_dElasticSigma );
	Get( tSection, _T( "Scaling for elastic distortions (higher numbers amplify distortions; Simard uses 0.34)" ), m_dElasticScaling );


}


// overloaded "Get" functions.  Each of these functions should be called with a default
// value already stored in the data type; if the entry exists in the INI file then that value 
// is returned, else the default value is unchanged and it is also stored into the INI file
// for future use
// NOTE: Do not use with the MS data type BOOL.  If used with type BOOL, then the "int" version
//       will be called.  Use the c-standard data type "bool" instead

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, UINT &uiVal)
{
	union { UINT ui;	int ii; } iInput, iRet;

	iInput.ui = uiVal;
	
	iRet.ii = m_pMainApp->GetProfileInt( strSection, strEntry, iInput.ii );

	uiVal = iRet.ui;
}

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, int &iVal)
{
	int iRet = m_pMainApp->GetProfileInt( strSection, strEntry, iVal );

	iVal = iRet;
}

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, float &fVal)
{
	CString strTemp;
	double dVal = (double)fVal;
	strTemp.Format( _T("g"), dVal );

	CString sRet = m_pMainApp->GetProfileString( strSection, strEntry, strTemp );

	dVal = _tcstod( (LPCTSTR)sRet, NULL );

	fVal = (float)dVal;
}

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, double &dVal)
{
	CString strTemp;
	strTemp.Format( _T("g"), dVal );

	CString sRet = m_pMainApp->GetProfileString( strSection, strEntry, strTemp );

	dVal = _tcstod( (LPCTSTR)sRet, NULL );
}

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, LPTSTR pStrVal)
{
	CString sRet = m_pMainApp->GetProfileString( strSection, strEntry, pStrVal );

	_tcscpy( pStrVal, (LPCTSTR)sRet );
}

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, CString &strVal)
{
	CString sRet = m_pMainApp->GetProfileString( strSection, strEntry, strVal );

	strVal = sRet;
}

void CPreferences::Get(LPCTSTR strSection, LPCTSTR strEntry, bool &bVal)
{
	CString strTemp;

	strTemp = ( bVal==false ) ? ( _T( "No" ) ) : ( _T( "Yes" ) );

	CString sRet = m_pMainApp->GetProfileString( strSection, strEntry, strTemp );

	// check for "no", "false" or zero("0")

	sRet.MakeUpper();

	if ( sRet[0] == _T( 'N' ) ||
		 sRet[0] == _T( 'F' ) ||
		 sRet[0] == _T( '0' ) )
	{
		bVal = FALSE;
	}
	else
	{
		bVal = TRUE;
	}

}