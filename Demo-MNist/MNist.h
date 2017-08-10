// MNist.h : main header file for the MNIST application
//

#if !defined(AFX_MNIST_H__905CA67E_F9A3_448F_9B1D_0210659E4E2A__INCLUDED_)
#define AFX_MNIST_H__905CA67E_F9A3_448F_9B1D_0210659E4E2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "Preferences.h"	// Added by ClassView


///////////////////////////////////////////////////////////////////
//
// private message number

const UINT UWM_BACKPROPAGATION_NOTIFICATION = ::RegisterWindowMessage( 
		_T("UWM_BACKPROPAGATION_NOTIFICATION-{F4BB541B-473D-46d3-B8D0-2C8CA6058769}") );  



const UINT UWM_TESTING_NOTIFICATION = ::RegisterWindowMessage( 
		_T("UWM_TESTING_NOTIFICATION-{86B3253A-79DF-4f7a-87A7-D99E4B275B14}") );  


//////////////////////////////////////////////////////////////////////////
//
// declare global GetPreferences function

const CPreferences& GetPreferences();


///////////////////////////////////////////////////////////////////////////
//
// declare global SetThreadName function

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);


///////////////////////////////////////////////////////////////////////
//
// global function to support 64-bit atomic compare-and-exchange
// Probably will work only on Intel and AMD products, and no others
// Needed since the windows API provides an InterlockedCompareExchange64 function only for 
// "Vista" and higher, and because I do not have access to the VC++ 2005 compiler
// intrinsic _InterlockedCompareExchange64.
// See my newsgroup post:
// "InterlockedCompareExchange64 under VC++ 6.0: in-line assembly using cmpxchg8b ??"
// at:
// http://groups.google.com/group/comp.programming.threads/browse_thread/thread/1c3b38cd249ff2ba/e90ff2c919f84612
//
// The following code was obtained from:
// http://www.audiomulch.com/~rossb/code/lockfree/ATOMIC.H

#pragma warning(push)
#pragma warning(disable : 4035) // disable no-return warning

inline unsigned __int64 
_InterlockedCompareExchange64(volatile unsigned __int64 *dest,
                           unsigned __int64 exchange,
                           unsigned __int64 comparand) 
{
    //value returned in eax::edx
    __asm {
        lea esi,comparand;
        lea edi,exchange;
        
        mov eax,[esi];
        mov edx,4[esi];
        mov ebx,[edi];
        mov ecx,4[edi];
        mov esi,dest;
        //lock CMPXCHG8B [esi] is equivalent to the following except
        //that it's atomic:
        //ZeroFlag = (edx:eax == *esi);
        //if (ZeroFlag) *esi = ecx:ebx;
        //else edx:eax = *esi;
        lock CMPXCHG8B [esi];			
    }
}

#pragma warning(pop)


/////////////////////////////////////////////////////////////////////////////
// CMNistApp:
// See MNist.cpp for the implementation of this class
//

class CMNistApp : public CWinApp
{
public:
	CPreferences m_Preferences;
	CString m_sModulePath;
	CMNistApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMNistApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CMNistApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MNIST_H__905CA67E_F9A3_448F_9B1D_0210659E4E2A__INCLUDED_)
