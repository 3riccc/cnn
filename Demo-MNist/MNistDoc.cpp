// MNistDoc.cpp : implementation of the CMNistDoc class
//

#include "stdafx.h"
#include "MNist.h"

#include "MNistDoc.h"
#include "CntrItem.h"

extern CMNistApp theApp;

UINT CMNistDoc::m_iBackpropThreadIdentifier = 0;  // static member used by threads to identify themselves
UINT CMNistDoc::m_iTestingThreadIdentifier = 0;  

#include "SHLWAPI.H"	// for the path functions
#pragma comment( lib, "shlwapi.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMNistDoc

IMPLEMENT_DYNCREATE(CMNistDoc, COleDocument)

BEGIN_MESSAGE_MAP(CMNistDoc, COleDocument)
//{{AFX_MSG_MAP(CMNistDoc)
ON_BN_CLICKED(IDC_BUTTON_OPEN_MNIST_FILES, OnButtonOpenMnistFiles)
ON_BN_CLICKED(IDC_BUTTON_CLOSE_MNIST_FILES, OnButtonCloseMnistFiles)

//}}AFX_MSG_MAP
// Enable default OLE container implementation
ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, COleDocument::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMNistDoc construction/destruction

CMNistDoc::CMNistDoc()
{
	// Use OLE compound files
	EnableCompoundFile();
	
	// TODO: add one-time construction code here
	
	m_bFilesOpen = FALSE;
	m_bBackpropThreadAbortFlag = FALSE;
	m_bBackpropThreadsAreRunning = FALSE;
	m_cBackprops = 0;
	m_nAfterEveryNBackprops = 1;
	
	m_bTestingThreadsAreRunning = FALSE;
	m_bTestingThreadAbortFlag = FALSE;
	
	m_iNextTestingPattern = 0;
	m_iNextTrainingPattern = 0;
	
	::InitializeCriticalSection( &m_csTrainingPatterns );
	::InitializeCriticalSection( &m_csTestingPatterns );
	
	m_utxNeuralNet = ::CreateMutex( NULL, FALSE, NULL );  // anonymous mutex which is unowned initially
	
	ASSERT( m_utxNeuralNet != NULL );
	
	
	// allocate memory to store the distortion maps
	
	m_cCols = 29;
	m_cRows = 29;
	
	m_cCount = m_cCols * m_cRows;
	
	m_DispH = new double[ m_cCount ];
	m_DispV = new double[ m_cCount ];
	
	
	// create a gaussian kernel, which is constant, for use in generating elastic distortions
	
	int iiMid = GAUSSIAN_FIELD_SIZE/2;  // GAUSSIAN_FIELD_SIZE is strictly odd
	
	double twoSigmaSquared = 2.0 * (::GetPreferences().m_dElasticSigma) * (::GetPreferences().m_dElasticSigma);
	twoSigmaSquared = 1.0 /  twoSigmaSquared;
	double twoPiSigma = 1.0 / (::GetPreferences().m_dElasticSigma) * sqrt( 2.0 * 3.1415926535897932384626433832795 );
	
	for ( int col=0; col<GAUSSIAN_FIELD_SIZE; ++col )
	{
		for ( int row=0; row<GAUSSIAN_FIELD_SIZE; ++row )
		{
			m_GaussianKernel[ row ][ col ] = twoPiSigma * 
				( exp(- ( ((row-iiMid)*(row-iiMid) + (col-iiMid)*(col-iiMid)) * twoSigmaSquared ) ) );
		}
	}
	
	
	
}

CMNistDoc::~CMNistDoc()
{
	if ( m_bFilesOpen != FALSE )
	{
		CloseMnistFiles();
	}
	
	::DeleteCriticalSection( &m_csTrainingPatterns );
	::DeleteCriticalSection( &m_csTestingPatterns );
	
	::CloseHandle( m_utxNeuralNet );
	
	// delete memory of the distortion maps, allocated in constructor
	
	delete[] m_DispH;
	delete[] m_DispV;
	
	
}

//DEL BOOL CMNistDoc::OnOpenDocument(LPCTSTR lpszPathName) 
//DEL {
//DEL 	if (!COleDocument::OnOpenDocument(lpszPathName))
//DEL 		return FALSE;
//DEL 
//DEL 	
//DEL 	// TODO: Add your specialized creation code here
//DEL 	
//DEL 	return TRUE;
//DEL }

void CMNistDoc::DeleteContents() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	COleDocument::DeleteContents();
	
	m_NN.Initialize();
	
}




BOOL CMNistDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;
	
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	
	// grab the mutex for the neural network
	
	CAutoMutex tlo( m_utxNeuralNet );
	
	// initialize and build the neural net
	
	NeuralNetwork& NN = m_NN;  // for easier nomenclature
	NN.Initialize();
	
	NNLayer* pLayer;
	
	int ii, jj, kk;
	int icNeurons = 0;
	int icWeights = 0;
	double initWeight;
	CString label;
	
	// layer zero, the input layer.
	// Create neurons: exactly the same number of neurons as the input
	// vector of 29x29=841 pixels, and no weights/connections
	
	pLayer = new NNLayer( _T("Layer00") );
	NN.m_Layers.push_back( pLayer );
	
	for ( ii=0; ii<841; ++ii )
	{
		label.Format( _T("Layer00_Neuron%04d_Num%06d"), ii, icNeurons );
		pLayer->m_Neurons.push_back( new NNNeuron( (LPCTSTR)label ) );
		icNeurons++;
	}
	
#define UNIFORM_PLUS_MINUS_ONE ( (double)(2.0 * rand())/RAND_MAX - 1.0 )
	
	// layer one:
	// This layer is a convolutional layer that has 6 feature maps.  Each feature 
	// map is 13x13, and each unit in the feature maps is a 5x5 convolutional kernel
	// of the input layer.
	// So, there are 13x13x6 = 1014 neurons, (5x5+1)x6 = 156 weights
	
	pLayer = new NNLayer( _T("Layer01"), pLayer );
	NN.m_Layers.push_back( pLayer );
	
	for ( ii=0; ii<1014; ++ii )
	{
		label.Format( _T("Layer01_Neuron%04d_Num%06d"), ii, icNeurons );
		pLayer->m_Neurons.push_back( new NNNeuron( (LPCTSTR)label ) );
		icNeurons++;
	}
	
	for ( ii=0; ii<156; ++ii )
	{
		label.Format( _T("Layer01_Weight%04d_Num%06d"), ii, icWeights );
		initWeight = 0.05 * UNIFORM_PLUS_MINUS_ONE;
		pLayer->m_Weights.push_back( new NNWeight( (LPCTSTR)label, initWeight ) );
	}
	
	// interconnections with previous layer: this is difficult
	// The previous layer is a top-down bitmap image that has been padded to size 29x29
	// Each neuron in this layer is connected to a 5x5 kernel in its feature map, which 
	// is also a top-down bitmap of size 13x13.  We move the kernel by TWO pixels, i.e., we
	// skip every other pixel in the input image
	
	int kernelTemplate[25] = {
		0,  1,  2,  3,  4,
		29, 30, 31, 32, 33,
		58, 59, 60, 61, 62,
		87, 88, 89, 90, 91,
		116,117,118,119,120 };
		
	int iNumWeight;
		
	int fm;
		
	for ( fm=0; fm<6; ++fm)
	{
		for ( ii=0; ii<13; ++ii )
		{
			for ( jj=0; jj<13; ++jj )
			{
				iNumWeight = fm * 26;  // 26 is the number of weights per feature map
				NNNeuron& n = *( pLayer->m_Neurons[ jj + ii*13 + fm*169 ] );
				
				n.AddConnection( ULONG_MAX, iNumWeight++ );  // bias weight
				
				for ( kk=0; kk<25; ++kk )
				{
					// note: max val of index == 840, corresponding to 841 neurons in prev layer
					n.AddConnection( 2*jj + 58*ii + kernelTemplate[kk], iNumWeight++ );
				}
			}
		}
	}
	
	
	// layer two:
	// This layer is a convolutional layer that has 50 feature maps.  Each feature 
	// map is 5x5, and each unit in the feature maps is a 5x5 convolutional kernel
	// of corresponding areas of all 6 of the previous layers, each of which is a 13x13 feature map
	// So, there are 5x5x50 = 1250 neurons, (5x5+1)x6x50 = 7800 weights
	
	pLayer = new NNLayer( _T("Layer02"), pLayer );
	NN.m_Layers.push_back( pLayer );
	
	for ( ii=0; ii<1250; ++ii )
	{
		label.Format( _T("Layer02_Neuron%04d_Num%06d"), ii, icNeurons );
		pLayer->m_Neurons.push_back( new NNNeuron( (LPCTSTR)label ) );
		icNeurons++;
	}
	
	for ( ii=0; ii<7800; ++ii )
	{
		label.Format( _T("Layer02_Weight%04d_Num%06d"), ii, icWeights );
		initWeight = 0.05 * UNIFORM_PLUS_MINUS_ONE;
		pLayer->m_Weights.push_back( new NNWeight( (LPCTSTR)label, initWeight ) );
	}
	
	// Interconnections with previous layer: this is difficult
	// Each feature map in the previous layer is a top-down bitmap image whose size
	// is 13x13, and there are 6 such feature maps.  Each neuron in one 5x5 feature map of this 
	// layer is connected to a 5x5 kernel positioned correspondingly in all 6 parent
	// feature maps, and there are individual weights for the six different 5x5 kernels.  As
	// before, we move the kernel by TWO pixels, i.e., we
	// skip every other pixel in the input image.  The result is 50 different 5x5 top-down bitmap
	// feature maps
	
	int kernelTemplate2[25] = {
		0,  1,  2,  3,  4,
		13, 14, 15, 16, 17, 
		26, 27, 28, 29, 30,
		39, 40, 41, 42, 43, 
		52, 53, 54, 55, 56   };
		
		
	for ( fm=0; fm<50; ++fm)
	{
		for ( ii=0; ii<5; ++ii )
		{
			for ( jj=0; jj<5; ++jj )
			{
				iNumWeight = fm * 26;  // 26 is the number of weights per feature map
				NNNeuron& n = *( pLayer->m_Neurons[ jj + ii*5 + fm*25 ] );
				
				n.AddConnection( ULONG_MAX, iNumWeight++ );  // bias weight
				
				for ( kk=0; kk<25; ++kk )
				{
					// note: max val of index == 1013, corresponding to 1014 neurons in prev layer
					n.AddConnection(       2*jj + 26*ii + kernelTemplate2[kk], iNumWeight++ );
					n.AddConnection( 169 + 2*jj + 26*ii + kernelTemplate2[kk], iNumWeight++ );
					n.AddConnection( 338 + 2*jj + 26*ii + kernelTemplate2[kk], iNumWeight++ );
					n.AddConnection( 507 + 2*jj + 26*ii + kernelTemplate2[kk], iNumWeight++ );
					n.AddConnection( 676 + 2*jj + 26*ii + kernelTemplate2[kk], iNumWeight++ );
					n.AddConnection( 845 + 2*jj + 26*ii + kernelTemplate2[kk], iNumWeight++ );
				}
			}
		}
	}
			
	
	// layer three:
	// This layer is a fully-connected layer with 100 units.  Since it is fully-connected,
	// each of the 100 neurons in the layer is connected to all 1250 neurons in
	// the previous layer.
	// So, there are 100 neurons and 100*(1250+1)=125100 weights
	
	pLayer = new NNLayer( _T("Layer03"), pLayer );
	NN.m_Layers.push_back( pLayer );
	
	for ( ii=0; ii<100; ++ii )
	{
		label.Format( _T("Layer03_Neuron%04d_Num%06d"), ii, icNeurons );
		pLayer->m_Neurons.push_back( new NNNeuron( (LPCTSTR)label ) );
		icNeurons++;
	}
	
	for ( ii=0; ii<125100; ++ii )
	{
		label.Format( _T("Layer03_Weight%04d_Num%06d"), ii, icWeights );
		initWeight = 0.05 * UNIFORM_PLUS_MINUS_ONE;
		pLayer->m_Weights.push_back( new NNWeight( (LPCTSTR)label, initWeight ) );
	}
	
	// Interconnections with previous layer: fully-connected
	
	iNumWeight = 0;  // weights are not shared in this layer
	
	for ( fm=0; fm<100; ++fm )
	{
		NNNeuron& n = *( pLayer->m_Neurons[ fm ] );
		n.AddConnection( ULONG_MAX, iNumWeight++ );  // bias weight
		
		for ( ii=0; ii<1250; ++ii )
		{
			n.AddConnection( ii, iNumWeight++ );
		}
	}
	
			
			
	// layer four, the final (output) layer:
	// This layer is a fully-connected layer with 10 units.  Since it is fully-connected,
	// each of the 10 neurons in the layer is connected to all 100 neurons in
	// the previous layer.
	// So, there are 10 neurons and 10*(100+1)=1010 weights
	
	pLayer = new NNLayer( _T("Layer04"), pLayer );
	NN.m_Layers.push_back( pLayer );
	
	for ( ii=0; ii<10; ++ii )
	{
		label.Format( _T("Layer04_Neuron%04d_Num%06d"), ii, icNeurons );
		pLayer->m_Neurons.push_back( new NNNeuron( (LPCTSTR)label ) );
		icNeurons++;
	}
	
	for ( ii=0; ii<1010; ++ii )
	{
		label.Format( _T("Layer04_Weight%04d_Num%06d"), ii, icWeights );
		initWeight = 0.05 * UNIFORM_PLUS_MINUS_ONE;
		pLayer->m_Weights.push_back( new NNWeight( (LPCTSTR)label, initWeight ) );
	}
	
	// Interconnections with previous layer: fully-connected
	
	iNumWeight = 0;  // weights are not shared in this layer
	
	for ( fm=0; fm<10; ++fm )
	{
		NNNeuron& n = *( pLayer->m_Neurons[ fm ] );
		n.AddConnection( ULONG_MAX, iNumWeight++ );  // bias weight
		
		for ( ii=0; ii<100; ++ii )
		{
			n.AddConnection( ii, iNumWeight++ );
		}
	}
	
	
	SetModifiedFlag( TRUE );
	
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CMNistDoc serialization

void CMNistDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
		
	}
	else
	{
		// TODO: add loading code here
		
	}
	
	{
		// grab the mutex for the neural network in a local scope, and then serialize
		
		CAutoMutex tlo( m_utxNeuralNet );
		
		m_NN.Serialize( ar );
		
	}
	
	// Calling the base class COleDocument enables serialization
	//  of the container document's COleClientItem objects.
	COleDocument::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CMNistDoc diagnostics

#ifdef _DEBUG
void CMNistDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CMNistDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMNistDoc commands

void CMNistDoc::OnButtonOpenMnistFiles() 
{
	// opens the MNIST image data file and the label file
	
	struct FILEBEGINNING
	{
		union
		{
			char raw0[ 4 ];
			int nMagic;
		};
		union
		{
			char raw1[ 4 ];
			UINT nItems;
		};
		union
		{
			char raw2[ 4 ];
			int nRows;
		};
		union
		{
			char raw3[ 4 ];
			int nCols;
		};
	};
	
	if ( m_bFilesOpen != FALSE )
	{
		::MessageBox( NULL, _T("Files already open"), _T("Files already open"), MB_ICONEXCLAMATION );
		return;
	}
	
	CWaitCursor wc;
	
	// we need to open four files: (1) training images, (2) training labels, (3) testing images, (4) testing labels
	
	
	// (1) Training images
	
	CFileDialog fd( TRUE );  // constructs an "open" dialog; we are happy with the remaining defaults
	
	fd.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0\0");
	fd.m_ofn.lpstrTitle = _T("TRAINING Images");
	fd.m_ofn.lpstrInitialDir = theApp.m_sModulePath;
	
	if ( fd.DoModal() != IDOK ) return;
	
	// store chosen directory so that subsequent open-file dialogs can use it
	
	CString newPath = fd.GetPathName();
	::PathMakePretty( newPath.GetBuffer(255) );
	::PathRemoveFileSpec( newPath.GetBuffer(255) );
	newPath.ReleaseBuffer();
	
	// open the file
	
	CFileException fe;
	
	if ( m_fileTrainingImages.Open( (LPCTSTR)fd.GetPathName(), CFile::modeRead | CFile::shareDenyNone, &fe ) != 0 )
	{
		// opened successfully
		// check for expected magic number and for expected dimensionality
		
		FILEBEGINNING fileBegin;
		m_fileTrainingImages.Read( fileBegin.raw0, sizeof( fileBegin ) );
		
		// convert endian-ness (using winsock functions for convenience)
		
		fileBegin.nMagic = ::ntohl( fileBegin.nMagic );
		fileBegin.nItems = ::ntohl( fileBegin.nItems );
		fileBegin.nRows = ::ntohl( fileBegin.nRows );
		fileBegin.nCols = ::ntohl( fileBegin.nCols );
		
		// check against expected values
		
		if ( fileBegin.nMagic != ::GetPreferences().m_nMagicTrainingImages ||
			fileBegin.nItems != ::GetPreferences().m_nItemsTrainingImages ||
			fileBegin.nRows != ::GetPreferences().m_nRowsImages ||
			fileBegin.nCols != ::GetPreferences().m_nColsImages )
		{
			// file is not configured as expected
			CString msg;
			msg.Format( _T("File header for training images contains unexpected values as follows\n")
				_T("Magic number: got %i, expected %i \n")
				_T("Number of items: got %i, expected %i \n")
				_T("Number of rows: got %i, expected %i \n")
				_T("Number of columns: got %i, expected %i \n")
				_T("\nCheck integrity of file for training images and/or adjust the expected values in the INI file"),
				fileBegin.nMagic, ::GetPreferences().m_nMagicTrainingImages,
				fileBegin.nItems, ::GetPreferences().m_nItemsTrainingImages,
				fileBegin.nRows, ::GetPreferences().m_nRowsImages,
				fileBegin.nCols, ::GetPreferences().m_nColsImages );
			
			::MessageBox( NULL, msg, _T("Unexpected Format"), MB_OK|MB_ICONEXCLAMATION );
			
			// close all files
			
			m_fileTrainingImages.Close();
			
			return;
		}
	}
	else
	{
		// could not open image file
		
		TRACE(_T("%s(%i): Could not open file for training images \n"), __FILE__,__LINE__);
		
		CString msg;
		TCHAR szCause[255] = {0};
		
		fe.GetErrorMessage(szCause, 254);
		
		msg.Format( _T("Image file for training images could not be opened\n")
			_T("File name: %s\nReason: %s\nError code: %d"), 
			fe.m_strFileName, szCause, fe.m_cause );
		
		::MessageBox( NULL, msg, _T("Failed to open file for training images"), MB_OK|MB_ICONEXCLAMATION );
		
		return;
	}
	
	
	
	// (2) Training labels
	
	CFileDialog fd2( TRUE );  // constructs an "open" dialog; we are happy with the remaining defaults
	
	fd2.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0\0");
	fd2.m_ofn.lpstrTitle = _T("TRAINING Labels");
	fd2.m_ofn.lpstrInitialDir = newPath;
	
	if ( fd2.DoModal() != IDOK ) 
	{
		// close the images file too
		
		m_fileTrainingImages.Close();
		return;
	}
	
	
	// open the file
	
	if ( m_fileTrainingLabels.Open( (LPCTSTR)fd2.GetPathName(), CFile::modeRead | CFile::shareDenyNone, &fe ) != 0 )
	{
		// opened successfully
		// check for expected magic number and for expected dimensionality
		
		FILEBEGINNING fileBegin;
		m_fileTrainingLabels.Read( fileBegin.raw0, sizeof( fileBegin ) );
		
		// convert endian-ness (using winsock functions for convenience)
		
		fileBegin.nMagic = ::ntohl( fileBegin.nMagic );
		fileBegin.nItems = ::ntohl( fileBegin.nItems );
		
		// check against expected values
		
		if ( fileBegin.nMagic != ::GetPreferences().m_nMagicTrainingLabels ||
			fileBegin.nItems != ::GetPreferences().m_nItemsTrainingLabels )
		{
			// file is not configured as expected
			CString msg;
			msg.Format( _T("File header for training labels contains unexpected values as follows\n")
				_T("Magic number: got %i, expected %i \n")
				_T("Number of items: got %i, expected %i \n")
				_T("\nCheck integrity of file for training labels and/or adjust the expected values in the INI file"),
				fileBegin.nMagic, ::GetPreferences().m_nMagicTrainingLabels,
				fileBegin.nItems, ::GetPreferences().m_nItemsTrainingLabels );
			
			::MessageBox( NULL, msg, _T("Unexpected Format"), MB_OK|MB_ICONEXCLAMATION );
			
			// close all files
			
			m_fileTrainingImages.Close();
			m_fileTrainingLabels.Close();
			
			return;
		}
	}
	else
	{
		// could not open label file
		
		TRACE(_T("%s(%i): Could not open file for training labels \n"), __FILE__,__LINE__);
		
		CString msg;
		TCHAR szCause[255] = {0};
		
		fe.GetErrorMessage(szCause, 254);
		
		msg.Format( _T("Label file for training labels could not be opened\n")
			_T("File name: %s\nReason: %s\nError code: %d"), 
			fe.m_strFileName, szCause, fe.m_cause );
		
		::MessageBox( NULL, msg, _T("Failed to open file for training labels"), MB_OK|MB_ICONEXCLAMATION );
		
		
		// close the already-opened files too
		
		m_fileTrainingImages.Close();
		
		return;
	}
	
	
	
	// (3) Testing images
	
	CFileDialog fd3( TRUE );  // constructs an "open" dialog; we are happy with the remaining defaults
	
	fd3.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0\0");
	fd3.m_ofn.lpstrTitle = _T("TESTING Images");
	fd3.m_ofn.lpstrInitialDir = newPath;
	
	if ( fd3.DoModal() != IDOK ) 
	{
		// close the already-opened files too
		
		m_fileTrainingLabels.Close();
		m_fileTrainingImages.Close();
		
		return;
	}
	
	
	// open the file
	
	if ( m_fileTestingImages.Open( (LPCTSTR)fd3.GetPathName(), CFile::modeRead | CFile::shareDenyNone, &fe ) != 0 )
	{
		// opened successfully
		// check for expected magic number and for expected dimensionality
		
		FILEBEGINNING fileBegin;
		m_fileTestingImages.Read( fileBegin.raw0, sizeof( fileBegin ) );
		
		// convert endian-ness (using winsock functions for convenience)
		
		fileBegin.nMagic = ::ntohl( fileBegin.nMagic );
		fileBegin.nItems = ::ntohl( fileBegin.nItems );
		fileBegin.nRows = ::ntohl( fileBegin.nRows );
		fileBegin.nCols = ::ntohl( fileBegin.nCols );
		
		// check against expected values
		
		if ( fileBegin.nMagic != ::GetPreferences().m_nMagicTestingImages ||
			fileBegin.nItems != ::GetPreferences().m_nItemsTestingImages ||
			fileBegin.nRows != ::GetPreferences().m_nRowsImages ||
			fileBegin.nCols != ::GetPreferences().m_nColsImages )
		{
			// file is not configured as expected
			CString msg;
			msg.Format( _T("File header for testing images contains unexpected values as follows\n")
				_T("Magic number: got %i, expected %i \n")
				_T("Number of items: got %i, expected %i \n")
				_T("Number of rows: got %i, expected %i \n")
				_T("Number of columns: got %i, expected %i \n")
				_T("\nCheck integrity of file for testing images and/or adjust the expected values in the INI file"),
				fileBegin.nMagic, ::GetPreferences().m_nMagicTestingImages,
				fileBegin.nItems, ::GetPreferences().m_nItemsTestingImages,
				fileBegin.nRows, ::GetPreferences().m_nRowsImages,
				fileBegin.nCols, ::GetPreferences().m_nColsImages );
			
			::MessageBox( NULL, msg, _T("Unexpected Format"), MB_OK|MB_ICONEXCLAMATION );
			
			
			// close all files
			
			m_fileTestingImages.Close();
			m_fileTrainingLabels.Close();
			m_fileTrainingImages.Close();
			
			return;
		}
	}
	else
	{
		// could not open image file
		
		TRACE(_T("%s(%i): Could not open file for testing images \n"), __FILE__,__LINE__);
		
		CString msg;
		TCHAR szCause[255] = {0};
		
		fe.GetErrorMessage(szCause, 254);
		
		msg.Format( _T("Image file for testing images could not be opened\n")
			_T("File name: %s\nReason: %s\nError code: %d"), 
			fe.m_strFileName, szCause, fe.m_cause );
		
		::MessageBox( NULL, msg, _T("Failed to open file for testing images"), MB_OK|MB_ICONEXCLAMATION );
		
		
		// close the already-opened files too
		
		m_fileTrainingLabels.Close();
		m_fileTrainingImages.Close();
		
		return;
	}
	
	
	
	// (4) Testing labels
	
	CFileDialog fd4( TRUE );  // constructs an "open" dialog; we are happy with the remaining defaults
	
	fd4.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0\0");
	fd4.m_ofn.lpstrTitle = _T("TESTING Labels");
	fd4.m_ofn.lpstrInitialDir = newPath;
	
	if ( fd4.DoModal() != IDOK ) 
	{
		// close the already-opened files too
		
		m_fileTestingImages.Close();
		m_fileTrainingLabels.Close();
		m_fileTrainingImages.Close();
		
		return;
	}
	
	
	// open the file
	
	if ( m_fileTestingLabels.Open( (LPCTSTR)fd4.GetPathName(), CFile::modeRead | CFile::shareDenyNone, &fe ) != 0 )
	{
		// opened successfully
		// check for expected magic number and for expected dimensionality
		
		FILEBEGINNING fileBegin;
		m_fileTestingLabels.Read( fileBegin.raw0, sizeof( fileBegin ) );
		
		// convert endian-ness (using winsock functions for convenience)
		
		fileBegin.nMagic = ::ntohl( fileBegin.nMagic );
		fileBegin.nItems = ::ntohl( fileBegin.nItems );
		
		// check against expected values
		
		if ( fileBegin.nMagic != ::GetPreferences().m_nMagicTestingLabels ||
			fileBegin.nItems != ::GetPreferences().m_nItemsTestingLabels )
		{
			// file is not configured as expected
			CString msg;
			msg.Format( _T("File header for testing labels contains unexpected values as follows\n")
				_T("Magic number: got %i, expected %i \n")
				_T("Number of items: got %i, expected %i \n")
				_T("\nCheck integrity of file for testing labels and/or adjust the expected values in the INI file"),
				fileBegin.nMagic, ::GetPreferences().m_nMagicTestingLabels,
				fileBegin.nItems, ::GetPreferences().m_nItemsTestingLabels );
			
			::MessageBox( NULL, msg, _T("Unexpected Format"), MB_OK|MB_ICONEXCLAMATION );
			
			// close all files
			
			m_fileTestingLabels.Close();
			m_fileTestingImages.Close();
			m_fileTrainingImages.Close();
			m_fileTrainingLabels.Close();
			
			return;
		}
	}
	else
	{
		// could not open label file
		
		TRACE(_T("%s(%i): Could not open file for testing labels \n"), __FILE__,__LINE__);
		
		CString msg;
		TCHAR szCause[255] = {0};
		
		fe.GetErrorMessage(szCause, 254);
		
		msg.Format( _T("Label file for testing labels could not be opened\n")
			_T("File name: %s\nReason: %s\nError code: %d"), 
			fe.m_strFileName, szCause, fe.m_cause );
		
		::MessageBox( NULL, msg, _T("Failed to open file for testing labels"), MB_OK|MB_ICONEXCLAMATION );
		
		// close all already-opened files
		
		m_fileTestingImages.Close();
		m_fileTrainingImages.Close();
		m_fileTrainingLabels.Close();
		
		return;
	}
	
	
	
	// all four files are opened, and all four contain expected header information
	
	m_bFilesOpen = TRUE;
	
	ASSERT( g_cImageSize == 28 );
	
}

