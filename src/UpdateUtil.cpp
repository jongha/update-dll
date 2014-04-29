// UpdateUtil.cpp: implementation of the CUpdateUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UpdateDLL.h"
#include "UpdateUtil.h"
#include "HttpFileDownload.h"
#include "HttpFileCheck.h"
#include "./include/Markup.h"
#include "./include/Tokenizer.h"
#include <ole2.h>
#include <comdef.h>
#include "MD5.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UPDATE_FLAG_NORMAL		1
#define UPDATE_FLAG_UPDATE		2
#define UPDATE_FLAG_DELETE		3
#define UPDATE_FLAG_NEW			4
#define UPDATE_FLAG_DIFFERENT	5

#define UPDATE_REGSVR			"yes"
#define UPDATE_FORCE			"yes"

#define UPDATE_ERROR				0
#define UPDATE_CURRENTDIRECTORY		1
#define UPDATE_DESKTOP				2
#define UPDATE_MYDOCUMENTS			3
#define UPDATE_PROGRAM_FILES		101
#define UPDATE_SYSTEM				102
#define UPDATE_WINDOWS				103
#define UPDATE_TEMP					104

#define UPDATE_FILE_SIZE_NOT_EXIST	0
#define UPDATE_FILE_SIZE_EXIST		1

#define UTIL_CANCEL()			\
	if((BOOL)(*m_pbCancel)){	\
		return FALSE;			\
	}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUpdateUtil::CUpdateUtil()
{
	m_pbCancel = NULL;
}

CUpdateUtil::CUpdateUtil(BOOL *pbCancel)
{
	m_pbCancel = pbCancel;
}

CUpdateUtil::~CUpdateUtil()
{
	m_pbCancel = NULL;
}


CString CUpdateUtil::GetFolderPath(LPCSTR szPath)
{	
	CString strOriginPath = szPath;
	CString strPath = szPath;
	CString strretval = "";
	CString strReplace = "";

//	strPath.MakeLower();
	
	int nFolder = GetFolderSymbol(strPath, strReplace);
	switch (nFolder)
	{
	case UPDATE_CURRENTDIRECTORY:
		GetCurrentDirectory(strretval);
		break;
	case UPDATE_DESKTOP:
		GetShSpecialDirectory(CSIDL_DESKTOPDIRECTORY, strretval);
		break;
	case UPDATE_MYDOCUMENTS:
		GetShSpecialDirectory(CSIDL_PERSONAL, strretval);
		break;
	case UPDATE_PROGRAM_FILES:
		if(!GetSpecialDirectory(UPDATE_PROGRAM_FILES, strretval))
			strretval = "";
		break;
	case UPDATE_SYSTEM:		
		if(!GetSpecialDirectory(UPDATE_SYSTEM, strretval))
			strretval = "";
		break;
	case UPDATE_WINDOWS:	
		if(!GetSpecialDirectory(UPDATE_WINDOWS, strretval))
			strretval = "";
		break;
	case UPDATE_TEMP:
		if(!GetSpecialDirectory(UPDATE_TEMP, strretval))
			strretval = "";
		break;
	case UPDATE_ERROR:
		strretval = strOriginPath;
		break;
	default:
		GetCurrentDirectory(strretval);
		break;
	}

	if(nFolder != UPDATE_ERROR){
		strPath.Replace(strReplace, strretval);
		strretval= strPath;
	}

	return strretval;
}

int CUpdateUtil::GetFolderSymbol(CString& strPath, CString& strReplace)
{
	int retval = 0;
	CString strCompare = strPath;
	CString strCompareOriginal = strPath;
	CString strCompareReplace = "";

	if(*m_pbCancel)
	{		
		retval = UPDATE_ERROR;
		return retval;
	}

	int nStart = strCompare.Find('%');
	if(nStart != -1 && nStart == 0)
	{
		int nEnd = strCompare.Find('%', nStart+1);
		if(nEnd != -1)
		{
			strReplace = strCompare.Left(nEnd+1);
			strCompare = strCompare.Left(nEnd);
			strCompareReplace = strCompare.Right(strCompare.GetLength()-1);
			retval = GetFolder(strCompareReplace);
		}
		else
			retval = UPDATE_ERROR;
	}
	else
	{
		int nFindColon = strCompare.Find(':');
		if(nFindColon !=-1 && nFindColon == 1)
		{
			retval = UPDATE_ERROR;
		}
		else
		{
			strReplace = "%CurrentDirectory%";
			int nFind = strCompare.Find('\\');
			if(nFind != -1 && nFind == 0)
			{
				strPath = strReplace + strCompareOriginal;
			}
			else
			{
				strPath = strReplace + "\\" + strCompareOriginal;		
			}
			retval = UPDATE_CURRENTDIRECTORY;
		}
	}
	
	return retval;
}


int CUpdateUtil::CreateCheck(CString strPath)
{
	if( !CreateDirectory(strPath, 0) )
	{
		int ret = GetLastError();
		if(ret == ERROR_ALREADY_EXISTS)
		{
			return 1;
		}
		return 0;
	}

	return 1;


//	strcat(szExePath, "");
}

bool CUpdateUtil::GetCurrentDirectory(CString &strPath)
{
	char szExePath[MAX_PATH];
	GetModuleFileName(::AfxGetApp()->m_hInstance, szExePath, MAX_PATH);

	CString strtmp;
	strtmp.Format("%s", szExePath);
	strPath = strtmp.Left(strtmp.ReverseFind('\\'));

	return true;
}

bool CUpdateUtil::GetShSpecialDirectory(int nFolder, CString &strPath)
{
///*	
	BOOL			bResult;
	LPITEMIDLIST    pidl;
	LPMALLOC        pMalloc;

	bResult = FALSE;
	if (SUCCEEDED (SHGetMalloc (&pMalloc))) 
	{
		if (SUCCEEDED (SHGetSpecialFolderLocation (NULL, nFolder, &pidl))) 
		{
			bResult = SHGetPathFromIDList (pidl, strPath.GetBuffer(_MAX_PATH));
			strPath.ReleaseBuffer();
			pMalloc->Free (pidl);
		}
		pMalloc->Release ();
	}
	
	if(bResult)		return true;
	else			return false;
//*/
//	TCHAR szPath[MAX_PATH];

/*	
	if(SUCCEEDED(SHGetFolderPath(NULL, nFolder, NULL, 0, szPath)) 
	{
		strPath.Format("%s", szPath);
		return true
	}
	else 
		return false;
*/
/*
	if(SHGetSpecialFolderPath(NULL, szPath, nFolder, FALSE))
	{
		strPath.Format("%s", szPath);
		return true;
	}
	else
		return false;
*/
}


