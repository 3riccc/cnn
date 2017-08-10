// MNistDoc.h : interface of the CMNistDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MNISTDOC_H__33D2ECF2_FA6C_44B8_A44A_004C2D073AA6__INCLUDED_)
#define AFX_MNISTDOC_H__33D2ECF2_FA6C_44B8_A44A_004C2D073AA6__INCLUDED_


// disable the template warning C4786 : identifier was truncated to '255' characters in the browser information

#pragma warning( push )
#pragma warning( disable : 4786 )

#include "NeuralNetwork.h"	// Added by ClassView

using namespace std;

#include <vector>

#include <afxmt.h>  // for critical section, multi-threaded etc

#define GAUSSIAN_FIELD_SIZE ( 21 )  // strictly odd number

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CMNistDoc : public COleDocument
{
protected: // create from serialization only
	CMNistDoc();
	DECLARE_DYNCREATE(CMNistDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMNistDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	//}}AFX_VIRTUAL

// Implementation
public:

	void ApplyDistortionMap( double* inputVector );
	void GenerateDistortionMap( double severityFactor = 1.0 );
	double* m_DispH;  // horiz distortion map array
	double* m_DispV;  // vert distortion map array
	double m_GaussianKernel[ GAUSSIAN_FIELD_SIZE ] [ GAUSSIAN_FIELD_SIZE ];

	int m_cCols;  // size of the distortion maps
	int m_cRows;
	int m_cCount;
	inline double& At( double* p, int row, int col )  // zero-based indices, starting at bottom-left
		{ int location = row * m_cCols + col;
		  ASSERT( location>=0 && location<m_cCount && row<m_cRows && row>=0 && col<m_cCols && col>=0 );
		  return p[ location ];
		}


	double GetCurrentEta();
	double GetPreviousEta();
	void BackpropagateNeuralNet(double *inputVector, int iCount, double* targetOutputVector, 
		double* actualOutputVector, int oCount, 
		std::vector< std::vector< double > >* pMemorizedNeuronOutputs, 
		BOOL bDistort );	
	void CalculateNeuralNet(double* inputVector, int count, double* outputVector = NULL, 
		int oCount = 0, std::vector< std::vector< double > >* pNeuronOutputs = NULL, BOOL bDistort = FALSE );
	void CalculateHessian();
	HANDLE m_utxNeuralNet;  // mutex to guard access to neural net; the MFC CMutex is not necessarily trustworthy

	
	// backpropagation and training-related members

	volatile UINT m_cBackprops;
	volatile BOOL m_bNeedHessian;
	
	HWND m_hWndForBackpropPosting;
	UINT m_nAfterEveryNBackprops;
	double m_dEtaDecay;
	double m_dMinimumEta;
	volatile double m_dEstimatedCurrentMSE;  // this number will be changed by one thread and used by others

	static UINT m_iBackpropThreadIdentifier;  // static member used by threads to identify themselves
	
	UINT m_iNumBackpropThreadsRunning;
	CWinThread* m_pBackpropThreads[100];
	BOOL m_bDistortTrainingPatterns;
	BOOL m_bBackpropThreadsAreRunning;
	volatile BOOL m_bBackpropThreadAbortFlag;
	void StopBackpropagation();
	BOOL StartBackpropagation(UINT iStartPattern = 0, UINT iNumThreads = 2, HWND hWnd = NULL,
			double initialEta = 0.005, double minimumEta = 0.000001, double etaDecay = 0.990,
			UINT nAfterEvery = 1000, BOOL bDistortPatterns = TRUE, double estimatedCurrentMSE = 1.0 );
	static UINT BackpropagationThread( LPVOID pVoid);

	CRITICAL_SECTION m_csTrainingPatterns;  // critical section for "get next pattern"-like operations; the MFC CCriticalSection is only marginally more convenient

	volatile UINT m_iNextTrainingPattern;
	volatile UINT m_iRandomizedTrainingPatternSequence[ 60000 ];
	void RandomizeTrainingPatternSequence();
	UINT GetCurrentTrainingPatternNumber( BOOL bFromRandomizedPatternSequence = FALSE );
	void GetTrainingPatternArrayValues( int iNumImage = 0, unsigned char* pArray = NULL, int* pLabel = NULL, BOOL bFlipGrayscale = TRUE );
	UINT GetNextTrainingPattern(unsigned char* pArray = NULL, int* pLabel = NULL, BOOL bFlipGrayscale = TRUE,
		BOOL bFromRandomizedPatternSequence = TRUE, UINT* iSequenceNum = NULL );
	UINT GetRandomTrainingPattern(unsigned char* pArray=NULL, int* pLabel=NULL, BOOL bFlipGrayscale=TRUE);


	// testing-related members

	UINT m_iNumTestingThreadsRunning;
	CWinThread* m_pTestingThreads[100];
	BOOL m_bDistortTestingPatterns;
	BOOL m_bTestingThreadsAreRunning;
	HWND m_hWndForTestingPosting;
	UINT m_iWhichImageSet;  // 0 == training set; 1 == testing set (which is the default

	static UINT m_iTestingThreadIdentifier;  // static member used by threads to identify themselves

	static UINT TestingThread( LPVOID pVoid );
	void StopTesting();
	BOOL StartTesting( UINT iStartingPattern, UINT iNumThreads, HWND hWnd, BOOL bDistortPatterns, UINT iWhichImageSet = 1 );
	volatile BOOL m_bTestingThreadAbortFlag;


	CRITICAL_SECTION m_csTestingPatterns;  // critical section for "get next pattern"-like operations; the MFC CCriticalSection is only marginally more convenient
	
	volatile UINT m_iNextTestingPattern;
	UINT GetNextTestingPatternNumber();
	void GetTestingPatternArrayValues( int iNumImage = 0, unsigned char* pArray = NULL, int* pLabel = NULL, BOOL bFlipGrayscale = TRUE );
	UINT GetNextTestingPattern(unsigned char* pArray=NULL, int* pLabel=NULL, BOOL bFlipGrayscale=TRUE);  // returns TRUE to signify roll-over back to zero-th pattern
	
	CFile m_fileTrainingLabels;
	CFile m_fileTrainingImages;
	CFile m_fileTestingLabels;
	CFile m_fileTestingImages;
	void CloseMnistFiles();
	BOOL m_bFilesOpen;
	virtual ~CMNistDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	NeuralNetwork m_NN;

protected:


	// CAutoCS: a helper class for automatically locking and unlocking a critical section

	class CAutoCS
	{
	public:
		CAutoCS(CRITICAL_SECTION& rcs) : m_rcs(rcs)
		{ ::EnterCriticalSection( &m_rcs ); }
		virtual ~CAutoCS() { ::LeaveCriticalSection( &m_rcs ); }
	private:
		CRITICAL_SECTION& m_rcs;
	};		// class CAutoCS


	// class CAutoMutex: a helper class for automatically obtaining and releasing ownership of a mutex

	class CAutoMutex
	{
	public:
		CAutoMutex( HANDLE& rutx ) : m_rutx( rutx )
		{  ASSERT( rutx != NULL );
		   DWORD dwRet = ::WaitForSingleObject( m_rutx, INFINITE );
		   ASSERT( dwRet == WAIT_OBJECT_0 );
		}
		virtual ~CAutoMutex() { ::ReleaseMutex( m_rutx ); }
	private:
		HANDLE& m_rutx;
	};     // CAutoMutex



// Generated message map functions
protected:
	//{{AFX_MSG(CMNistDoc)
	afx_msg void OnButtonOpenMnistFiles();
	afx_msg void OnButtonCloseMnistFiles();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// re-enable warning C4786 re : identifier was truncated to '255' characters in the browser information

#pragma warning( pop )


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MNISTDOC_H__33D2ECF2_FA6C_44B8_A44A_004C2D073AA6__INCLUDED_)