void CMNistDoc::OnButtonCloseMnistFiles()
{
	CloseMnistFiles();
}

void CMNistDoc::CloseMnistFiles()
{
	m_fileTestingImages.Close();
	m_fileTestingLabels.Close();
	m_fileTrainingImages.Close();
	m_fileTrainingLabels.Close();
	
	m_bFilesOpen = FALSE;
	
}



double CMNistDoc::GetCurrentEta()
{
	return m_NN.m_etaLearningRate;
}


double CMNistDoc::GetPreviousEta()
{
	// provided because threads might change the current eta before we are able to read it
	
	return m_NN.m_etaLearningRatePrevious;
}


UINT CMNistDoc::GetCurrentTrainingPatternNumber( BOOL bFromRandomizedPatternSequence /* =FALSE */ )
{
	// returns the current number of the training pattern, either from the straight sequence, or from
	// the randomized sequence
	
	UINT iRet;
	
	if ( bFromRandomizedPatternSequence == FALSE )
	{
		iRet = m_iNextTrainingPattern;
	}
	else
	{
		iRet = m_iRandomizedTrainingPatternSequence[ m_iNextTrainingPattern ];
	}
	
	return iRet;
}



// UNIFORM_ZERO_THRU_ONE gives a uniformly-distributed number between zero (inclusive) and one (exclusive)

#define UNIFORM_ZERO_THRU_ONE ( (double)(rand())/(RAND_MAX + 1 ) ) 