bool CUpdateUtil::GetSpecialDirectory(int nFolder, CString &strPath)
{
	CString strFullPath;
	LPTSTR	lpszSystemInfo;     // pointer to system information string 
	TCHAR	tchBuffer[MAX_PATH] = { 0, };   // buffer for expanded string 
	lpszSystemInfo = tchBuffer; 
	int nSize; 

	switch (nFolder)
	{
	case UPDATE_SYSTEM:
		nSize = ::GetSystemDirectory(lpszSystemInfo, MAX_PATH+1);
		break;
	case UPDATE_WINDOWS:
		nSize = ::GetWindowsDirectory(lpszSystemInfo, MAX_PATH+1);
		break;
	case UPDATE_PROGRAM_FILES:
		nSize = ::GetWindowsDirectory(lpszSystemInfo, MAX_PATH+1);
		break;
	case UPDATE_TEMP:
		TCHAR szTempFilePath[MAX_PATH] = { 0, };
		//TCHAR szTempFileLongPath[MAX_PATH+1];
		DWORD dwPathLength;
		dwPathLength = ::GetTempPath(MAX_PATH, szTempFilePath);
//		::GetFullPathName(szTempFilePath, sizeof(szTempFileLongPath)/sizeof(szTempFileLongPath[0]), szTempFileLongPath, NULL);
//		::GetLongPathName((LPCTSTR)szTempFilePath, (LPTSTR)szTempFileLongPath, sizeof(szTempFileLongPath)/sizeof(szTempFileLongPath[0]));
		GetLongPathNameEx(szTempFilePath, strFullPath);
		strcpy(szTempFilePath, strFullPath);
		lpszSystemInfo = szTempFilePath;
		break;
	}
	
	if(nSize == 0)
	{
		return false;
	}

	strPath = (LPCTSTR)lpszSystemInfo;
	
	if(nFolder == UPDATE_PROGRAM_FILES)
	{
		
		CString strTmp = strPath.Left( strPath.Find( '\\' ) );		// C:\ //
		strTmp += "\\Program Files";
		if(!CreateCheck(strTmp))
		{
			return false;
		}
		strPath = strTmp;
	}

	return true;
}


