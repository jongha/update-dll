// ItemListAgent.h: interface for the CItemListAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMLISTAGENT_H__769B01FF_CA7A_41F5_AD92_929FF2774D12__INCLUDED_)
#define AFX_ITEMLISTAGENT_H__769B01FF_CA7A_41F5_AD92_929FF2774D12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UpdateUtil.h"

typedef struct _DOWNLOAD_PARAMS
{
	CUpdateUtil* pUtil;
	UPDATE_TRANSNOTIFY* pNotiInfo;
	CItemList*   pSoruceItemList;
	CItemList*   pDestItemList;
	BOOL*		 pbComplete;
	BOOL*		 pbSuccess;
	CString*	 pstrLoginId;
	CString*	 pstrLoginPassword;

} DOWNLOAD_PARAMS, *PDOWNLOAD_PARAMS;

class CItemListAgent  
{
public:
	CItemListAgent();
	virtual ~CItemListAgent();

	BOOL	Start(PUPDATE_TRANSINFO pInfo, BOOL *pbCancel);
	void	RemoveAll();

//DEL 	void	SetPath(CString newpath)	{ m_strFile = newpath; };
//DEL 	CString GetPath()					{ return m_strFile; };

protected:
	CPtrArray	m_PtrArray;
//	CString		m_strFile;
protected:
	BOOL		FindAndDownloadDelete(HWND pParentWnd);
	CItemList*	FindItemList(HWND pParentWnd, int nKind = UPDATE_KIND_SOURCE);
	void		SetError(PUPDATE_TRANSINFO pInfo, int nErr);
	void		DoEvent(){
					MSG message;
					if(::PeekMessage(&message,NULL,0,0,PM_REMOVE)) {
						::TranslateMessage(&message);
						::DispatchMessage(&message); 
					}
				}

};

#endif // !defined(AFX_ITEMLISTAGENT_H__769B01FF_CA7A_41F5_AD92_929FF2774D12__INCLUDED_)