void CMNistDoc::RandomizeTrainingPatternSequence()
{
	// randomizes the order of m_iRandomizedTrainingPatternSequence, which is a UINT array
	// holding the numbers 0..59999 in random order
	
	CAutoCS tlo( m_csTrainingPatterns );
	
	UINT ii, jj, iiMax, iiTemp;
	
	iiMax = ::GetPreferences().m_nItemsTrainingImages;
	
	ASSERT( iiMax == 60000 );  // requirement of sloppy and unimaginative code
	
	// initialize array in sequential order
	
	for ( ii=0; ii<iiMax; ++ii )
	{
		m_iRandomizedTrainingPatternSequence[ ii ] = ii;  
	}
	
	
	// now at each position, swap with a random position
	
	for ( ii=0; ii<iiMax; ++ii )
	{
		jj = (UINT)( UNIFORM_ZERO_THRU_ONE * iiMax );
		
		ASSERT( jj < iiMax );
		
		iiTemp = m_iRandomizedTrainingPatternSequence[ ii ];
		m_iRandomizedTrainingPatternSequence[ ii ] = m_iRandomizedTrainingPatternSequence[ jj ];
		m_iRandomizedTrainingPatternSequence[ jj ] = iiTemp;
	}
	
}



UINT CMNistDoc::GetNextTrainingPattern(unsigned char *pArray /* =NULL */, int *pLabel /* =NULL */, 
									   BOOL bFlipGrayscale /* =TRUE */, BOOL bFromRandomizedPatternSequence /* =TRUE */, UINT* iSequenceNum /* =NULL */)
{
	// returns the number of the pattern corresponding to the pattern that will be stored in pArray
	// if BOOL bFromRandomizedPatternSequence is TRUE (which is the default) then the pattern
	// stored will be a pattern from the randomized sequence; otherwise the pattern will be a straight
	// sequential run through all the training patterns, from start to finish.  The sequence number,
	// which runs from 0..59999 monotonically, is returned in iSequenceNum (if it's not NULL)
	
	CAutoCS tlo( m_csTrainingPatterns );
	
	UINT iPatternNum;
	
	if ( bFromRandomizedPatternSequence == FALSE )
	{
		iPatternNum = m_iNextTrainingPattern;
	}
	else
	{
		iPatternNum = m_iRandomizedTrainingPatternSequence[ m_iNextTrainingPattern ];
	}

	ASSERT( iPatternNum < ::GetPreferences().m_nItemsTrainingImages );
	
	GetTrainingPatternArrayValues( iPatternNum, pArray, pLabel, bFlipGrayscale );
	
	if ( iSequenceNum != NULL )
	{
		*iSequenceNum = m_iNextTrainingPattern;
	}
	
	m_iNextTrainingPattern++;
	
	if ( m_iNextTrainingPattern >= ::GetPreferences().m_nItemsTrainingImages )
	{
		m_iNextTrainingPattern = 0;
	}
	
	return iPatternNum;
}




UINT CMNistDoc::GetRandomTrainingPattern(unsigned char *pArray /* =NULL */, int *pLabel /* =NULL */, BOOL bFlipGrayscale /* =TRUE */ )
{
	// returns the number of the pattern corresponding to the pattern stored in pArray
	
	CAutoCS tlo( m_csTrainingPatterns );
	
	UINT patternNum = (UINT)( UNIFORM_ZERO_THRU_ONE * (::GetPreferences().m_nItemsTrainingImages - 1) );
	
	GetTrainingPatternArrayValues( patternNum, pArray, pLabel, bFlipGrayscale );
	
	return patternNum;
}





void CMNistDoc::GetTrainingPatternArrayValues(int iNumImage /* =0 */, unsigned char *pArray /* =NULL */, int *pLabel /* =NULL */,
											  BOOL bFlipGrayscale /* =TRUE */ )
{
	// fills an unsigned char array with gray values, corresponding to iNumImage, and also
	// returns the label for the image
	
	CAutoCS tlo( m_csTrainingPatterns );
	
	int cCount = g_cImageSize*g_cImageSize;
	int fPos;
	
	if ( m_bFilesOpen != FALSE )
	{
		if ( pArray != NULL )
		{
			fPos = 16 + iNumImage*cCount;  // 16 compensates for file header info
			m_fileTrainingImages.Seek( fPos, CFile::begin );
			m_fileTrainingImages.Read( pArray, cCount );
			
			if ( bFlipGrayscale != FALSE )
			{
				for ( int ii=0; ii<cCount; ++ii )
				{
					pArray[ ii ] = 255 - pArray[ ii ];
				}
			}
		}
		
		if ( pLabel != NULL )
		{
			fPos = 8 + iNumImage;
			char r;
			m_fileTrainingLabels.Seek( fPos, CFile::begin );
			m_fileTrainingLabels.Read( &r, 1 );  // single byte
			
			*pLabel = r;
		}
	}
	else  // no files are open: return a simple gray wedge
	{
		if ( pArray != NULL )
		{
			for ( int ii=0; ii<cCount; ++ii )
			{
				pArray[ ii ] = ii*255/cCount;
			}
		}
		
		if ( pLabel != NULL )
		{
			*pLabel = INT_MAX;
		}
	}
}



UINT CMNistDoc::GetNextTestingPatternNumber()
{
	return m_iNextTestingPattern;
}



UINT CMNistDoc::GetNextTestingPattern(unsigned char *pArray /* =NULL */, int *pLabel /* =NULL */, BOOL bFlipGrayscale /* =TRUE */ )
{
	// returns the number of the pattern corresponding to the pattern stored in pArray
	
	CAutoCS tlo( m_csTestingPatterns );
	
	
	GetTestingPatternArrayValues( m_iNextTestingPattern, pArray, pLabel, bFlipGrayscale );
	
	UINT iRet = m_iNextTestingPattern;
	m_iNextTestingPattern++;
	
	if ( m_iNextTestingPattern >= ::GetPreferences().m_nItemsTestingImages )
	{
		m_iNextTestingPattern = 0;
	}
	
	return iRet ;
}


void CMNistDoc::GetTestingPatternArrayValues(int iNumImage /* =0 */, unsigned char *pArray /* =NULL */, int *pLabel /* =NULL */,
											 BOOL bFlipGrayscale /* =TRUE */ )
{
	// fills an unsigned char array with gray values, corresponding to iNumImage, and also
	// returns the label for the image
	
	CAutoCS tlo( m_csTestingPatterns );
	
	int cCount = g_cImageSize*g_cImageSize;
	int fPos;
	
	if ( m_bFilesOpen != FALSE )
	{
		if ( pArray != NULL )
		{
			fPos = 16 + iNumImage*cCount;  // 16 compensates for file header info
			m_fileTestingImages.Seek( fPos, CFile::begin );
			m_fileTestingImages.Read( pArray, cCount );
			
			if ( bFlipGrayscale != FALSE )
			{
				for ( int ii=0; ii<cCount; ++ii )
				{
					pArray[ ii ] = 255 - pArray[ ii ];
				}
			}
		}
		
		if ( pLabel != NULL )
		{
			fPos = 8 + iNumImage;
			char r;
			m_fileTestingLabels.Seek( fPos, CFile::begin );
			m_fileTestingLabels.Read( &r, 1 );  // single byte
			
			*pLabel = r;
		}
	}
	else  // no files are open: return a simple gray wedge
	{
		if ( pArray != NULL )
		{
			for ( int ii=0; ii<cCount; ++ii )
			{
				pArray[ ii ] = ii*255/cCount;
			}
		}
		
		if ( pLabel != NULL )
		{
			*pLabel = INT_MAX;
		}
	}
}



void CMNistDoc::GenerateDistortionMap( double severityFactor /* =1.0 */ )
{
	// generates distortion maps in each of the horizontal and vertical directions
	// Three distortions are applied: a scaling, a rotation, and an elastic distortion
	// Since these are all linear tranformations, we can simply add them together, after calculation
	// one at a time
	
	// The input parameter, severityFactor, let's us control the severity of the distortions relative
	// to the default values.  For example, if we only want half as harsh a distortion, set
	// severityFactor == 0.5
	
	// First, elastic distortion, per Patrice Simard, "Best Practices For Convolutional Neural Networks..."
	// at page 2.
	// Three-step process: seed array with uniform randoms, filter with a gaussian kernel, normalize (scale)
	
	int row, col;
	double* uniformH = new double[ m_cCount ];
	double* uniformV = new double[ m_cCount ];
	
	
	for ( col=0; col<m_cCols; ++col )
	{
		for ( row=0; row<m_cRows; ++row )
		{
			At( uniformH, row, col ) = UNIFORM_PLUS_MINUS_ONE;
			At( uniformV, row, col ) = UNIFORM_PLUS_MINUS_ONE;
		}
	}
	
	// filter with gaussian
	
	double fConvolvedH, fConvolvedV;
	double fSampleH, fSampleV;
	double elasticScale = severityFactor * ::GetPreferences().m_dElasticScaling;
	int xxx, yyy, xxxDisp, yyyDisp;
	int iiMid = GAUSSIAN_FIELD_SIZE/2;  // GAUSSIAN_FIELD_SIZE is strictly odd
	
	for ( col=0; col<m_cCols; ++col )
	{
		for ( row=0; row<m_cRows; ++row )
		{
			fConvolvedH = 0.0;
			fConvolvedV = 0.0;
			
			for ( xxx=0; xxx<GAUSSIAN_FIELD_SIZE; ++xxx )
			{
				for ( yyy=0; yyy<GAUSSIAN_FIELD_SIZE; ++yyy )
				{
					xxxDisp = col - iiMid + xxx;
					yyyDisp = row - iiMid + yyy;
					
					if ( xxxDisp<0 || xxxDisp>=m_cCols || yyyDisp<0 || yyyDisp>=m_cRows )
					{
						fSampleH = 0.0;
						fSampleV = 0.0;
					}
					else
					{
						fSampleH = At( uniformH, yyyDisp, xxxDisp );
						fSampleV = At( uniformV, yyyDisp, xxxDisp );
					}
					
					fConvolvedH += fSampleH * m_GaussianKernel[ yyy ][ xxx ];
					fConvolvedV += fSampleV * m_GaussianKernel[ yyy ][ xxx ];
				}
			}
			
			At( m_DispH, row, col ) = elasticScale * fConvolvedH;
			At( m_DispV, row, col ) = elasticScale * fConvolvedV;
		}
	}
	
	delete[] uniformH;
	delete[] uniformV;
	
	// next, the scaling of the image by a random scale factor
	// Horizontal and vertical directions are scaled independently
	
	double dSFHoriz = severityFactor * ::GetPreferences().m_dMaxScaling / 100.0 * UNIFORM_PLUS_MINUS_ONE;  // m_dMaxScaling is a percentage
	double dSFVert = severityFactor * ::GetPreferences().m_dMaxScaling / 100.0 * UNIFORM_PLUS_MINUS_ONE;  // m_dMaxScaling is a percentage

	
	int iMid = m_cRows/2;
	
	for ( row=0; row<m_cRows; ++row )
	{
		for ( col=0; col<m_cCols; ++col )
		{
			At( m_DispH, row, col ) += dSFHoriz * ( col-iMid );
			At( m_DispV, row, col ) -= dSFVert * ( iMid-row );  // negative because of top-down bitmap
		}
	}
	
	
	// finally, apply a rotation
	
	double angle = severityFactor * ::GetPreferences().m_dMaxRotation * UNIFORM_PLUS_MINUS_ONE;
	angle = angle * 3.1415926535897932384626433832795 / 180.0;  // convert from degrees to radians
	
	double cosAngle = cos( angle );
	double sinAngle = sin( angle );
	
	for ( row=0; row<m_cRows; ++row )
	{
		for ( col=0; col<m_cCols; ++col )
		{
			At( m_DispH, row, col ) += ( col-iMid ) * ( cosAngle - 1 ) - ( iMid-row ) * sinAngle;
			At( m_DispV, row, col ) -= ( iMid-row ) * ( cosAngle - 1 ) + ( col-iMid ) * sinAngle;  // negative because of top-down bitmap
		}
	}
	
}