#define ISKIND(str)		\
	(strReplace.CompareNoCase(#str) == 0)

int CUpdateUtil::GetFolder(CString strReplace)
{
	strReplace.MakeLower();

	if(ISKIND(programfiles) || ISKIND(program files) || ISKIND(program))
	{
		return UPDATE_PROGRAM_FILES;
	}

	if(ISKIND(mydocuments))
	{
		return UPDATE_MYDOCUMENTS;
	}

	if(ISKIND(desktop))
	{
		return UPDATE_DESKTOP;
	}

	if(ISKIND(system) || ISKIND(system32))
	{
		return UPDATE_SYSTEM;
	}

	if(ISKIND(systemroot) || ISKIND(windows) || ISKIND(win) || ISKIND(winnt) || ISKIND(windir))
	{
		return UPDATE_WINDOWS;
	}

	if(ISKIND(temp) || ISKIND(temporary))
	{
		return UPDATE_TEMP;
	}

	return UPDATE_CURRENTDIRECTORY;
}

BOOL CUpdateUtil::DownloadXMLfile(CString& strSrcURL, CString& strXMLdoc, CString& strFilePath)
{
	BOOL retval = FALSE;

	CHttpFileDownload *pHttpFileDownload =NULL;

	UTIL_CANCEL();
	pHttpFileDownload = new CHttpFileDownload(strSrcURL, HTTP_DOWNLOAD_PATH_TEMP);

	TRACE("Download Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownload->Start();
	if (bDownResult)
	{
		pHttpFileDownload->GetDownFilePath(strFilePath);
	}
	else
	{
		PTR_REMOVE(pHttpFileDownload);
		SetLastError(UPDATE_ERROR_FAIL_DOWNLOADXMLFILE);
		return FALSE;
	}

	PTR_REMOVE(pHttpFileDownload);
	
	UTIL_CANCEL();
	return OpenXML(strFilePath, strXMLdoc);
}

BOOL CUpdateUtil::DownloadXMLfileEx(CString& strSrcURL, CString& strXMLdoc, CString& strFilePath, 
									CString& strLoginId, CString& strLoginPassword)
{
	BOOL retval = FALSE;

	CHttpFileDownload *pHttpFileDownload =NULL;

	UTIL_CANCEL();
	pHttpFileDownload = new CHttpFileDownload(strSrcURL, HTTP_DOWNLOAD_PATH_TEMP, strLoginId, strLoginPassword);

	TRACE("Download Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownload->Start();
	if (bDownResult)
	{
		pHttpFileDownload->GetDownFilePath(strFilePath);
	}
	else
	{
		PTR_REMOVE(pHttpFileDownload);
		SetLastError(UPDATE_ERROR_FAIL_DOWNLOADXMLFILE);
		return FALSE;
	}

	PTR_REMOVE(pHttpFileDownload);
	
	UTIL_CANCEL();
	return OpenXML(strFilePath, strXMLdoc);
}

BOOL CUpdateUtil::OpenXML(CString& strSourcePath, CString& strXMLdoc)
{
	UTIL_CANCEL();
	// Load up buffer
	unsigned char* pBuffer = NULL;
	int nFileLen = 0;
	char* pFileName = (LPSTR)(LPCSTR)(LPCTSTR)strSourcePath;

	try
	{
		CFile file(pFileName, CFile::modeRead);
		nFileLen = file.GetLength();

		// Allocate Buffer for Ansi file data
		pBuffer = new unsigned char[nFileLen + 1];
		nFileLen = file.Read(pBuffer, nFileLen);
		file.Close();
		pBuffer[nFileLen] = '\0';
	}
	catch (CFileException*)
	{
		PTR_REMOVE(pBuffer)
		DWORD dwError = GetLastError();
		if(dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND)
		{
			CString strMsg;
			strMsg.Format("'%s'을(를) 찾을 수 없습니다. 경로가 올바른지 확인하신 후 다시 시도해 주세요", pFileName);
			MessageBox(NULL, strMsg, "에러", MB_OK|MB_ICONERROR);
		}
		TRACE("XML file Open Fail : %s\n", strSourcePath);
		SetLastError(UPDATE_ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	strXMLdoc = (char*)pBuffer;
	PTR_REMOVE(pBuffer)
	TRACE("XML file Open Success : %s\n", strSourcePath);
	return TRUE;
}

BOOL CUpdateUtil::CompareFileVersion(CItemList* pItemListSourceCmp, CItemList* pItemListDestCmp)
{
//	UPDATE_DATA* tmpClientSection = NULL;
//	UPDATE_DATA* tmpServerSection = NULL;

//	memset(tmpClientSection, 0, sizeof(UPDATE_DATA));
//	memset(tmpServerSection, 0, sizeof(UPDATE_DATA));
	
	CPtrList* pPLClient = pItemListDestCmp->GetPtrList();
	CPtrList* pPLServer = pItemListSourceCmp->GetPtrList();
	BOOL bExist = FALSE;
	UPDATE_DATA *psiServer = NULL;
	UPDATE_DATA *psiClient = NULL;
	CString strFilePath = "";

	UTIL_CANCEL();
	if(pPLClient->GetCount() == 0)
	{
		// Server xml File check
		POSITION posServer= pPLServer->GetHeadPosition();
		psiServer = NULL;
		strFilePath = "";
		while(posServer)
		{	
			bExist = FALSE;

			psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
			strFilePath.Format("%s\\%s", psiServer->DownPath, psiServer->Name);

			switch (VersionCheck(strFilePath, "0", psiServer->Version, psiServer->DownURL, psiServer->CheckSum, psiServer->Size))
			{
			case UPDATE_FLAG_NORMAL:
				TRACE("%s File : Do not anything  because %s == %s\n", strFilePath, psiServer->Version, psiServer->Version);
				psiServer->UpdateFlag = UPDATE_FLAG_NORMAL;
				break;
			case UPDATE_FLAG_NEW:
				TRACE("%s File : New  because not exsist\n", strFilePath);
				psiServer->UpdateFlag = UPDATE_FLAG_NEW;
				break;
			case UPDATE_FLAG_UPDATE:
				TRACE("%s File : Update  because %s < %s\n", strFilePath, psiServer->Version, psiServer->Version);
				psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
				break;
			case UPDATE_FLAG_DIFFERENT:
				TRACE("%s File : Update because different between xml version file and exist file\n", strFilePath);
				psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
				break;
			}
		}
	}
	else
	{
		return	CompareFileVersion2(pItemListSourceCmp, pItemListDestCmp);
	}

	return TRUE;
}
/*			
SET_ITEM_DATA(type, Type, false);
SET_ITEM_DATA(version, Version, true);
SET_ITEM_DATA(downurl, DownURL, true);
SET_ITEM_DATA(downpath, DownPath, false);
SET_ITEM_DATA(description, Description, false);
SET_ITEM_DATA(force, Force, false);
SET_ITEM_DATA(regsrv, RegSrv, false);
*/
BOOL CUpdateUtil::ReadXML(CString& strXMLdoc, CItemList* pSourceItemList, int nKind)
{
	CString strData = "";
	
	if(strXMLdoc.IsEmpty())
		return FALSE;

	CMarkup xml;
	
	if(!xml.SetDoc(strXMLdoc))
		return FALSE;

	xml.ResetPos();

	if(!xml.FindElem(_T("update-data")))
		return FALSE;

	UTIL_CANCEL();
	while(xml.FindChildElem(_T("file")))
	{	
		UTIL_CANCEL();
		strData = "";
		strData = xml.GetChildAttrib(_T("name"));
		
		if(!strData.IsEmpty())
		{
			xml.IntoElem();
			if(nKind == UPDATE_KIND_SOURCE)
			{
				PUPDATE_DATA pFileData = (PUPDATE_DATA)pSourceItemList->GetNewItem();
				memset(pFileData, 0, sizeof(UPDATE_DATA));

				STRCPY(Name);
				if(xml.FindChildElem(_T("type")))
				{
					DATA_INIT_AND_DEPOSIT();
					STRCPY(Type);
				}

				if(xml.FindChildElem(_T("version")))
				{
					DATA_INIT_AND_DEPOSIT();
					STRCPY(Version);
				}
				
				if(xml.FindChildElem(_T("size")))
				{				
					DATA_INIT_AND_DEPOSIT();					
					pFileData->Size = CString2Integer(strData);
				}

				if(xml.FindChildElem(_T("downurl")))
				{				
					DATA_INIT_AND_DEPOSIT();
					STRCPY(DownURL);
				}
				else											
					return FALSE;							

				if(xml.FindChildElem(_T("downpath")))
				{				
					DATA_INIT_AND_DEPOSIT();
					strData = GetFolderPath(strData);
					STRCPY(DownPath);
				}
				
				if(xml.FindChildElem(_T("description")))
				{				
					DATA_INIT_AND_DEPOSIT();
					STRCPY(Description);
				}

				if(xml.FindChildElem(_T("force")))
				{				
					DATA_INIT_AND_DEPOSIT_NOCASE();
					STRCPY(Force);
				}

				if(xml.FindChildElem(_T("regsrv")))
				{				
					DATA_INIT_AND_DEPOSIT_NOCASE();
					STRCPY(RegSrv);
				}

				if(xml.FindChildElem(_T("checksum")))
				{
					DATA_INIT_AND_DEPOSIT_NOCASE();
					STRCPY(CheckSum);
				}
				
				if(xml.FindChildElem(_T("optionflag")))
				{
					DATA_INIT_AND_DEPOSIT();
					pFileData->OptionFlag = CString2Integer(strData);
				}
			}
			else
			{
				PUPDATE_DATA pFileData = (PUPDATE_DATA)pSourceItemList->GetNewItem();
				memset(pFileData, 0, sizeof(UPDATE_DATA));
				
				STRCPY(Name);
				if(xml.FindChildElem(_T("version")))
				{				
					DATA_INIT_AND_DEPOSIT();
					STRCPY(Version);
				}
				else
				{											
					return FALSE;							
				}

				if(xml.FindChildElem(_T("downpath")))
				{				
					DATA_INIT_AND_DEPOSIT();
					strData = GetFolderPath(strData);
					STRCPY(DownPath);
				}
			}
			xml.OutOfElem();
		}
	}
	
	return TRUE;
}


int CUpdateUtil::VersionCheck(LPCSTR lpszFileName, LPCSTR lpszClientVersion, LPCSTR lpszServerVersion, 
							  LPCSTR lpszServerAddress, LPCSTR lpszServerMD5, unsigned long ulServerSize)
{
	CString strFileFullPath = lpszFileName;
	unsigned long ulClientSize = 0;

	// 0. 파일 존재 유무 확인
	// 1. MD5 체크
	// 2. 파일 크기 먼저 확인(0으로 되어 있는 경우도 참고)
	//  2.1 파일 크기가 0으로 되어 있는 경우(솔루션의 v3d 파일 같은 경우 자주 다운 안 받아도 됨)는
	//      파일 크기만 확인하여 업데이트
	// 3. 파일 버전 확인(없는 경우도 참고)

	// 0. file exist check
	if( IsFileExist(strFileFullPath, ulClientSize) ){
		// 1. md5 check
		CString strMD5 = lpszServerMD5;
		if(!strMD5.IsEmpty() && strMD5.GetLength()==32){
			MD5 context(strFileFullPath);
			if( !context.GetHash().IsEmpty() )
				if( 0==context.GetHash().CompareNoCase(strMD5) )
					return UPDATE_FLAG_NORMAL;
				else 
					return UPDATE_FLAG_UPDATE;
		}		

		CString strClientVersion = lpszClientVersion;
		CString strServerVersion = lpszServerVersion;

		// 2. file size check
		if( ulServerSize == UPDATE_FILE_SIZE_NOT_EXIST ){ // 0
			CString strFileAddress = lpszServerAddress;
			DownloadFileSizeCheck(strFileAddress, ulServerSize);
		}

		// 3. version check 
		if(strServerVersion.IsEmpty() || 0==strServerVersion.CompareNoCase("0")){
			// only file size check
			if( (ulClientSize >= ulServerSize) )	return UPDATE_FLAG_NORMAL;
			else /* ulClientSize < ulServerSize*/	return UPDATE_FLAG_UPDATE;
		}
		else{
			char cFileVersion[MAX_PATH];
			
			if( GetFileVersion((LPTSTR)(LPCTSTR)strFileFullPath, cFileVersion) ){
				char cServerVersion[MAX_PATH];
				strcpy(cServerVersion, (LPCSTR)strServerVersion);	

				if( strcmp(cFileVersion, cServerVersion) == 0 ){
					if(ulClientSize>0 && ulServerSize>0){
						if( (ulClientSize >= ulServerSize) )	return UPDATE_FLAG_NORMAL;
						else									return UPDATE_FLAG_UPDATE;
					}
					else
						return UPDATE_FLAG_NORMAL;
				}
				else{
					// 1. Client file version > Server file version : 업데이트 취소
					CUIntArray unarrayClient, unarrayServer;
					
					strClientVersion = cFileVersion;
					strServerVersion = cServerVersion;
					
					StringTokenize(strClientVersion, unarrayClient);
					StringTokenize(strServerVersion, unarrayServer);
					
					return RealVersionCheck(unarrayClient, unarrayServer);
				}
			}
			else{
				if(strClientVersion.IsEmpty() || 0==strClientVersion.CompareNoCase("0")){
					if( (ulClientSize >= ulServerSize) )	return UPDATE_FLAG_NORMAL;
					else									return UPDATE_FLAG_UPDATE;
				}
				else{
					if( 0==strServerVersion.CompareNoCase(strClientVersion) ){
						if( (ulClientSize >= ulServerSize) )	return UPDATE_FLAG_NORMAL;
						else									return UPDATE_FLAG_UPDATE;
					}
					else{
						CUIntArray unarrayClient, unarrayServer;
						
						StringTokenize(strClientVersion, unarrayClient);
						StringTokenize(strServerVersion, unarrayServer);
						
						return RealVersionCheck(unarrayClient, unarrayServer);
					}
				}
			}
		}
	}
	else
		return UPDATE_FLAG_NEW;
}

int	CUpdateUtil::RealVersionCheck(CUIntArray& unarrayClient, CUIntArray& unarrayServer)
{
	int i = 0;
	int count = 0;

	if(unarrayClient.GetSize() == unarrayServer.GetSize()){
		count = unarrayClient.GetSize();
		for(i=0; i <count; i++){
			if(unarrayClient.ElementAt(i) != unarrayServer.ElementAt(i)){
				if(unarrayClient.ElementAt(i) < unarrayServer.ElementAt(i))			return UPDATE_FLAG_UPDATE;
				else if(unarrayClient.ElementAt(i) > unarrayServer.ElementAt(i))	return UPDATE_FLAG_NORMAL;
			}
		}
	}
	else if(unarrayClient.GetSize() < unarrayServer.GetSize()){	// 1.1 : 1.1.0.1
		count = unarrayClient.GetSize();
		for(i=0; i <count; i++){
			if(unarrayClient.ElementAt(i) != unarrayServer.ElementAt(i)){
				if(unarrayClient.ElementAt(i) < unarrayServer.ElementAt(i))			return UPDATE_FLAG_UPDATE;
				else if(unarrayClient.ElementAt(i) > unarrayServer.ElementAt(i))	return UPDATE_FLAG_NORMAL;
			}
		}
		return UPDATE_FLAG_UPDATE;
	}
	else{	// unarrayClient.GetSize() > unarrayServer.GetSize() // 1.1.0.1 : 1.2
		count = unarrayServer.GetSize();
		for(i=0; i <count; i++){
			if(unarrayClient.ElementAt(i) != unarrayServer.ElementAt(i)){
				if(unarrayClient.ElementAt(i) < unarrayServer.ElementAt(i))			return UPDATE_FLAG_UPDATE;
				else if(unarrayClient.ElementAt(i) > unarrayServer.ElementAt(i))	return UPDATE_FLAG_NORMAL;
			}
		}
	}

	return UPDATE_FLAG_NORMAL;
}

BOOL CUpdateUtil::GetFileVersion(LPTSTR lpDestFile, LPTSTR VersionStr)
{
	DWORD dwHdlDest;
	DWORD dwSizeDest;
	DWORD dwDestLS, dwDestMS;
	CString strVersion;
	BOOL retVal = FALSE;	

	dwSizeDest = GetFileVersionInfoSize(lpDestFile, &dwHdlDest);
	
	UTIL_CANCEL();
	if(dwSizeDest){
		_TCHAR* pDestData = new char[dwSizeDest + 1];

		if(GetFileVersionInfo(lpDestFile, dwHdlDest, dwSizeDest, pDestData) ){
			VS_FIXEDFILEINFO * pvsInfo;
			UINT uLen;
			BOOL dwRet;
			dwRet = VerQueryValue(pDestData, _T("\\"), (void**)&pvsInfo, &uLen);
			if(dwRet){
				dwDestMS = pvsInfo->dwFileVersionMS; 
				dwDestLS = pvsInfo->dwFileVersionLS; 

				DWORD dwDestLS1, dwDestLS2, dwDestMS1, dwDestMS2;
				dwDestMS1 = (dwDestMS / 65536);
				dwDestMS2 = (dwDestMS % 65536);
				dwDestLS1 = (dwDestLS / 65536);
				dwDestLS2 = (dwDestLS % 65536);

				wsprintf(VersionStr, "%d.%d.%d.%d", dwDestMS1, dwDestMS2, 
				dwDestLS1, dwDestLS2);

				TRACE("Version information of  %s : %s\n", lpDestFile, VersionStr);
				retVal = TRUE;
			}
		}
		delete [] pDestData;
	}
	return retVal;

}


void CUpdateUtil::StringTokenize(CString& strSource, CUIntArray &UIntDestArray)
{
	CTokenizer tok(strSource, _T(".,;|"));
	CString cs;

	while(tok.Next(cs)){
		UIntDestArray.Add((UINT)atoi((LPCTSTR)cs));
	}
}


BOOL CUpdateUtil::DeleteFile(CString &strFilePath)
{
	// File Struct
	SHFILEOPSTRUCT shStruct;
	TCHAR pszFrom[MAX_PATH]={0};
	TCHAR pszTo[MAX_PATH]={0};

	ZeroMemory(&shStruct,sizeof(SHFILEOPSTRUCT));

	shStruct.fFlags=FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI ;
	shStruct.hwnd=NULL;
	lstrcpy(pszFrom,strFilePath);
	shStruct.pFrom=pszFrom;
	shStruct.pTo=NULL;
	shStruct.wFunc=FO_DELETE;
	
	int retval = SHFileOperation(&shStruct);

	return (retval == 0);
}

BOOL CUpdateUtil::RegSvr32(CString &strFilePath)
{
	if(RegisterOLE(strFilePath,_szDllRegSvr))
		return TRUE;
	else 
		return FALSE;

	return FALSE;
}


bool CUpdateUtil::RegisterOLE(CString pszDllName, CString pszDllEntryPoint)
{
	::CoInitialize(NULL);
	bool bReturn =  false;
	CString strTempFileName = pszDllName;
	strTempFileName.MakeLower();
	if(strTempFileName.Find(".exe") > 0)
	{
		strTempFileName += " /RegServer";
		ShellExecute(NULL, NULL, strTempFileName, NULL, NULL, SW_HIDE);
		bReturn = true;
	}
	else
	{
			
		// Load the library.
		HINSTANCE hLib = LoadLibraryEx(pszDllName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (hLib < (HINSTANCE)HINSTANCE_ERROR) {
			SetLastError(UPDATE_ERROR_DLL_LOAD_LIBRARY);
			TRACE("UPDATE_ERROR_DLL_LOAD_LIBRARY : %s\n", pszDllName);
			goto CleanupOle;
		}

		// Find the entry point.
		FARPROC lpDllEntryPoint;
		(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, pszDllEntryPoint);

		if (lpDllEntryPoint == NULL) {
			SetLastError(UPDATE_ERROR_DLL_GET_PROC_ADDRESS);
			TRACE("UPDATE_ERROR_DLL_GET_PROC_ADDRESS : %s\n", pszDllName);
			goto CleanupLibrary;
		}
		
		// Call the entry point.
		if (lpDllEntryPoint)
		{
			if(Error((*lpDllEntryPoint)()) )
					goto CleanupLibrary;
		}

		SetLastError(UPDATE_SUCCESS_REGSVR_DLL);
		TRACE("UPDATE_SUCCESS_REGSVR_DLL : %s\n", pszDllName);
		
		//Horray
		bReturn = true;

	CleanupLibrary:
			FreeLibrary(hLib);
	    
	CleanupOle:
		OleUninitialize();
	}
	::CoUninitialize();	

    return bReturn;
}
			
bool CUpdateUtil::Error(HRESULT hRes)
{
	if(FAILED(hRes)){
		_com_error e(hRes);
		::MessageBox(NULL, e.ErrorMessage(),  UPDATEDLL ,MB_ICONEXCLAMATION | MB_OK );
		return true;
	}
	return false;
}

BOOL CUpdateUtil::IsFileExist(CString& strDestFile, unsigned long& ulClientSize)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(strDestFile, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
		return FALSE;
	else 
		FindClose(hFind);
/*	
	if(ulClientSize != UPDATE_FILE_SIZE_NOT_EXIST)
		ulClientSize = FindFileData.nFileSizeLow;
	else
		ulClientSize = UPDATE_FILE_SIZE_NOT_EXIST;
*/	
	ulClientSize = FindFileData.nFileSizeLow;
	return TRUE;
}

BOOL CUpdateUtil::MakeClientXMLfile(CItemList *pItemListSoruce, LPCSTR lpszDestFilePath)
{
	if(!pItemListSoruce)
		return FALSE;

	CStdioFile writeFile;
	CString strFilePath = lpszDestFilePath;
	
	BOOL bFind = CheckFolder(strFilePath);
	if(bFind){
		if(!writeFile.Open(lpszDestFilePath, (CFile::modeCreate | CFile::modeWrite | CFile::typeText))){	
			SetLastError(UPDATE_ERROR_CANT_OPEN_XML_FILE);
			TRACE("UPDATE_ERROR_CANT_OPEN_XML_FILE : %s\n", lpszDestFilePath);
			return FALSE;
		}
	}
	else{
		return FALSE;
	}

	UTIL_CANCEL();
	CString strWriteLine, strFileName, strVersion, strDownPath, strCheckSum;
	strWriteLine = "<?xml version=\"1.0\" encoding=\"euc-kr\"?> \n";
	strWriteLine +=	"<update-data> \n\n";

	CPtrList* pPLServer = pItemListSoruce->GetPtrList();
	POSITION posServer = pPLServer->GetHeadPosition();
	while(posServer)
	{	
		UPDATE_DATA *psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		strFileName = psiServer->Name;
		strVersion = psiServer->Version;
		strDownPath = psiServer->DownPath;
		strCheckSum = psiServer->CheckSum;
		strWriteLine +=	"\t<file name=\"" + strFileName + "\"> \n";
		strWriteLine += "\t\t<version>" + strVersion + "</version> \n";
		strWriteLine += "\t\t<downpath>" + strDownPath + "</downpath> \n";
		strWriteLine += "\t\t<checksum>" + strCheckSum + "</checksum> \n";
		strWriteLine += "\t</file> \n\n";
	}
	
	strWriteLine +=	"</update-data>";	
	writeFile.WriteString(strWriteLine);
	writeFile.Close();
//	TRACE("Write Xml File Success \n Contents : \n%s\n", strWriteLine);
	return TRUE;
}

BOOL CUpdateUtil::Download(PUPDATE_TRANSNOTIFY pNotiInfo, CItemList *pItemListSoruce, CItemList *pItemListDest)
{
	CPtrList* pPLClient = pItemListDest->GetPtrList();
	CPtrList* pPLServer = pItemListSoruce->GetPtrList();
	
	UTIL_CANCEL();
	// Server Update check 
	POSITION posServer = pPLServer->GetHeadPosition();
	while(posServer)
	{	
		UPDATE_DATA *psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		
		if(psiServer->UpdateFlag == UPDATE_FLAG_UPDATE || psiServer->UpdateFlag == UPDATE_FLAG_NEW){
			CString strSrcURL = psiServer->DownURL;
			UTIL_CANCEL();
			pNotiInfo->nCurrentSize = 0;
			if(DownloadFileCheck(pNotiInfo, strSrcURL)){
				pNotiInfo->nErrno = UPDATE_ERROR_NONE;
				pNotiInfo->nTotalSize += pNotiInfo->nCurrentSize;
				pNotiInfo->nTotalCount++;
				psiServer->Size = pNotiInfo->nCurrentSize;
			}
		}
	}
	
	UTIL_CANCEL();
	posServer = pPLServer->GetHeadPosition();
	while(posServer)
	{	
		BOOL bExist = FALSE;
		UPDATE_DATA *psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		
		if(psiServer->UpdateFlag == UPDATE_FLAG_UPDATE || psiServer->UpdateFlag == UPDATE_FLAG_NEW){
			CString strSrcURL = psiServer->DownURL;
			CString strDownFolderPath  = psiServer->DownPath;
			UTIL_CANCEL();
			pNotiInfo->lpszName = psiServer->Name;
			pNotiInfo->lpszVersion = psiServer->Version;
			pNotiInfo->lpszDownURL = psiServer->DownURL;
			pNotiInfo->lpszDownPath = psiServer->DownPath;
			pNotiInfo->lpszDescription = psiServer->Description;
			pNotiInfo->nCurrentSize = psiServer->Size;
			pNotiInfo->nCurrentCount++;
			pNotiInfo->nErrno = UPDATE_ERROR_NONE;
			pNotiInfo->nSendPos = 0;
			SendFileNotify(pNotiInfo);
			switch(psiServer->OptionFlag)
			{
			case UPDATE_OPTION_FLAG_RENAME:
				pNotiInfo->pInfo->bNewCreateFile = TRUE;
				break;
			default:
				break;
			}
			
			if(DownloadFile(pNotiInfo, strSrcURL, strDownFolderPath, psiServer->CheckSum)){
				TRACE("Download Complete : %s of %s\n", strSrcURL, psiServer->Name);
				if(strcmp(psiServer->RegSrv, UPDATE_REGSVR) == 0){
					CString strDownFilePath;
					CString strTmp = psiServer->Name;
					strDownFilePath = strDownFolderPath + "\\" + strTmp;
					if(RegSvr32(strDownFilePath))
						TRACE("RegSvr32 Successs : %s\n", psiServer->Name);
					else
						TRACE("RegSvr32 Failed : %s\n", psiServer->Name);
				}
			}
			else{
				TRACE("Download Failed : %s of %s\n", strSrcURL, psiServer->Name);
				return FALSE;
			}
		}
	}
	
	// Client delete check
	POSITION posClient = pPLClient->GetHeadPosition();
	while(posClient)
	{	
		BOOL bExist = FALSE;
		UPDATE_DATA *psiClient = (UPDATE_DATA *)pPLClient->GetNext(posClient);

		if(psiClient->UpdateFlag == UPDATE_FLAG_DELETE){
			UTIL_CANCEL();
			CString strFilePath;
			strFilePath.Format("%s\\%s", psiClient->DownPath, psiClient->Name);
			if(DeleteFile(strFilePath))
				TRACE("Delete Failed : %s\n", strFilePath);
			else
				TRACE("Delete Failed : %s\n", strFilePath);
		}
	}

	return MakeClientXMLfile(pItemListSoruce, pNotiInfo->pInfo->lpszDestXmlFile);
}

BOOL CUpdateUtil::DownloadEx(PUPDATE_TRANSNOTIFY pNotiInfo, CItemList *pItemListSoruce, CItemList *pItemListDest, 
							 CString strLoginId, CString strLoginPassword)
{
	CPtrList* pPLClient = pItemListDest->GetPtrList();
	CPtrList* pPLServer = pItemListSoruce->GetPtrList();
	
	UTIL_CANCEL();
	// Server Update check 
	POSITION posServer = pPLServer->GetHeadPosition();
	while(posServer)
	{	
		UPDATE_DATA *psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		
		if(psiServer->UpdateFlag == UPDATE_FLAG_UPDATE || psiServer->UpdateFlag == UPDATE_FLAG_NEW){
			CString strSrcURL = psiServer->DownURL;
			UTIL_CANCEL();
			pNotiInfo->nCurrentSize = 0;
			if(DownloadFileCheck(pNotiInfo, strSrcURL)){
				pNotiInfo->nErrno = UPDATE_ERROR_NONE;
				pNotiInfo->nTotalSize += pNotiInfo->nCurrentSize;
				pNotiInfo->nTotalCount++;
				psiServer->Size = pNotiInfo->nCurrentSize;
			}
		}
	}
	
	UTIL_CANCEL();
	posServer = pPLServer->GetHeadPosition();
	while(posServer)
	{	
		BOOL bExist = FALSE;
		UPDATE_DATA *psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		
		if(psiServer->UpdateFlag == UPDATE_FLAG_UPDATE || psiServer->UpdateFlag == UPDATE_FLAG_NEW){
			CString strSrcURL = psiServer->DownURL;
			CString strDownFolderPath  = psiServer->DownPath;
			UTIL_CANCEL();
			pNotiInfo->lpszName = psiServer->Name;
			pNotiInfo->lpszVersion = psiServer->Version;
			pNotiInfo->lpszDownURL = psiServer->DownURL;
			pNotiInfo->lpszDownPath = psiServer->DownPath;
			pNotiInfo->lpszDescription = psiServer->Description;
			pNotiInfo->nCurrentSize = psiServer->Size;
			pNotiInfo->nCurrentCount++;
			pNotiInfo->nErrno = UPDATE_ERROR_NONE;
			pNotiInfo->nSendPos = 0;
			SendFileNotify(pNotiInfo);

			switch(psiServer->OptionFlag)
			{
			case UPDATE_OPTION_FLAG_RENAME:
				pNotiInfo->pInfo->bNewCreateFile = TRUE;
				break;
			default:
				break;
			}

			if(DownloadFileEx(pNotiInfo, strSrcURL, strDownFolderPath, strLoginId, strLoginPassword, psiServer->CheckSum)){
				TRACE("Download Complete : %s of %s\n", strSrcURL, psiServer->Name);
				if(strcmp(psiServer->RegSrv, UPDATE_REGSVR) == 0){
					CString strDownFilePath;
					CString strTmp = psiServer->Name;
					strDownFilePath = strDownFolderPath + "\\" + strTmp;
					if(RegSvr32(strDownFilePath))
						TRACE("RegSvr32 Successs : %s\n", psiServer->Name);
					else
						TRACE("RegSvr32 Failed : %s\n", psiServer->Name);
				}
			}
			else{
				TRACE("Download Failed : %s of %s\n", strSrcURL, psiServer->Name);
				return FALSE;
			}
		}
	}
	
	// Client delete check
	POSITION posClient = pPLClient->GetHeadPosition();
	while(posClient)
	{	
		BOOL bExist = FALSE;
		UPDATE_DATA *psiClient = (UPDATE_DATA *)pPLClient->GetNext(posClient);

		if(psiClient->UpdateFlag == UPDATE_FLAG_DELETE){
			UTIL_CANCEL();
			CString strFilePath;
			strFilePath.Format("%s\\%s", psiClient->DownPath, psiClient->Name);
			if(DeleteFile(strFilePath))
				TRACE("Delete Failed : %s\n", strFilePath);
			else
				TRACE("Delete Failed : %s\n", strFilePath);
		}
	}

	return MakeClientXMLfile(pItemListSoruce, pNotiInfo->pInfo->lpszDestXmlFile);
}

BOOL CUpdateUtil::DownloadFile(PUPDATE_TRANSNOTIFY pNotiInfo, CString &strSrcURL, CString& strDownFolderPath, CString strMD5)
{
	BOOL retval = FALSE;
	CHttpFileDownload *pHttpFileDownload =NULL;

	UTIL_CANCEL();
	pHttpFileDownload = new CHttpFileDownload(strSrcURL,  strDownFolderPath, HTTP_DOWNLOAD_PATH_USER, m_pbCancel, pNotiInfo);

	TRACE("Download Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownload->Start();
	if (bDownResult){
		CString strSourcePath;
		pHttpFileDownload->GetDownFilePath(strSourcePath);
#ifdef _DEBUG
		TRACE("Download End to : %s\n", strSourcePath);
#endif
		if(!strMD5.IsEmpty() && strMD5.GetLength()==32){
			MD5 context(strSourcePath);
			if( !context.GetHash().IsEmpty() )
				if( 0==context.GetHash().CompareNoCase(strMD5) )
					retval = TRUE;
		}
		else
			retval = TRUE;
	}

	PTR_REMOVE(pHttpFileDownload);
	return retval;
}

BOOL CUpdateUtil::DownloadFileEx(PUPDATE_TRANSNOTIFY pNotiInfo, CString &strSrcURL, CString& strDownFolderPath, 
								 CString strLoginId, CString strLoginPassword, CString strMD5)
{
	BOOL retval = FALSE;
	CHttpFileDownload *pHttpFileDownload =NULL;

	UTIL_CANCEL();
	pHttpFileDownload = new CHttpFileDownload(strSrcURL, strDownFolderPath, HTTP_DOWNLOAD_PATH_USER, m_pbCancel, pNotiInfo, strLoginId, strLoginPassword);

	TRACE("Download Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownload->Start();
	if (bDownResult){
		CString strSourcePath;
		pHttpFileDownload->GetDownFilePath(strSourcePath);
#ifdef _DEBUG
		TRACE("Download End to : %s\n", strSourcePath);
#endif
		if(!strMD5.IsEmpty() && strMD5.GetLength()==32){
			MD5 context(strSourcePath);
			if( !context.GetHash().IsEmpty() )
				if( 0==context.GetHash().CompareNoCase(strMD5) )
					retval = TRUE;
		}
		else
			retval = TRUE;
	}

	PTR_REMOVE(pHttpFileDownload);
	return retval;
}

BOOL CUpdateUtil::CheckFolder(CString strPath)
{
	CString strFindPath = strPath;
	CString strTmp;
	BOOL bSuccess = TRUE;
	int nFind = 3;
	int nEnd = strPath.ReverseFind('\\');
	
	while(nFind <= nEnd && bSuccess)
	{	
		nFind = strPath.Find('\\', nFind);
		strTmp = strFindPath.Left( nFind );
		bSuccess = CreateCheck(strTmp);
		nFind++;
	}
	
	if(!bSuccess)
		return FALSE;
	
	return TRUE;
}

int CUpdateUtil::CheckFile(CString strPath)
{
	int retval = UPDATE_FILECHECK_RESULT_ERROR;
	
	BOOL bFind = CheckFolder(strPath);
	if(bFind){
		CStdioFile stdioOpenFile;		
		if( !stdioOpenFile.Open(strPath, CFile::modeRead | CFile::typeText) ){
			if(!stdioOpenFile.Open(strPath, CFile::modeCreate | CFile::typeText))
				retval =  UPDATE_FILECHECK_RESULT_ERROR;
			else
				retval = UPDATE_FILECHECK_RESULT_CREATE;
		}
		else{
			retval = UPDATE_FILECHECK_RESULT_EXIST;
		}
		stdioOpenFile.Close();
	}
	else{
		retval = UPDATE_FILECHECK_RESULT_ERROR;
	}

	return retval;
}


BOOL CUpdateUtil::DownloadFileCheck(PUPDATE_TRANSNOTIFY pNotiInfo, CString &strSrcURL)
{
	BOOL retval = FALSE;

	CHttpFileCheck *pHttpFileDownloadCheck =NULL;

	UTIL_CANCEL();
	pHttpFileDownloadCheck = new CHttpFileCheck(strSrcURL, pNotiInfo);

	TRACE("Download File Check Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownloadCheck->Start();
	if (bDownResult){
		TRACE("Download File Check End to : %s\n", strSrcURL);
	}
	else{
		PTR_REMOVE(pHttpFileDownloadCheck);
		return FALSE;
	}

	PTR_REMOVE(pHttpFileDownloadCheck);

	return TRUE;
}

BOOL CUpdateUtil::DownloadFileCheckEx(PUPDATE_TRANSNOTIFY pNotiInfo, CString &strSrcURL, CString strLoginId, CString strLoginPassword)
{
	BOOL retval = FALSE;

	CHttpFileCheck *pHttpFileDownloadCheck =NULL;

	UTIL_CANCEL();
	pHttpFileDownloadCheck = new CHttpFileCheck(strSrcURL, pNotiInfo, strLoginId, strLoginPassword);

	TRACE("Download File Check Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownloadCheck->Start();
	if (bDownResult){
		TRACE("Download File Check End to : %s\n", strSrcURL);
	}
	else{
		PTR_REMOVE(pHttpFileDownloadCheck);
		return FALSE;
	}

	PTR_REMOVE(pHttpFileDownloadCheck);

	return TRUE;
}

int CUpdateUtil::CString2Integer(CString string)
{
	int retval = 0;

	if(string.IsEmpty())
		return retval;

	char cpstr[MAX_PATH];
	strcpy(cpstr, string.GetBuffer(MAX_PATH));
	string.ReleaseBuffer();

	retval = atoi(cpstr);

	return retval;
}

BOOL CUpdateUtil::CompareFileVersion2(CItemList *pItemListSourceCmp, CItemList *pItemListDestCmp)
{
	CPtrList* pPLClient = pItemListDestCmp->GetPtrList();
	CPtrList* pPLServer = pItemListSourceCmp->GetPtrList();
	BOOL bExist = FALSE;
	UPDATE_DATA *psiServer = NULL;
	UPDATE_DATA *psiClient = NULL;
	CString strFilePath = "";

	UTIL_CANCEL();
	// Client xml File check
	POSITION posClient = pPLClient->GetHeadPosition();
	while(posClient)
	{	
		bExist = FALSE;
		psiClient = (UPDATE_DATA *)pPLClient->GetNext(posClient);
		
		UTIL_CANCEL();
		POSITION posServer= pPLServer->GetHeadPosition();
		while(posServer)
		{		
			psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
			if(strcmp(psiClient->Name, psiServer->Name) == 0){
				bExist = TRUE;
				strFilePath.Format("%s\\%s", psiClient->DownPath, psiClient->Name);
				
				UTIL_CANCEL();
				switch (VersionCheck(strFilePath, psiClient->Version, psiServer->Version, psiServer->DownURL, psiServer->CheckSum, psiServer->Size))
				{
				case UPDATE_FLAG_NORMAL:
					TRACE("%s File : Do not anything  because %s == %s\n", strFilePath, psiClient->Version, psiServer->Version);
					psiServer->UpdateFlag = UPDATE_FLAG_NORMAL;
					break;
				case UPDATE_FLAG_NEW:
					TRACE("%s File : New  because not exsist\n", strFilePath);
					psiServer->UpdateFlag = UPDATE_FLAG_NEW;
					break;
				case UPDATE_FLAG_UPDATE:
					TRACE("%s File : Update  because %s < %s\n", strFilePath, psiClient->Version, psiServer->Version);
					psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
					break;
				case UPDATE_FLAG_DIFFERENT:
					TRACE("%s File : Update because different between xml version file and exist file\n", strFilePath);
					psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
					break;
				}
			}
			
			if(bExist)
				break;
		}
		
		if(!bExist){
			TRACE("%s File : Delete\n", psiClient->Name);
			psiClient->UpdateFlag = UPDATE_FLAG_DELETE;
		}
	
	}

	UTIL_CANCEL();
	// Server xml File check
	POSITION posServer= pPLServer->GetHeadPosition();
	while(posServer)
	{	
		bExist = FALSE;
		psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		
		if(strcmp(psiServer->Force, UPDATE_FORCE) == 0){
			TRACE("%s File : Update because forced\n", psiServer->Name);
			psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
			continue;
		}
		
		UTIL_CANCEL();
		POSITION posClient = pPLClient->GetHeadPosition();
		while(posClient)
		{	
			psiClient = (UPDATE_DATA *)pPLClient->GetNext(posClient);	
			if(strcmp(psiClient->Name, psiServer->Name) == 0){
				bExist = TRUE;
				if(psiServer->UpdateFlag == 0){					
					UTIL_CANCEL();
					strFilePath.Format("%s\\%s", psiServer->DownPath, psiServer->Name);
					switch (VersionCheck(strFilePath, psiClient->Version, psiServer->Version, psiServer->DownURL, psiServer->CheckSum, psiServer->Size))
					{
					case UPDATE_FLAG_NORMAL:
						TRACE("%s File : Do not anything  because %s == %s\n", strFilePath, psiClient->Version, psiServer->Version);
						psiServer->UpdateFlag = UPDATE_FLAG_NORMAL;
						break;
					case UPDATE_FLAG_NEW:
						TRACE("%s File : New  because not exsist\n", strFilePath);
						psiServer->UpdateFlag = UPDATE_FLAG_NEW;
						break;
					case UPDATE_FLAG_UPDATE:
						TRACE("%s File : Update  because %s < %s\n", strFilePath, psiClient->Version, psiServer->Version);
						psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
						break;
					case UPDATE_FLAG_DIFFERENT:
						TRACE("%s File : Update because different between xml version file and exist file\n", strFilePath);
						psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
						break;
					}
				}
		
				break;
			}
		}			

		if(!bExist){
			TRACE("%s File : new\n", psiServer->Name);
			psiServer->UpdateFlag = UPDATE_FLAG_NEW;
		}
	}

	// last server update new file check
	UTIL_CANCEL();
	posServer= pPLServer->GetHeadPosition();
	while(posServer)
	{	
		psiServer = (UPDATE_DATA *)pPLServer->GetNext(posServer);
		
		UTIL_CANCEL();
		if(psiServer->UpdateFlag == UPDATE_FLAG_NEW){
			strFilePath.Format("%s\\%s", psiServer->DownPath, psiServer->Name);
			switch (VersionCheck(strFilePath, "0", psiServer->Version, psiServer->DownURL, psiServer->CheckSum, psiServer->Size))
			{
			case UPDATE_FLAG_NORMAL:
				TRACE("%s File : Do not anything  because %s == %s\n", strFilePath, psiServer->Version, psiServer->Version);
				psiServer->UpdateFlag = UPDATE_FLAG_NORMAL;
				break;
			case UPDATE_FLAG_NEW:
				TRACE("%s File : New  because not exsist\n", strFilePath);
				psiServer->UpdateFlag = UPDATE_FLAG_NEW;
				break;
			case UPDATE_FLAG_UPDATE:
				TRACE("%s File : Update  because %s < %s\n", strFilePath, psiServer->Version, psiServer->Version);
				psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
				break;
			case UPDATE_FLAG_DIFFERENT:
				TRACE("%s File : Update because different between xml version file and exist file\n", strFilePath);
				psiServer->UpdateFlag = UPDATE_FLAG_UPDATE;
				break;
			}
		}
	}

	return TRUE;
}

//DEL void CUpdateUtil::SendFileNotify(PUPDATE_TRANSNOTIFY pNotiInfo)
//DEL {
//DEL #ifdef _DEBUG
//DEL 	static UINT nSendCount = 0;
//DEL #endif
//DEL 	if(pNotiInfo->pInfo)
//DEL 	{
//DEL 		if(pNotiInfo->pInfo->hNotifyWnd && pNotiInfo->pInfo->nNotifyMsg)
//DEL 			PostMessage(pNotiInfo->pInfo->hNotifyWnd, pNotiInfo->pInfo->nNotifyMsg, 0, (LPARAM)pNotiInfo);
//DEL #ifdef _DEBUG
//DEL 		TRACE("Send Message Count : %ld\n", nSendCount++);
//DEL #endif
//DEL 		if(pNotiInfo->pInfo->pNotifyFunc)
//DEL 			pNotiInfo->pInfo->pNotifyFunc((LPARAM)pNotiInfo);
//DEL 	}
//DEL }

BOOL CUpdateUtil::DownloadFileSizeCheck(CString &strSrcURL, DWORD &dwFileSize)
{
	BOOL retval = FALSE;

	CHttpFileCheck *pHttpFileDownloadCheck =NULL;

	pHttpFileDownloadCheck = new CHttpFileCheck(strSrcURL);

	TRACE("Download File Check Start : %s\n", strSrcURL);
	BOOL bDownResult = pHttpFileDownloadCheck->Start();
	if (bDownResult){
		TRACE("Download File Check End to : %s\n", strSrcURL);
	}
	else{
		PTR_REMOVE(pHttpFileDownloadCheck);
		return FALSE;
	}
	
	dwFileSize  = pHttpFileDownloadCheck->GetFileSize();
	PTR_REMOVE(pHttpFileDownloadCheck);

	return TRUE;
}
