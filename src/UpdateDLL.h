// UpdateDLL.h : main header file for the UPDATEDLL DLL
//

#if !defined(AFX_UPDATEDLL_H__00E2473A_F344_4A54_930F_DE61C03E6CD3__INCLUDED_)
#define AFX_UPDATEDLL_H__00E2473A_F344_4A54_930F_DE61C03E6CD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "ItemListAgent.h"

/////////////////////////////////////////////////////////////////////////////
// CUpdateDLLApp
// See UpdateDLL.cpp for the implementation of this class
//

class CUpdateDLLApp : public CWinApp
{
public:
	CUpdateDLLApp();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateDLLApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CUpdateDLLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEDLL_H__00E2473A_F344_4A54_930F_DE61C03E6CD3__INCLUDED_)