void CMNistDoc::ApplyDistortionMap(double *inputVector)
{
	// applies the current distortion map to the input vector
	
	// For the mapped array, we assume that 0.0 == background, and 1.0 == full intensity information
	// This is different from the input vector, in which +1.0 == background (white), and 
	// -1.0 == information (black), so we must convert one to the other
	
	std::vector< std::vector< double > >   mappedVector( m_cRows, std::vector< double >( m_cCols, 0.0 ));
	
	double sourceRow, sourceCol;
	double fracRow, fracCol;
	double w1, w2, w3, w4;
	double sourceValue;
	int row, col;
	int sRow, sCol, sRowp1, sColp1;
	BOOL bSkipOutOfBounds;
	
	for ( row=0; row<m_cRows; ++row )
	{
		for ( col=0; col<m_cCols; ++col )
		{
			// the pixel at sourceRow, sourceCol is an "phantom" pixel that doesn't really exist, and
			// whose value must be manufactured from surrounding real pixels (i.e., since 
			// sourceRow and sourceCol are floating point, not ints, there's not a real pixel there)
			// The idea is that if we can calculate the value of this phantom pixel, then its 
			// displacement will exactly fit into the current pixel at row, col (which are both ints)
			
			sourceRow = (double)row - At( m_DispV, row, col );
			sourceCol = (double)col - At( m_DispH, row, col );
			
			// weights for bi-linear interpolation
			
			fracRow = sourceRow - (int)sourceRow;
			fracCol = sourceCol - (int)sourceCol;
			
			
			w1 = ( 1.0 - fracRow ) * ( 1.0 - fracCol );
			w2 = ( 1.0 - fracRow ) * fracCol;
			w3 = fracRow * ( 1 - fracCol );
			w4 = fracRow * fracCol;
			
			
			// limit indexes

/*
			while (sourceRow >= m_cRows ) sourceRow -= m_cRows;
			while (sourceRow < 0 ) sourceRow += m_cRows;
			
			while (sourceCol >= m_cCols ) sourceCol -= m_cCols;
			while (sourceCol < 0 ) sourceCol += m_cCols;
*/
			bSkipOutOfBounds = FALSE;

			if ( (sourceRow + 1.0) >= m_cRows )	bSkipOutOfBounds = TRUE;
			if ( sourceRow < 0 )				bSkipOutOfBounds = TRUE;
			
			if ( (sourceCol + 1.0) >= m_cCols )	bSkipOutOfBounds = TRUE;
			if ( sourceCol < 0 )				bSkipOutOfBounds = TRUE;
			
			if ( bSkipOutOfBounds == FALSE )
			{
				// the supporting pixels for the "phantom" source pixel are all within the 
				// bounds of the character grid.
				// Manufacture its value by bi-linear interpolation of surrounding pixels
				
				sRow = (int)sourceRow;
				sCol = (int)sourceCol;
				
				sRowp1 = sRow + 1;
				sColp1 = sCol + 1;
				
				while (sRowp1 >= m_cRows ) sRowp1 -= m_cRows;
				while (sRowp1 < 0 ) sRowp1 += m_cRows;
				
				while (sColp1 >= m_cCols ) sColp1 -= m_cCols;
				while (sColp1 < 0 ) sColp1 += m_cCols;
				
				// perform bi-linear interpolation
				
				sourceValue =	w1 * At( inputVector, sRow  , sCol   ) +
					w2 * At( inputVector, sRow  , sColp1 ) +
					w3 * At( inputVector, sRowp1, sCol   ) +
					w4 * At( inputVector, sRowp1, sColp1 );
			}
			else
			{
				// At least one supporting pixel for the "phantom" pixel is outside the
				// bounds of the character grid. Set its value to "background"

				sourceValue = 1.0;  // "background" color in the -1 -> +1 range of inputVector
			}
			
			mappedVector[ row ][ col ] = 0.5 * ( 1.0 - sourceValue );  // conversion to 0->1 range we are using for mappedVector
			
		}
	}
	
	// now, invert again while copying back into original vector
	
	for ( row=0; row<m_cRows; ++row )
	{
		for ( col=0; col<m_cCols; ++col )
		{
			At( inputVector, row, col ) = 1.0 - 2.0 * mappedVector[ row ][ col ];
		}
	}			
	
}



void CMNistDoc::CalculateNeuralNet(double *inputVector, int count, 
								   double* outputVector /* =NULL */, int oCount /* =0 */,
								   std::vector< std::vector< double > >* pNeuronOutputs /* =NULL */,
								   BOOL bDistort /* =FALSE */ )
{
	// wrapper function for neural net's Calculate() function, needed because the NN is a protected member
	// waits on the neural net mutex (using the CAutoMutex object, which automatically releases the
	// mutex when it goes out of scope) so as to restrict access to one thread at a time
	
	CAutoMutex tlo( m_utxNeuralNet );
	
	if ( bDistort != FALSE )
	{
		GenerateDistortionMap();
		ApplyDistortionMap( inputVector );
	}
	
	
	m_NN.Calculate( inputVector, count, outputVector, oCount, pNeuronOutputs );
	
}


void CMNistDoc::BackpropagateNeuralNet(double *inputVector, int iCount, double* targetOutputVector, 
									   double* actualOutputVector, int oCount, 
									   std::vector< std::vector< double > >* pMemorizedNeuronOutputs, 
									   BOOL bDistort )
{
	// function to backpropagate through the neural net. 
	
	ASSERT( (inputVector != NULL) && (targetOutputVector != NULL) && (actualOutputVector != NULL) );

	///////////////////////////////////////////////////////////////////////
	//
	// CODE REVIEW NEEDED:
	//
	// It does not seem worthwhile to backpropagate an error that's very small.  "Small" needs to be defined
	// and for now, "small" is set to a fixed size of pattern error ErrP <= 0.10 * MSE, then there will
	// not be a backpropagation of the error.  The current MSE is updated from the neural net dialog CDlgNeuralNet

	BOOL bWorthwhileToBackpropagate;  /////// part of code review
	
	{	
		// local scope for capture of the neural net, only during the forward calculation step,
		// i.e., we release neural net for other threads after the forward calculation, and after we
		// have stored the outputs of each neuron, which are needed for the backpropagation step
		
		CAutoMutex tlo( m_utxNeuralNet );
		
		// determine if it's time to adjust the learning rate
		
		if ( (( m_cBackprops % m_nAfterEveryNBackprops ) == 0) && (m_cBackprops != 0) )
		{
			double eta = m_NN.m_etaLearningRate;
			eta *= m_dEtaDecay;
			if ( eta < m_dMinimumEta )
				eta = m_dMinimumEta;
			m_NN.m_etaLearningRatePrevious = m_NN.m_etaLearningRate;
			m_NN.m_etaLearningRate = eta;
		}
		
		
		// determine if it's time to adjust the Hessian (currently once per epoch)
		
		if ( (m_bNeedHessian != FALSE) || (( m_cBackprops % ::GetPreferences().m_nItemsTrainingImages ) == 0) )
		{
			// adjust the Hessian.  This is a lengthy operation, since it must process approx 500 labels
			
			CalculateHessian();
			
			m_bNeedHessian = FALSE;
		}
		
		
		// determine if it's time to randomize the sequence of training patterns (currently once per epoch)
		
		if ( ( m_cBackprops % ::GetPreferences().m_nItemsTrainingImages ) == 0 )
		{
			RandomizeTrainingPatternSequence();
		}
		
		
		// increment counter for tracking number of backprops
		
		m_cBackprops++;
		
		
		// forward calculate through the neural net
		
		CalculateNeuralNet( inputVector, iCount, actualOutputVector, oCount, pMemorizedNeuronOutputs, bDistort );


		// calculate error in the output of the neural net
		// note that this code duplicates that found in many other places, and it's probably sensible to 
		// define a (global/static ??) function for it

		double dMSE = 0.0;
		for ( int ii=0; ii<10; ++ii )
		{
			dMSE += ( actualOutputVector[ii]-targetOutputVector[ii] ) * ( actualOutputVector[ii]-targetOutputVector[ii] );
		}
		dMSE /= 2.0;

		if ( dMSE <= ( 0.10 * m_dEstimatedCurrentMSE ) )
		{
			bWorthwhileToBackpropagate = FALSE;
		}
		else
		{
			bWorthwhileToBackpropagate = TRUE;
		}

		
		if ( (bWorthwhileToBackpropagate != FALSE) && (pMemorizedNeuronOutputs == NULL) )
		{
			// the caller has not provided a place to store neuron outputs, so we need to
			// backpropagate now, while the neural net is still captured.  Otherwise, another thread
			// might come along and call CalculateNeuralNet(), which would entirely change the neuron
			// outputs and thereby inject errors into backpropagation 
			
			m_NN.Backpropagate( actualOutputVector, targetOutputVector, oCount, NULL );
			
			SetModifiedFlag( TRUE );
			
			// we're done, so return
			
			return;
		}
		
	}
	
	// if we have reached here, then the mutex for the neural net has been released for other 
	// threads.  The caller must have provided a place to store neuron outputs, which we can 
	// use to backpropagate, even if other threads call CalculateNeuralNet() and change the outputs
	// of the neurons

	if ( (bWorthwhileToBackpropagate != FALSE) )
	{
		m_NN.Backpropagate( actualOutputVector, targetOutputVector, oCount, pMemorizedNeuronOutputs );
		
		// set modified flag to prevent closure of doc without a warning
		
		SetModifiedFlag( TRUE );
	}
	
}



