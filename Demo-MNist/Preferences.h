// Preferences.h: interface for the CPreferences class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PREFERENCES_H__47D8F8EB_8E88_4B05_B51E_14332D5DD2EC__INCLUDED_)
#define AFX_PREFERENCES_H__47D8F8EB_8E88_4B05_B51E_14332D5DD2EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxcoll.h>  // for CStringList


class CPreferences  
{
public:
	int m_cNumBackpropThreads;

	int m_nMagicTrainingLabels;
	int m_nMagicTrainingImages;

	UINT m_nItemsTrainingLabels;
	UINT m_nItemsTrainingImages;

	int m_cNumTestingThreads;

	int m_nMagicTestingLabels;
	int m_nMagicTestingImages;

	UINT m_nItemsTestingLabels;
	UINT m_nItemsTestingImages;

	int m_nRowsImages;
	int m_nColsImages;

	int m_nMagWindowSize;
	int m_nMagWindowMagnification;

	double m_dInitialEtaLearningRate;
	double m_dLearningRateDecay;
	double m_dMinimumEtaLearningRate;
	UINT m_nAfterEveryNBackprops;

	// for limiting the step size in backpropagation, since we are using second order
	// "Stochastic Diagonal Levenberg-Marquardt" update algorithm.  See Yann LeCun 1998
	// "Gradianet-Based Learning Applied to Document Recognition" at page 41

	double m_dMicronLimitParameter;
	UINT m_nNumHessianPatterns;

	// for distortions of the input image, in an attempt to improve generalization

	double m_dMaxScaling;  // as a percentage, such as 20.0 for plus/minus 20%
	double m_dMaxRotation;  // in degrees, such as 20.0 for plus/minus rotations of 20 degrees
	double m_dElasticSigma;  // one sigma value for randomness in Simard's elastic distortions
	double m_dElasticScaling;  // after-smoohting scale factor for Simard's elastic distortions
	
	void ReadIniFile(CWinApp* pApp);
	CWinApp* m_pMainApp;
	CPreferences();
	virtual ~CPreferences();

protected:
	void Get( LPCTSTR strSection, LPCTSTR strEntry, UINT& uiVal );
	void Get( LPCTSTR strSection, LPCTSTR strEntry, int &iVal );
	void Get( LPCTSTR strSection, LPCTSTR strEntry, float &fVal );
	void Get( LPCTSTR strSection, LPCTSTR strEntry, double &dVal );
	void Get( LPCTSTR strSection, LPCTSTR strEntry, LPTSTR pStrVal );
	void Get( LPCTSTR strSection, LPCTSTR strEntry, CString &strVal );
	void Get( LPCTSTR strSection, LPCTSTR strEntry, bool &bVal );

};

#endif // !defined(AFX_PREFERENCES_H__47D8F8EB_8E88_4B05_B51E_14332D5DD2EC__INCLUDED_)
