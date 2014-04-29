// UpdateDLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "UpdateDLL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateDLLApp

BEGIN_MESSAGE_MAP(CUpdateDLLApp, CWinApp)
	//{{AFX_MSG_MAP(CUpdateDLLApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateDLLApp construction

CUpdateDLLApp::CUpdateDLLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUpdateDLLApp object

CUpdateDLLApp theApp;
CItemListAgent g_ItemListAgent;

BOOL CUpdateDLLApp::InitInstance() 
{

	g_ItemListAgent.RemoveAll();

	return CWinApp::InitInstance();
}

int CUpdateDLLApp::ExitInstance() 
{
	
	g_ItemListAgent.RemoveAll();
	
	return CWinApp::ExitInstance();
}

UPDATE_DLLEXPORT 
BOOL WINAPI StartUpdate(PUPDATE_TRANSINFO pInfo, BOOL *pbCancel)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (g_ItemListAgent.Start(pInfo, pbCancel));
}