void CMNistDoc::CalculateHessian()
{
	// controls the Neural network's calculation if the diagonal Hessian for the Neural net
	// This will be called from a thread, so although the calculation is lengthy, it should not interfere
	// with the UI
	
	// we need the neural net exclusively during this calculation, so grab it now
	
	CAutoMutex tlo( m_utxNeuralNet );
	
	double inputVector[841] = {0.0};  // note: 29x29, not 28x28
	double targetOutputVector[10] = {0.0};
	double actualOutputVector[10] = {0.0};
	
	unsigned char grayLevels[g_cImageSize * g_cImageSize] = { 0 };
	int label = 0;
	int ii, jj;
	UINT kk;
	
	// calculate the diagonal Hessian using 500 random patterns, per Yann LeCun 1998 "Gradient-Based Learning
	// Applied To Document Recognition"
	
	// message to dialog that we are commencing calculation of the Hessian
	
	if ( m_hWndForBackpropPosting != NULL )
	{
		// wParam == 4L -> related to Hessian, lParam == 1L -> commenced calculation
		::PostMessage( m_hWndForBackpropPosting, UWM_BACKPROPAGATION_NOTIFICATION, 4L, 1L );
	}
	
	
	// some of this code is similar to the BackpropagationThread() code
	
	m_NN.EraseHessianInformation();
	
	UINT numPatternsSampled = ::GetPreferences().m_nNumHessianPatterns ;
	
	for ( kk=0; kk<numPatternsSampled; ++kk )
	{
		GetRandomTrainingPattern( grayLevels, &label, TRUE );
		
		if ( label < 0 ) label = 0;
		if ( label > 9 ) label = 9;
		
		
		// pad to 29x29, convert to double precision
		
		for ( ii=0; ii<841; ++ii )
		{
			inputVector[ ii ] = 1.0;  // one is white, -one is black
		}
		
		// top row of inputVector is left as zero, left-most column is left as zero 
		
		for ( ii=0; ii<g_cImageSize; ++ii )
		{
			for ( jj=0; jj<g_cImageSize; ++jj )
			{
				inputVector[ 1 + jj + 29*(ii+1) ] = (double)((int)(unsigned char)grayLevels[ jj + g_cImageSize*ii ])/128.0 - 1.0;  // one is white, -one is black
			}
		}
		
		// desired output vector
		
		for ( ii=0; ii<10; ++ii )
		{
			targetOutputVector[ ii ] = -1.0;
		}
		targetOutputVector[ label ] = 1.0;
		
		
		// apply distortion map to inputVector.  It's not certain that this is needed or helpful.
		// The second derivatives do NOT rely on the output of the neural net (i.e., because the 
		// second derivative of the MSE function is exactly 1 (one), regardless of the actual output
		// of the net).  However, since the backpropagated second derivatives rely on the outputs of
		// each neuron, distortion of the pattern might reveal previously-unseen information about the
		// nature of the Hessian.  But I am reluctant to give the full distortion, so I set the
		// severityFactor to only 2/3 approx
		
		GenerateDistortionMap( 0.65 );
		ApplyDistortionMap( inputVector );
		
		
		// forward calculate the neural network
		
		m_NN.Calculate( inputVector, 841, actualOutputVector, 10, NULL );
		
		
		// backpropagate the second derivatives
		
		m_NN.BackpropagateSecondDervatives( actualOutputVector, targetOutputVector, 10 );
		
		
		// progress message to dialog that we are calculating the Hessian
		
		if ( kk%50 == 0 )
		{
			// every 50 iterations ...
			if ( m_hWndForBackpropPosting != NULL )
			{
				// wParam == 4L -> related to Hessian, lParam == 2L -> progress indicator
				::PostMessage( m_hWndForBackpropPosting, UWM_BACKPROPAGATION_NOTIFICATION, 4L, 2L );
			}
		}
		
		if ( m_bBackpropThreadAbortFlag != FALSE )
			break;
		
	}
	
	m_NN.DivideHessianInformationBy( (double)numPatternsSampled );
	
	// message to dialog that we are finished calculating the Hessian
	
	if ( m_hWndForBackpropPosting != NULL )
	{
		// wParam == 4L -> related to Hessian, lParam == 4L -> finished calculation
		::PostMessage( m_hWndForBackpropPosting, UWM_BACKPROPAGATION_NOTIFICATION, 4L, 4L );
	}
	
}



BOOL CMNistDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// check if any threads are running before we allow the main frame to close down
	
	BOOL bRet = TRUE;
	
	if ( (m_bBackpropThreadsAreRunning != FALSE) || (m_bTestingThreadsAreRunning != FALSE) )
	{
		CString str;
		int iRet;
		
		str.Format( _T( "This will stop backpropagation and/or testing threads \n" )
			_T( "Are you certain that you wish to close the application? \n\n" )
			_T( "Click \"Yes\" to stop all threads and close the application \n" )
			_T( "Click \"No\" or \"Cancel\" to continue running the threads and the application " ) );
		
		iRet = ::MessageBox( NULL, str, _T( "Threads Are Running" ), MB_ICONEXCLAMATION | MB_YESNOCANCEL );
		
		if ( (iRet == IDYES) || (iRet == IDOK) )
		{
			bRet = TRUE;
			
			if ( m_bBackpropThreadsAreRunning != FALSE )
			{
				StopBackpropagation();
			}
			
			if ( m_bTestingThreadsAreRunning != FALSE )
			{
				StopTesting();
			}
		}
		else
		{
			bRet = FALSE;
		}
	}
	
	if ( bRet != FALSE )
	{
		// only call the base class if, so far, it's still safe to close.  If we
		// always called the base class, then we would get needless reminders to save the
		// current document.  These reminders are not needed, since we're not closing 
		// anyway
		
		bRet &= COleDocument::CanCloseFrame(pFrame);
	}
	
	return bRet;
}



BOOL CMNistDoc::StartBackpropagation(UINT iStartPattern /* =0 */, UINT iNumThreads /* =2 */, HWND hWnd /* =NULL */,
									 double initialEta /* =0.005 */, double minimumEta /* =0.000001 */, double etaDecay /* =0.990 */, 
									 UINT nAfterEvery  /* =1000 */, BOOL bDistortPatterns /* =TRUE */, double estimatedCurrentMSE /* =1.0 */)
{
	if ( m_bBackpropThreadsAreRunning != FALSE )
		return FALSE;
	
	m_bBackpropThreadAbortFlag = FALSE;
	m_bBackpropThreadsAreRunning = TRUE;
	m_iNumBackpropThreadsRunning = 0;
	m_iBackpropThreadIdentifier = 0;
	m_cBackprops = iStartPattern;
	m_bNeedHessian = TRUE;
	
	m_iNextTrainingPattern = iStartPattern;
	m_hWndForBackpropPosting = hWnd;
	
	if ( m_iNextTrainingPattern < 0 ) 
		m_iNextTrainingPattern = 0;
	if ( m_iNextTrainingPattern >= ::GetPreferences().m_nItemsTrainingImages )
		m_iNextTrainingPattern = ::GetPreferences().m_nItemsTrainingImages - 1;
	
	if ( iNumThreads < 1 ) 
		iNumThreads = 1;
	if ( iNumThreads > 10 )  // 10 is arbitrary upper limit
		iNumThreads = 10;
	
	m_NN.m_etaLearningRate = initialEta;
	m_NN.m_etaLearningRatePrevious = initialEta;
	m_dMinimumEta = minimumEta;
	m_dEtaDecay = etaDecay;
	m_nAfterEveryNBackprops = nAfterEvery;
	m_bDistortTrainingPatterns = bDistortPatterns;

	m_dEstimatedCurrentMSE = estimatedCurrentMSE;  // estimated number that will define whether a forward calculation's error is significant enough to warrant backpropagation

	RandomizeTrainingPatternSequence();
	
	for ( UINT ii=0; ii<iNumThreads; ++ii )
	{
		CWinThread* pThread = ::AfxBeginThread( BackpropagationThread, (LPVOID)this, 
			THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED, NULL );
		
		if ( pThread == NULL )
		{
			// creation failed; un-do everything
			
			StopBackpropagation();
			return FALSE;
		}
		
		pThread->m_bAutoDelete = FALSE;		 
		m_pBackpropThreads[ ii ] = pThread;
		pThread->ResumeThread();
		m_iNumBackpropThreadsRunning++;
	}
	
	return TRUE;
	
}




