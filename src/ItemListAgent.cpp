// ItemListAgent.cpp: implementation of the CItemListAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UpdateDLL.h"
#include "ItemListAgent.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemListAgent::CItemListAgent()
{

}

CItemListAgent::~CItemListAgent()
{
	RemoveAll();	
}

void CItemListAgent::RemoveAll()
{
	PTR_ARRAY_REMOVE_ALL(CItemList, &m_PtrArray);
}


BOOL CItemListAgent::FindAndDownloadDelete(HWND pParentWnd)
{
	CItemList* pItemList= NULL;
	BOOL bFind = FALSE;

	for(int i=0; i < m_PtrArray.GetSize(); i++){
		pItemList = (CItemList*)m_PtrArray.GetAt(i);
		if(pItemList->GetParentWnd() == (HWND)pParentWnd){	
			m_PtrArray.RemoveAt(i);
			delete pItemList;
			pItemList = NULL;
			bFind = TRUE;
		}
	}
	
	return bFind;
}

CItemList* CItemListAgent::FindItemList(HWND pParentWnd, int nKind)
{
	CItemList* pItemList= NULL;

	for(int i=0; i < m_PtrArray.GetSize(); i++){
		pItemList = (CItemList*)m_PtrArray.GetAt(i);
		if(pItemList->GetParentWnd() == (HWND)pParentWnd){
			if(pItemList->GetKind() == nKind){
				return pItemList;
			}
		}
	}
	
	return 	NULL;
}

static UINT DownloadThread(LPVOID lParam)
{
	PDOWNLOAD_PARAMS pParams = (DOWNLOAD_PARAMS*)lParam;
	if(pParams->pstrLoginId == NULL || pParams->pstrLoginPassword == NULL)
		(*(pParams->pbSuccess)) = ((CUpdateUtil*)pParams->pUtil)->Download(pParams->pNotiInfo, pParams->pSoruceItemList, pParams->pDestItemList);
	else
		(*(pParams->pbSuccess)) = ((CUpdateUtil*)pParams->pUtil)->DownloadEx(pParams->pNotiInfo, pParams->pSoruceItemList, pParams->pDestItemList, (CString)(*(pParams->pstrLoginId)), (CString)(*(pParams->pstrLoginPassword)));

	(*(pParams->pbComplete)) = TRUE;
	return 0;
}