void CMNistDoc::StopBackpropagation()
{
	// stops all the backpropagation threads
	
	if ( m_bBackpropThreadsAreRunning == FALSE )
	{
		// it's curious to select "stop" if no threads are running, but perform some
		// shutdown safeguards, just to be certain
		
		m_bBackpropThreadAbortFlag = TRUE;
		m_bBackpropThreadsAreRunning = FALSE;
		m_iNumBackpropThreadsRunning = 0;
		m_iBackpropThreadIdentifier = 0;
		m_cBackprops = 0;
		
		return;
	}
	
	m_bBackpropThreadAbortFlag = TRUE;
	
	HANDLE hThread[25];
	CString msg;
	DWORD dwTimeOut = 5000;  // 5 second default timeout
	UINT ii;
	
	while ( m_iNumBackpropThreadsRunning > 0 )
	{
		for ( ii=0; ii<m_iNumBackpropThreadsRunning; ++ii )
		{
			hThread[ ii ] = m_pBackpropThreads[ ii ]->m_hThread;
		}
		
		DWORD dwRet = ::WaitForMultipleObjects( m_iNumBackpropThreadsRunning, hThread, FALSE, dwTimeOut );
		DWORD dwErr = ::GetLastError();		// if an error occurred get its value as soon as possible
		
		if ( dwRet==WAIT_FAILED )
		{
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
			
			::MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error Waiting For Backpropagation Thread Shutdown"), MB_OK | MB_ICONINFORMATION );
			LocalFree( lpMsgBuf );
		}
		else if ( dwRet==WAIT_TIMEOUT )
		{
			// bad -- no threads are responding
			// give user option of waiting a bit more, or of terminating the threads forcefully
			
			msg.Format( _T("No thread has responded after waiting %d milliseconds\n\n")
				_T("Click \"Retry\" to wait another %d milliseconds and give a thread\n")
				_T("a chance to respond\n\n")
				_T("Click \"Cancel\" to terminate a thread forcibly\n\n")
				_T("Note: Forceful termination is not recommended and might cause memory leaks"), dwTimeOut, dwTimeOut );
			if ( IDCANCEL == ::MessageBox( NULL, msg, _T("No Thread Is Responding"), MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1 ) )
			{
				// forceful thread termination was selected
				// pick first thread from list and terminate it
				
				::TerminateThread( hThread[0], 98 );	// specify exit code of "98"
			}
			
		}
		else if ( dwRet>=WAIT_ABANDONED_0 && dwRet<=(WAIT_ABANDONED_0+m_iNumBackpropThreadsRunning-1) )
		{
			msg.Format( _T("Thread reports mutex was abandoned") );
			::MessageBox( NULL, msg, _T("Mutex Abandoned"), MB_OK|MB_ICONEXCLAMATION );
		}
		else if ( dwRet>=WAIT_OBJECT_0 && dwRet<=(WAIT_OBJECT_0+m_iNumBackpropThreadsRunning-1) )
		{
			// the most common and expected return value
			// delete the object that signalled
			
			int nDex = dwRet - WAIT_OBJECT_0;
			
			delete m_pBackpropThreads[ nDex ];
			
			for ( ii=nDex; ii<m_iNumBackpropThreadsRunning-1; ++ii )
			{
				m_pBackpropThreads[ ii ] = m_pBackpropThreads[ ii+1 ];
			}
			
			m_iNumBackpropThreadsRunning--;
			
		}
		else
		{
			ASSERT( FALSE );	// shouldn't be able to get here
		}
	}
	
	
	// at this point, all threads have been terminated, so re-set flags to allow
	// for future re-start of the threads
	
	m_bBackpropThreadAbortFlag = TRUE;
	m_bBackpropThreadsAreRunning = FALSE;
	m_iNumBackpropThreadsRunning = 0;
	m_iBackpropThreadIdentifier = 0;
	m_cBackprops = 0;
	
	
	
}




UINT CMNistDoc::BackpropagationThread(LPVOID pVoid)
{
	// thread for backpropagation training of NN
	//
	// thread is "owned" by the doc, and accepts a pointer to the doc
	// continuously backpropagates until m_bThreadAbortFlag is set to TRUE
	
	CMNistDoc* pThis = reinterpret_cast< CMNistDoc* >( pVoid );
	
	ASSERT( pThis != NULL );
	
	// set thread name (helps during debugging)
	
	char str[25] = {0};  // must use chars, not TCHARs, for SetThreadname function
	sprintf( str, "BACKP%02d", pThis->m_iBackpropThreadIdentifier++ );
	SetThreadName( -1, str );
	
	// do the work
	
	double inputVector[841] = {0.0};  // note: 29x29, not 28x28
	double targetOutputVector[10] = {0.0};
	double actualOutputVector[10] = {0.0};
	double dMSE;
	UINT scaledMSE;
	unsigned char grayLevels[g_cImageSize * g_cImageSize] = { 0 };
	int label = 0;
	int ii, jj;
	UINT iSequentialNum;
	
	std::vector< std::vector< double > > memorizedNeuronOutputs;
	
	while ( pThis->m_bBackpropThreadAbortFlag == FALSE )
	{
		int iRet = pThis->GetNextTrainingPattern( grayLevels, &label, TRUE, TRUE, &iSequentialNum );
		
		if ( label < 0 ) label = 0;
		if ( label > 9 ) label = 9;
		
		// post message to the dialog, telling it which pattern this thread is currently working on
		
		if ( pThis->m_hWndForBackpropPosting != NULL )
		{
			::PostMessage( pThis->m_hWndForBackpropPosting, UWM_BACKPROPAGATION_NOTIFICATION, 1L, (LPARAM)iSequentialNum );
		}
		
		
		// pad to 29x29, convert to double precision
		
		for ( ii=0; ii<841; ++ii )
		{
			inputVector[ ii ] = 1.0;  // one is white, -one is black
		}
		
		// top row of inputVector is left as zero, left-most column is left as zero 
		
		for ( ii=0; ii<g_cImageSize; ++ii )
		{
			for ( jj=0; jj<g_cImageSize; ++jj )
			{
				inputVector[ 1 + jj + 29*(ii+1) ] = (double)((int)(unsigned char)grayLevels[ jj + g_cImageSize*ii ])/128.0 - 1.0;  // one is white, -one is black
			}
		}
		
		// desired output vector
		
		for ( ii=0; ii<10; ++ii )
		{
			targetOutputVector[ ii ] = -1.0;
		}
		targetOutputVector[ label ] = 1.0;
		
		// now backpropagate

		pThis->BackpropagateNeuralNet( inputVector, 841, targetOutputVector, actualOutputVector, 10, 
			&memorizedNeuronOutputs, pThis->m_bDistortTrainingPatterns );

		
		// calculate error for this pattern and post it to the hwnd so it can calculate a running 
		// estimate of MSE
		
		dMSE = 0.0;
		for ( ii=0; ii<10; ++ii )
		{
			dMSE += ( actualOutputVector[ii]-targetOutputVector[ii] ) * ( actualOutputVector[ii]-targetOutputVector[ii] );
		}
		dMSE /= 2.0;

		scaledMSE = (UINT)( sqrt( dMSE ) * 2.0e8 );  // arbitrary large pre-agreed upon scale factor; taking sqrt is simply to improve the scaling
		
		if ( pThis->m_hWndForBackpropPosting != NULL )
		{
			::PostMessage( pThis->m_hWndForBackpropPosting, UWM_BACKPROPAGATION_NOTIFICATION, 2L, (LPARAM)scaledMSE );
		}
		
		
		// determine the neural network's answer, and compare it to the actual answer.
		// Post a message if the answer was incorrect, so the dialog can display mis-recognition
		// statistics
		
		int iBestIndex = 0;
		double maxValue = -99.0;
		
		for ( ii=0; ii<10; ++ii )
		{
			if ( actualOutputVector[ ii ] > maxValue )
			{
				iBestIndex = ii;
				maxValue = actualOutputVector[ ii ];
			}
		}

		if ( iBestIndex != label )
		{
			// pattern was mis-recognized.  Notify the testing dialog
			if ( pThis->m_hWndForBackpropPosting != NULL )
			{
				::PostMessage( pThis->m_hWndForBackpropPosting, UWM_BACKPROPAGATION_NOTIFICATION, 8L, (LPARAM)0L );
			}
		}
		
	}  // end of main "while not abort flag" loop
	
	return 0L;
}







BOOL CMNistDoc::StartTesting(UINT iStartingPattern, UINT iNumThreads, HWND hWnd, BOOL bDistortPatterns,
							 UINT iWhichImageSet /* =1 */ )
{
	// creates and starts testing threads
	
	if ( m_bTestingThreadsAreRunning != FALSE )
		return FALSE;
	
	m_bTestingThreadAbortFlag = FALSE;
	m_bTestingThreadsAreRunning = TRUE;
	m_iNumTestingThreadsRunning = 0;
	m_iTestingThreadIdentifier = 0;
	
	m_iNextTestingPattern = iStartingPattern;
	m_hWndForTestingPosting = hWnd;
	m_iWhichImageSet = iWhichImageSet;
	
	if ( m_iWhichImageSet > 1 )
		m_iWhichImageSet = 1;
	if ( m_iWhichImageSet < 0 )  // which is not possible, since m_iWhichImageSet is a UINT
		m_iWhichImageSet = 0;
	
	if ( m_iNextTestingPattern < 0 ) 
		m_iNextTestingPattern = 0;
	if ( m_iNextTestingPattern >= ::GetPreferences().m_nItemsTestingImages )
		m_iNextTestingPattern = ::GetPreferences().m_nItemsTestingImages - 1;
	
	if ( iNumThreads < 1 ) 
		iNumThreads = 1;
	if ( iNumThreads > 10 )  // 10 is arbitrary upper limit
		iNumThreads = 10;
	
	m_bDistortTestingPatterns = bDistortPatterns;
	
	for ( UINT ii=0; ii<iNumThreads; ++ii )
	{
		CWinThread* pThread = ::AfxBeginThread( TestingThread, (LPVOID)this, 
			THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED, NULL );
		
		if ( pThread == NULL )
		{
			// creation failed; un-do everything
			
			StopTesting();
			return FALSE;
		}
		
		pThread->m_bAutoDelete = FALSE;		 
		m_pTestingThreads[ ii ] = pThread;
		pThread->ResumeThread();
		m_iNumTestingThreadsRunning++;
	}
	
	return TRUE;
}

void CMNistDoc::StopTesting()
{
	// stops all the testing threads
	
	if ( m_bTestingThreadsAreRunning == FALSE )
	{
		// it's curious to select "stop" if no threads are running, but perform some
		// shutdown safeguards, just to be certain
		
		m_bTestingThreadAbortFlag = TRUE;
		m_bTestingThreadsAreRunning = FALSE;
		m_iNumTestingThreadsRunning = 0;
		m_iTestingThreadIdentifier = 0;
		return;
	}
	
	m_bTestingThreadAbortFlag = TRUE;
	
	HANDLE hThread[25];
	CString msg;
	DWORD dwTimeOut = 5000;  // 5 second default timeout
	UINT ii;
	
	while ( m_iNumTestingThreadsRunning > 0 )
	{
		for ( ii=0; ii<m_iNumTestingThreadsRunning; ++ii )
		{
			hThread[ ii ] = m_pTestingThreads[ ii ]->m_hThread;
		}
		
		DWORD dwRet = ::WaitForMultipleObjects( m_iNumTestingThreadsRunning, hThread, FALSE, dwTimeOut );
		DWORD dwErr = ::GetLastError();		// if an error occurred get its value as soon as possible
		
		if ( dwRet==WAIT_FAILED )
		{
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
			
			::MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error Waiting For Testing Thread Shutdown"), MB_OK | MB_ICONINFORMATION );
			LocalFree( lpMsgBuf );
		}
		else if ( dwRet==WAIT_TIMEOUT )
		{
			// bad -- no threads are responding
			// give user option of waiting a bit more, or of terminating the threads forcefully
			
			msg.Format( _T("No thread has responded after waiting %d milliseconds\n\n")
				_T("Click \"Retry\" to wait another %d milliseconds and give a thread\n")
				_T("a chance to respond\n\n")
				_T("Click \"Cancel\" to terminate a thread forcibly\n\n")
				_T("Note: Forceful termination is not recommended and might cause memory leaks"), dwTimeOut, dwTimeOut );
			if ( IDCANCEL == ::MessageBox( NULL, msg, _T("No Thread Is Responding"), MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1 ) )
			{
				// forceful thread termination was selected
				// pick first thread from list and terminate it
				
				::TerminateThread( hThread[0], 98 );	// specify exit code of "98"
			}
			
		}
		else if ( dwRet>=WAIT_ABANDONED_0 && dwRet<=(WAIT_ABANDONED_0+m_iNumTestingThreadsRunning-1) )
		{
			msg.Format( _T("Thread reports mutex was abandoned") );
			::MessageBox( NULL, msg, _T("Mutex Abandoned"), MB_OK|MB_ICONEXCLAMATION );
		}
		else if ( dwRet>=WAIT_OBJECT_0 && dwRet<=(WAIT_OBJECT_0+m_iNumTestingThreadsRunning-1) )
		{
			// the most common and expected return value
			// delete the object that signalled
			
			int nDex = dwRet - WAIT_OBJECT_0;
			
			delete m_pTestingThreads[ nDex ];
			
			for ( ii=nDex; ii<m_iNumTestingThreadsRunning-1; ++ii )
			{
				m_pTestingThreads[ ii ] = m_pTestingThreads[ ii+1 ];
			}
			
			m_iNumTestingThreadsRunning--;
			
		}
		else
		{
			ASSERT( FALSE );	// shouldn't be able to get here
		}
	}
	
	
	// at this point, all threads have been terminated, so re-set flags to allow
	// for future re-start of the threads
	
	m_bTestingThreadAbortFlag = TRUE;
	m_bTestingThreadsAreRunning = FALSE;
	m_iNumTestingThreadsRunning = 0;
	m_iTestingThreadIdentifier = 0;
	
	
}

UINT CMNistDoc::TestingThread(LPVOID pVoid)
{
	// thread for testing of Neural net
	// Continuously get the doc's next pattern, puts it through the neural net, and
	// inspects the output.  As the thread goes through the patterns, it post messages to the
	// m_hWndForTestingPosting, which presumably is the dialog that shows testing results,
	// advising it of the current pattern being tested.  If the actual output from the 
	// neural net differs from the desired output, another message is posted, advising the 
	// m_hWndForTestingPosting of the identity of the mis-recognized pattern
	
	// thread is owned by the doc and accepts a pointer to the doc as a parameter
	
	
	CMNistDoc* pThis = reinterpret_cast< CMNistDoc* >( pVoid );
	
	ASSERT( pThis != NULL );
	
	// set thread name (helps during debugging)
	
	char str[25] = {0};  // must use chars, not TCHARs, for SetThreadname function
	sprintf( str, "TEST%02d", pThis->m_iTestingThreadIdentifier++ );
	SetThreadName( -1, str );
	
	// do the work
	
	double inputVector[841] = {0.0};  // note: 29x29, not 28x28
	double targetOutputVector[10] = {0.0};
	double actualOutputVector[10] = {0.0};

	double dPatternMSE = 0.0;
	double dTotalMSE = 0.0;
	UINT scaledMSE = 0;
	UINT iPatternsProcessed = 0;
	
	unsigned char grayLevels[g_cImageSize * g_cImageSize] = { 0 };
	int label = 0;
	int ii, jj;
	UINT iPatNum, iSequentialNum;
	
	while ( pThis->m_bTestingThreadAbortFlag == FALSE )
	{
		// testing image set or training image set
		
		if ( pThis->m_iWhichImageSet == 1 )
		{
			// testing set
			
			iPatNum = pThis->GetNextTestingPattern( grayLevels, &label, TRUE );
			
			// post message to the dialog, telling it which pattern this thread is currently working on
			
			if ( pThis->m_hWndForTestingPosting != NULL )
			{
				::PostMessage( pThis->m_hWndForTestingPosting, UWM_TESTING_NOTIFICATION, 1L, (LPARAM)iPatNum );
			}
		}
		else
		{
			// training set
			
			iPatNum = pThis->GetNextTrainingPattern( grayLevels, &label, TRUE, FALSE, &iSequentialNum );
			
			// post message to the dialog, telling it which pattern this thread is currently working on
			
			if ( pThis->m_hWndForTestingPosting != NULL )
			{
				::PostMessage( pThis->m_hWndForTestingPosting, UWM_TESTING_NOTIFICATION, 1L, (LPARAM)iSequentialNum );
			}
		}
		
		
		if ( label < 0 ) label = 0;
		if ( label > 9 ) label = 9;
		
		
		
		
		// pad to 29x29, convert to double precision
		
		for ( ii=0; ii<841; ++ii )
		{
			inputVector[ ii ] = 1.0;  // one is white, -one is black
		}
		
		// top row of inputVector is left as zero, left-most column is left as zero 
		
		for ( ii=0; ii<g_cImageSize; ++ii )
		{
			for ( jj=0; jj<g_cImageSize; ++jj )
			{
				inputVector[ 1 + jj + 29*(ii+1) ] = (double)((int)(unsigned char)grayLevels[ jj + g_cImageSize*ii ])/128.0 - 1.0;  // one is white, -one is black
			}
		}
		
		// desired output vector
		
		for ( ii=0; ii<10; ++ii )
		{
			targetOutputVector[ ii ] = -1.0;
		}
		targetOutputVector[ label ] = 1.0;
		

		// now calculate output of neural network
		
		pThis->CalculateNeuralNet( inputVector, 841, actualOutputVector, 10, NULL, pThis->m_bDistortTestingPatterns );

		
		// calculate error for this pattern and accumulate it for posting of
		// total MSE of all patterns when thread is exiting
		
		dPatternMSE = 0.0;
		for ( ii=0; ii<10; ++ii )
		{
			dPatternMSE += ( actualOutputVector[ii]-targetOutputVector[ii] ) * ( actualOutputVector[ii]-targetOutputVector[ii] );
		}
		dPatternMSE /= 2.0;

		dTotalMSE += dPatternMSE;
		++iPatternsProcessed;		
		

		// determine the neural network's answer, and compare it to the actual answer
		
		int iBestIndex = 0;
		double maxValue = -99.0;
		UINT code;
		
		for ( ii=0; ii<10; ++ii )
		{
			if ( actualOutputVector[ ii ] > maxValue )
			{
				iBestIndex = ii;
				maxValue = actualOutputVector[ ii ];
			}
		}
		
		
		// moment of truth: Did neural net get the correct answer
		
		if ( iBestIndex != label )
		{
			// pattern was mis-recognized.  Notify the testing dialog
			
			// lParam is built to contain a coded bit pattern, as follows:
			//
			//  0          1          2         3
			//  0123456 7890123 456789012345678901
			// |  act  |  tar  |    pattern num   |
			//
			// where act == actual output of the neural net, and tar == target
			// this gives 2^7 = 128 possible outputs (only 10 are needed here... future expansion??)
			// and 2^18 = 262144 possible pattern numbers ( only 10000 are needed here )
			
			code  = ( iPatNum    & 0x0003FFFF );
			code |= ( label      & 0x0000007F ) << 18;
			code |= ( iBestIndex & 0x0000007F ) << 25;
			
			if ( pThis->m_hWndForTestingPosting != NULL )
			{
				::PostMessage( pThis->m_hWndForTestingPosting, UWM_TESTING_NOTIFICATION, 2L, (LPARAM)code );
			}
		}
		
	}


	// post the total MSE of tested patterns to the hwnd

	double divisor = (double)( (iPatternsProcessed>1) ? iPatternsProcessed : 1 );
	dTotalMSE /= divisor;
	scaledMSE = (UINT)( sqrt( dTotalMSE ) * 2.0e8 );  // arbitrary large pre-agreed upon scale factor; taking sqrt is simply to improve the scaling
		
	if ( pThis->m_hWndForTestingPosting != NULL )
	{
		::PostMessage( pThis->m_hWndForTestingPosting, UWM_TESTING_NOTIFICATION, 4L, (LPARAM)scaledMSE );
	}
	
	return 0L;
	
}


#undef UNIFORM_ZERO_THRU_ONE