BOOL CItemListAgent::Start(PUPDATE_TRANSINFO pInfo, BOOL *pbCancel)
{
	TRACE("----------------------------------UPDATE START----------------------------------\n");
	if(!(pInfo->hNotifyWnd) || !(pInfo->lpszDestXmlFile) || !(pInfo->lpszSrcURL)){
		SetError(pInfo, UPDATE_ERROR_INFORMATION);
		return FALSE;
	}

	CString strLoginId = "";
	CString strLoginPassword = "";

	if(pInfo->lpszURLLoginId)
		strLoginId = pInfo->lpszURLLoginId;
	if(pInfo->lpszURLLoginPassword)
		strLoginPassword = pInfo->lpszURLLoginPassword;

	pInfo->nErrno = UPDATE_ERROR_NONE;

	FindAndDownloadDelete(pInfo->hNotifyWnd);
	
	//init notify info
	UPDATE_TRANSNOTIFY NotiInfo;
	memset(&NotiInfo, 0, sizeof(UPDATE_TRANSNOTIFY));
	NotiInfo.pInfo = pInfo;
	NotiInfo.nErrno = UPDATE_ERROR_NONE;
	NotiInfo.lParam = pInfo->lParam;

	BOOL retval = FALSE;
	CString strSrcURL;
	CString strSrcXMLdoc, strDestXMLdoc;

	strSrcURL.Format("%s", pInfo->lpszSrcURL);
	
	CANCEL();
	CUpdateUtil UpdateUtil(pbCancel);
	CString strDownloadedXMLfile = "";

	BOOL bDownXML = FALSE;
	if(strLoginId == "" || strLoginPassword == ""){
		if(UpdateUtil.DownloadXMLfile(strSrcURL, strSrcXMLdoc, strDownloadedXMLfile))
			bDownXML = TRUE;
	}
	else{
		if(UpdateUtil.DownloadXMLfileEx(strSrcURL, strSrcXMLdoc, strDownloadedXMLfile, strLoginId, strLoginPassword))
			bDownXML = TRUE;
	}
	
	if(!bDownXML){
		if(GetLastError() == UPDATE_ERROR_FAIL_DOWNLOADXMLFILE)
			SetError(pInfo, UPDATE_ERROR_FAIL_DOWNLOADXMLFILE);
		else
			SetError(pInfo, UPDATE_ERROR_FILE_NOT_FOUND);
		return retval;
	}
		
	CItemList* pItemListSource = new CItemList(pInfo->hNotifyWnd, UPDATE_KIND_SOURCE);
	if(!pItemListSource){
		SetError(pInfo, UPDATE_ERROR_CANT_MAKE_ITEMLIST_SOURCE);
		return retval;
	}

	m_PtrArray.Add(pItemListSource);
	
	CANCEL();
	if(!UpdateUtil.ReadXML(strSrcXMLdoc, pItemListSource, UPDATE_KIND_SOURCE)){
		SetError(pInfo, UPDATE_ERROR_CANT_READ_XML_SOURCE);
		return retval;
	}

	CItemList* pItemListDest = new CItemList(pInfo->hNotifyWnd, UPDATE_KIND_DEST);
	if(!pItemListDest){
		SetError(pInfo, UPDATE_ERROR_CANT_MAKE_ITEMLIST_DEST);
		return retval;
	}

	m_PtrArray.Add(pItemListDest);
	
	CANCEL();
	CString strDestFilePath = pInfo->lpszDestXmlFile;
	int nFind = UpdateUtil.CheckFile(strDestFilePath);
	
	if(nFind == UPDATE_FILECHECK_RESULT_ERROR){
		SetError(pInfo, UPDATE_ERROR_CANT_MAKE_XML_FILE);
		return retval;
	}
	else{
		if(!UpdateUtil.OpenXML(strDestFilePath, strDestXMLdoc)){
			SetError(pInfo, UPDATE_ERROR_FILE_NOT_FOUND);
			return retval;
		}
	}

	CANCEL();
	UpdateUtil.ReadXML(strDestXMLdoc, pItemListDest, UPDATE_KIND_DEST);
/*	if(!UpdateUtil.ReadXML(strDestXMLdoc, pItemListDest, UPDATE_KIND_DEST)){
		SetError(pInfo, UPDATE_ERROR_CANT_READ_XML_DEST);
		return retval;
	}
*/

	CItemList* pItemListSourceCmp = NULL;
	CItemList* pItemListDestCmp = NULL; 

	pItemListSourceCmp = pItemListSource;
	pItemListDestCmp = pItemListDest;
	
	CANCEL();
	UpdateUtil.CompareFileVersion(pItemListSourceCmp, pItemListDestCmp);
	
	
/*
	DOWNLOAD_PARAMS DownloadParams;
	ZeroMemory(&DownloadParams,sizeof(DOWNLOAD_PARAMS));
	
	CANCEL();
	BOOL	bComplete = FALSE;
	DownloadParams.pbComplete =  &bComplete;
	DownloadParams.pUtil = &UpdateUtil;
	DownloadParams.pNotiInfo = &NotiInfo;
	DownloadParams.pSoruceItemList = pItemListSource;
	DownloadParams.pDestItemList = pItemListDest;
	DownloadParams.pbSuccess = &retval;

	CWinThread* pThread = NULL;
	pThread = AfxBeginThread((AFX_THREADPROC)DownloadThread, (LPVOID)&DownloadParams);
	if(!pThread){
		SetError(pInfo, UPDATE_ERROR_CANT_MAKE_DOWNLOAD_THREAD);
		return retval;
	}
	
	while(!bComplete){
		DoEvent();
//		Sleep(100);
	}
*/

///*
	CANCEL();
	retval = UpdateUtil.Download(&NotiInfo, pItemListSource, pItemListDest);
//*/	
	if(retval)
		SetError(pInfo, UPDATE_SUCCESS);
	else
		SetError(pInfo, NotiInfo.nErrno);

	UpdateUtil.DeleteFile(strDownloadedXMLfile);
	TRACE("----------------------------------UPDATE END----------------------------------\n");

	return retval;
}

void CItemListAgent::SetError(PUPDATE_TRANSINFO pInfo, int nErr)
{
	if(pInfo)
		pInfo->nErrno = nErr;

	SetLastError(nErr);
}
