// HttpFileDownload.cpp: implementation of the CHttpFileDownload class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpFileDownload.h"
#include "ATLBASE.H"	// only CRegKey

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HTTP_CANCEL()			\
	if(m_pbCancel){				\
		if((BOOL)(*m_pbCancel)){\
			return FALSE;		\
		}						\
	}

//#define BUFFER_SIZE 4096
#define BUFFER_SIZE 10240
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHttpFileDownload::CHttpFileDownload()
: m_hInternetSession(NULL)
, m_hHttpConnection(NULL)
, m_hHttpFile(NULL)
, m_strFileURL(_T(""))
, m_nDownMethod(HTTP_DOWNLOAD_PATH_DEFAULT)
, m_pbCancel(NULL)
, m_pNotiInfo(NULL)
, m_strLoginId(_T(""))
, m_strLoginPassword(_T(""))
#ifdef _URLDOWN
//, m_hDownThread(NULL)
#endif // _URLDOWN
{
}

CHttpFileDownload::CHttpFileDownload(CString strFileURL, int nDownMethod, CString strLoginId, CString strLoginPassword)
: m_hInternetSession(NULL)
, m_hHttpConnection(NULL)
, m_hHttpFile(NULL)
, m_strFileURL(_T(""))
, m_nDownMethod(HTTP_DOWNLOAD_PATH_DEFAULT)
, m_pbCancel(NULL)
, m_pNotiInfo(NULL)
, m_strLoginId(strLoginId)
, m_strLoginPassword(strLoginPassword)
#ifdef _URLDOWN
//, m_hDownThread(NULL)
#endif // _URLDOWN
{
	if(strFileURL){
		m_strFileURL = strFileURL;
		m_nDownMethod = nDownMethod;
	}
}

CHttpFileDownload::CHttpFileDownload(CString strFileURL, CString strDownPath, int nDownMethod, BOOL *pbCancel,  PUPDATE_TRANSNOTIFY pNotiInfo, CString strLoginId, CString strLoginPassword)
: m_hInternetSession(NULL)
, m_hHttpConnection(NULL)
, m_hHttpFile(NULL)
, m_strFileURL(_T(""))
, m_nDownMethod(HTTP_DOWNLOAD_PATH_DEFAULT)
, m_pbCancel(pbCancel)
, m_pNotiInfo(pNotiInfo)
, m_strLoginId(strLoginId)
, m_strLoginPassword(strLoginPassword)
#ifdef _URLDOWN
//, m_hDownThread(NULL)
#endif // _URLDOWN
{
	if(strFileURL && strDownPath){
		m_strFileURL = strFileURL;
		m_strDownPath = strDownPath;
		m_nDownMethod = nDownMethod;
	}
}

CHttpFileDownload::~CHttpFileDownload()
{
	//Wait for the worker thread to exit
	if (m_hHttpFile){
		::InternetCloseHandle(m_hHttpFile);
		m_hHttpFile = NULL;
	}
	if (m_hHttpConnection){
		::InternetCloseHandle(m_hHttpConnection);
		m_hHttpConnection = NULL;
	}
	if (m_hInternetSession){
		::InternetCloseHandle(m_hInternetSession);
		m_hInternetSession = NULL;
	}

#ifdef _URLDOWN
//	if(m_hDownThread){
//		::CloseHandle(m_hDownThread);
//		m_hDownThread = NULL;
//	}
#endif // _URLDOWN
}

short CHttpFileDownload::Download()
{
	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  
	ppszAcceptTypes[1] = NULL;
	DWORD dwFlags =INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION;
	
	//Now do the actual read of the file
//	DWORD dwStartTicks = ::GetTickCount();
//	DWORD dwCurrentTicks = dwStartTicks;
	DWORD dwBytesRead = 0;
	DWORD dwTotalBytesRead = 0;
	DWORD dwCurBytesRead = 0;
	DWORD dwLastTotalBytes = 0;
	BOOL bReboot = FALSE;
	CString strTmpDownFilePath;
	
		//Create the Internet session handle
		m_hInternetSession = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);    
		if (m_hInternetSession == NULL){
			SetError(UPDATE_HTTP_FAIL_INTERNET_SESSION);
			return -601;
		}

		//Make the connection to the HTTP server          
		if(m_strLoginId != "" && m_strLoginPassword != ""){
			m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_strServer, INTERNET_INVALID_PORT_NUMBER, m_strLoginId, 
												  m_strLoginPassword, INTERNET_SERVICE_HTTP, 0, (DWORD) this);
		}
		else{
			m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_strServer, INTERNET_INVALID_PORT_NUMBER, NULL, 
												  NULL, INTERNET_SERVICE_HTTP, 0, (DWORD)this);
		}

		if (m_hHttpConnection == NULL){
			SetError(UPDATE_HTTP_FAIL_INTERNET_CONNECTION);
			return -602;
		}

		//Set any CInternetSession options we  may need
		int ntimeOut = 30;
		::InternetSetOption(m_hInternetSession, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)(1000*ntimeOut), 0);
		::InternetSetOption(m_hInternetSession, INTERNET_OPTION_CONNECT_BACKOFF, (void*)(1000), 0);
		::InternetSetOption(m_hInternetSession, INTERNET_OPTION_CONNECT_RETRIES, (void*)(1), 0);
		
		//////////////////////////////////////////////////////////////////////////
		// 2004.11.4 by cabbage
		// 다운로드 중에 InternetReadFile()에서 응답이 없어서 멈춤 해결을 위해 
		// BUFFER_SIZE와 함께 수정
//		::InternetSetOption(m_hInternetSession, INTERNET_OPTION_SEND_TIMEOUT, (void*)(1000*5),0);
//		::InternetSetOption(m_hInternetSession, INTERNET_OPTION_RECEIVE_TIMEOUT, (void*)(1000*5),0);
		//////////////////////////////////////////////////////////////////////////

		m_hHttpFile = ::HttpOpenRequest(m_hHttpConnection, NULL, m_strObject, _T("HTTP/1.1"), NULL, ppszAcceptTypes, dwFlags, (DWORD)this);
		if (m_hHttpFile == NULL){
			SetError(UPDATE_HTTP_FAIL_INTERNET_OPEN_REQUEST);
			return -603;
		}

		//Issue the request
		BOOL bSend = FALSE;
		bSend = ::HttpSendRequest(m_hHttpFile, NULL, 0, NULL, 0);
		if (!bSend){
			TRACE(_T("Failed in call to HttpSendRequest, Error:%d\n"), ::GetLastError());
			SetError(UPDATE_HTTP_FAIL_INTERNET_SEND_REQUEST);
			return -604;
		}

		//Check the HTTP status code
		TCHAR szStatusCode[32];
		DWORD dwInfoSize = 32;
		if (!::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL)){
			TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
			SetError(UPDATE_HTTP_FAIL_INTERNET_QUERY_STATUS_CODE);
			return -605;
		}
		else{
			//Get the HTTP status code
			long nStatusCode = _ttol(szStatusCode);
  			if (nStatusCode == HTTP_STATUS_OK){
				// Get the length of the file.            
				DWORD dwFileSize = 0;
				TCHAR szContentLength[32];
				DWORD dwtmpInfoSize = 32;
				if (::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwtmpInfoSize, NULL)){
					//Set the progress control range
					dwFileSize = (DWORD) _ttol(szContentLength);
				}
				else{
					SetError(UPDATE_HTTP_FAIL_INTERNET_QUERY_CONTENT_LENGTH);
					return -605;
				}
			
				m_strDownFilePath = m_strDownPath + m_strFileName;		// Get Download file name;				

				DWORD dwCurBytesRead=0;
				HANDLE hFile = CreateFile(_T((LPCSTR)m_strDownFilePath),
											GENERIC_WRITE, FILE_SHARE_READ,
											NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE){
					bool bCreateError=false;
					switch(GetLastError())
					{
					case ERROR_ACCESS_DENIED:
					case ERROR_SHARING_VIOLATION:
					case ERROR_LOCK_VIOLATION:	bCreateError = true;	break;
					default:											break;
					}
				
					if(bCreateError){
						bool bSuccess = false;
						if(m_pNotiInfo && m_pNotiInfo->pInfo ){
#ifdef _V3
							if(m_pNotiInfo->pInfo->bNewCreateFile && m_pNotiInfo->pInfo->lpszFileType){
								m_strDownFilePath += ".";
								m_strDownFilePath += m_pNotiInfo->pInfo->lpszFileType;
								hFile = CreateFile(_T((LPCSTR)m_strDownFilePath),
									GENERIC_WRITE, FILE_SHARE_READ,
									NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile != INVALID_HANDLE_VALUE)
									bSuccess = true;
							}
#else // _V3
							if(m_pNotiInfo->pInfo->bNewCreateFile){
								int nCount = 0;
								strTmpDownFilePath = m_strDownFilePath;
								strTmpDownFilePath += UPDATE_RENAME_EXTENTION_NAME;
								while(IsFileExist(strTmpDownFilePath))
								{
									if(DeleteFile(strTmpDownFilePath))
										break;
									else{
										nCount++;
										strTmpDownFilePath.Format("%s%s_%d", m_strDownFilePath, UPDATE_RENAME_EXTENTION_NAME, nCount);
									}
								}

								MoveFile(m_strDownFilePath, strTmpDownFilePath);
								hFile = CreateFile(_T((LPCSTR)m_strDownFilePath),
									GENERIC_WRITE, FILE_SHARE_READ,
									NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile != INVALID_HANDLE_VALUE)
									bSuccess = true;
								else{
									// final reboot mode
									hFile = CreateFile(_T((LPCSTR)strTmpDownFilePath),
										GENERIC_WRITE, FILE_SHARE_READ,
										NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
									if (hFile != INVALID_HANDLE_VALUE){
										bReboot = TRUE;
										bSuccess = true;
									}

								}
							}
#endif // _V3
						}
						
						if(!bSuccess){
#ifdef _V3
							SetError(UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE_HIDE);
#else // _V3
							SetError(UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE);
#endif // _V3
							return -610;
						}
					}
					else{
#ifdef _V3
						SetError(UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE_HIDE);
#else // _V3
						SetError(UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE);
#endif // _V3
						return -610;
					}
				}

#ifdef _URLDOWN
				// 파일을 닫고 다운로드 시작
				if (hFile != INVALID_HANDLE_VALUE)
					CloseHandle(hFile);

				CUrlDownload urldownload(m_pNotiInfo, m_pbCancel);
				HRESULT hRes = ::URLDownloadToFile(NULL, m_strFileURL, m_strDownFilePath, 0, (IBindStatusCallback*)&urldownload);
				
				// 캐시파일을 지운다
				if(!urldownload.m_strCacheFilePath.IsEmpty()) ::DeleteFile(urldownload.m_strCacheFilePath);
				
				if (hRes != S_OK && !(m_pbCancel && *m_pbCancel)){					
					SetError(UPDATE_HTTP_FAIL_INTERNET_READ_FILE);
					return -666;
				}
				else if(m_pbCancel && *m_pbCancel){
					SetError(UPDATE_HTTP_FAIL_CANCEL);
					return -666;
				}

/*				BOOL retval = FALSE;
				DWORD dwExitCode;
				if(m_hDownThread){
					CloseHandle(m_hDownThread);
					m_hDownThread = NULL;
				}
				
				m_hDownThread = ::CreateThread(NULL, 0, DownThreadProc, (void *)this, 0, &dwExitCode);
				// while문을 안쓰고 기다리기 위해서
				// wait until the thread terminates
				if (::GetExitCodeThread(m_hDownThread, &dwExitCode) && dwExitCode == STILL_ACTIVE){
					::WaitForSingleObject(m_hDownThread, INFINITE);
				}

				::GetExitCodeThread(m_hDownThread, &dwExitCode);
				if(m_hDownThread){
					CloseHandle(m_hDownThread);
					m_hDownThread = NULL;
				}
				if(dwExitCode<0)
					return (short)dwExitCode;
*/
#else // _URLDOWN
				CFile myfile((int)hFile);
				//CFile myfile(m_strDownFilePath, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
				DWORD dwStartTicks = ::GetTickCount();
				char szReadBuf[BUFFER_SIZE];
				do
				{
					//OutputDebugString("Packet read Start");
					if (!::InternetReadFile(m_hHttpFile, szReadBuf, BUFFER_SIZE, &dwCurBytesRead)){
						//MessageBox(_T("Failed in call to InternetReadFile, Error:%d\n"), ::GetLastError());
						CloseHandle(hFile);
						SetError(UPDATE_HTTP_FAIL_INTERNET_READ_FILE);
						return -606;
					}
					else{
						//OutputDebugString("Packet read End");
						if(m_pbCancel){
							if((BOOL)(*m_pbCancel)){
								CloseHandle(hFile);
								SetError(UPDATE_HTTP_FAIL_CANCEL);
								return -666;
							}
						}

						if (dwCurBytesRead){
							//Write the data to file
							try{
								myfile.Write(szReadBuf, dwCurBytesRead);	//szReadBuf[dwCurBytesRead] = '\0';
								if(m_pNotiInfo){
									m_pNotiInfo->nSendPos = dwCurBytesRead;
									m_pNotiInfo->nTotalPos += dwCurBytesRead;
									m_pNotiInfo->nRequiredTime = ::GetTickCount() - dwStartTicks;
									//OutputDebugString("Packet Send Start");
									SendFileNotify(m_pNotiInfo);
									//OutputDebugString("Packet Send End");
								}
							}
							catch(CFileException* pEx){
								TRACE(_T("An exception occured while writing to the download file\n"));
								pEx->Delete();
								CloseHandle(hFile);
								SetError(UPDATE_HTTP_FAIL_INTERNET_READ_FILE);
								return -607;
							}
						}
					}
				} 
				while (dwCurBytesRead);
				
				CloseHandle(hFile);
#endif // _URLDOWN
			}
			//Handle any errors
			else if (nStatusCode == HTTP_STATUS_DENIED){
				SetError(UPDATE_HTTP_FAIL_INTERNET_STATUS_DENIED);
				return -608;
			}
			else if (nStatusCode == HTTP_STATUS_NOT_FOUND){
				SetError(UPDATE_HTTP_FAIL_INTERNET_STATUS_NOT_FOUND);
				return -609;
			}
			else{
				SetError(UPDATE_HTTP_FAIL_INTERNET_OTHER_FAIL);
				return -666;
			}
		}

	    //Free up the internet handles we may be using
		if (m_hHttpFile){
			::InternetCloseHandle(m_hHttpFile);
			m_hHttpFile = NULL;
		}
		if (m_hHttpConnection){
			::InternetCloseHandle(m_hHttpConnection);
			m_hHttpConnection = NULL;
		}
		if (m_hInternetSession){
			::InternetCloseHandle(m_hInternetSession);
			m_hInternetSession = NULL;
		}

		if(bReboot)
		{
			m_pNotiInfo->pInfo->nRebootAskResult = IDNO;
			SendRebootAskNotify(m_pNotiInfo);
			if(IDYES==m_pNotiInfo->pInfo->nRebootAskResult){
				if(SetWriteRunOnceAndReboot(strTmpDownFilePath, m_strDownFilePath)){
					SetError(UPDATE_ERROR_NONE_NOT_EXECUTE_AND_REBOOT);
					return -610;
				}
			}
#ifdef _V3
			SetError(UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE_HIDE);
#else // _V3
			SetError(UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE_REBOOT);
#endif // _V3
			return -610;
		}
	//We're finished

	return 1;
}



BOOL CHttpFileDownload::Start()
{	
	HTTP_CANCEL();
	if(ParseURL()){
		HTTP_CANCEL();
		if(GetDownPath() != ""){
			HTTP_CANCEL();
			if(Download() == 1)
				return TRUE;
			else
				return FALSE;
		}
		else{
			SetError(UPDATE_HTTP_FAIL_PARSE_DOWN_PATH);
		}
	}
	else{
		SetError(UPDATE_HTTP_FAIL_PARSE_URL);		
	}

	return FALSE;
}


BOOL CHttpFileDownload::ParseURL()
{	
	if(!m_strFileURL){
		// ErrMsgBox("Failed to read file URL!");
		return FALSE;
	}
	
	DWORD dwServiceType = 0;
	
	if (!AfxParseURL(m_strFileURL, dwServiceType, m_strServer, m_strObject, m_nPort)){
		//Try sticking "http://" before it
		m_strFileURL = _T("http://") + m_strFileURL;
	
		if (!AfxParseURL(m_strFileURL, dwServiceType, m_strServer, m_strObject, m_nPort)){
			TRACE(_T("Failed to parse the URL: %s\n"), m_strFileURL);
			// ErrMsgBox("Failed to parse the URL!");
			return FALSE;
		}
	}

	 //Pull out just the filename component
	int nSlash = m_strObject.ReverseFind(_T('/'));
	if (nSlash == -1)
		nSlash = m_strObject.ReverseFind(_T('\\'));
	if (nSlash != -1 && m_strObject.GetLength() > 1){
		m_strFileName = m_strObject.Right(m_strObject.GetLength() - nSlash - 1);
		//m_strObject = m_strObject.Left(nSlash+1);
	}
	else{
		// ErrMsgBox("Failed to parse the URL!");
		return FALSE;
	}

	return	TRUE;
}


// Find Current Execute Directory
CString CHttpFileDownload::GetExecutePath()
{	
	CString strTmp;
	char	buf[MAX_PATH];
	::GetModuleFileName(::AfxGetApp()->m_hInstance, buf, MAX_PATH);
	
	strTmp.Format("%s", buf);

    m_strExecutePath = strTmp.Left( strTmp.ReverseFind('\\'));
	m_strExecutePath += "\\";

	return m_strExecutePath;
}

CString CHttpFileDownload::GetTempPath()
{	
	TCHAR szTempFilePath[MAX_PATH];
	DWORD dwPathLength;

	dwPathLength = ::GetTempPath(MAX_PATH, szTempFilePath);
	GetLongPathNameEx(szTempFilePath, m_strTempPath);
	return m_strTempPath;
}


BOOL CHttpFileDownload::SetDownPath(CString strDownPath)
{
	int nLength = 0;
	nLength = strDownPath.GetLength();
	nLength -= 1;
	if((nLength != 0) && (strDownPath.ReverseFind('\\') < nLength)){
		m_strDownPath = strDownPath + "\\";
		return TRUE;
	}
	else if((nLength != 0) && (strDownPath.ReverseFind('\\') == nLength)){
		m_strDownPath = strDownPath;
		return TRUE;
	}
	else{
		// ErrMsgBox("Download Folder is not Correct!");
		return FALSE;
	}
	
	return FALSE;
}

BOOL CHttpFileDownload::CheckFolder(CString	strPath)
{
	CString strFindPath = strPath;
	CString strTmp;
	BOOL bSuccess = TRUE;
	int nFind = 3;
	int nEnd = strPath.ReverseFind('\\');
	
	HTTP_CANCEL();
	while(nFind <= nEnd && bSuccess)
	{	
		HTTP_CANCEL();
		nFind = strPath.Find('\\', nFind);
		strTmp = strFindPath.Left( nFind );
		bSuccess = CreateCheck(strTmp);
		nFind++;
	}
	
	if(!bSuccess)
		return FALSE;
	
	return TRUE;
/*
	if(_chdir((LPCSTR)(LPCTSTR)strPath) == 0)
		return TRUE;
	else{
		if(_mkdir((LPCSTR)(LPCTSTR)strPath) == 0)
			return TRUE;
		else{
			// ErrMsgBox("Failed to Create Download Directory!");
			return FALSE;
		}

	}
*/
}


CString CHttpFileDownload::GetDownPath()
{
	switch (m_nDownMethod){
		case HTTP_DOWNLOAD_PATH_EXECUTE:
			m_strDownPath = GetExecutePath();
			break;

		case HTTP_DOWNLOAD_PATH_TEMP:
			m_strDownPath = GetTempPath();
			break;
		
		case HTTP_DOWNLOAD_PATH_USER:
			if(SetDownPath(m_strDownPath)){
				if(!CheckFolder(m_strDownPath))
					m_strDownPath = "";
			}
			break;
		
		default:
			m_strDownPath = "";
	}

	return m_strDownPath;
}

BOOL CHttpFileDownload::SetFileURL(CString &strFileURL)
{
	if(strFileURL != ""){
		m_strFileURL = strFileURL;
		return TRUE;
	}
	else{
		m_strFileURL = strFileURL;
		return FALSE;
	}
}

void CHttpFileDownload::GetFileURL(CString &strFileURL)
{
	strFileURL = m_strFileURL;
}

BOOL CHttpFileDownload::SetDownFilePath(CString &strDownFilePath)
{
	if(strDownFilePath != ""){
		m_strDownFilePath = strDownFilePath;
		return TRUE;
	}
	else{
		m_strDownFilePath = strDownFilePath;
		return FALSE;
	}
}

void CHttpFileDownload::GetDownFilePath(CString &strDownFilePath)
{
	strDownFilePath = m_strDownFilePath;
}

CString CHttpFileDownload::GetDownFolder()
{
	CString retval = "";
	int nLength;
	int nCutPoint;

	nLength = m_strDownPath.GetLength();
	nLength -= 1;
	
	nCutPoint = m_strDownPath.ReverseFind('\\');
	
	if((nLength != 0) && (nCutPoint < nLength)){
		retval = m_strDownPath;
	}
	else if((nLength != 0) && (nCutPoint == nLength)){
		retval = m_strDownPath.Left(nCutPoint);
	}
	else{
		;
	}
	
	return retval;
}

void CHttpFileDownload::ResetInternet()
{
  //Free up the internet handles we may be using
  if (m_hHttpFile){
    ::InternetCloseHandle(m_hHttpFile);
    m_hHttpFile = NULL;
  }
  if (m_hHttpConnection){
    ::InternetCloseHandle(m_hHttpConnection);
    m_hHttpConnection = NULL;
  }
}

BOOL CHttpFileDownload::CreateCheck(CString strPath)
{
	if( !CreateDirectory(strPath, 0) ){
		int ret = GetLastError();
		if(ret == ERROR_ALREADY_EXISTS){
			return TRUE;
		}
		return FALSE;
	}

	return TRUE;
}

void CHttpFileDownload::SetError(int nErr)
{
	if(m_pNotiInfo)
		m_pNotiInfo->nErrno = nErr;
}

bool CHttpFileDownload::IsFileExist(LPCSTR strFullPath)
{
	WIN32_FIND_DATA FileData;
	HANDLE hSearch = FindFirstFile(strFullPath, &FileData);
	if(hSearch == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	FindClose(hSearch);

	return true;
}

bool CHttpFileDownload::SetWriteRunOnceAndReboot(CString strSource, CString strDest)
{
	bool retval = false;
	CString strRunOnceFilePath = GetRunOnceFilePath();
	if(!strRunOnceFilePath.IsEmpty())
	{
		CRegKey regkey;
		if(regkey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", KEY_WRITE) == ERROR_SUCCESS )
		{
			CString strLine;
			strLine.Format("\"%s\" /cs %s /cd %s", strRunOnceFilePath, strSource, strDest);
			regkey.SetValue(strLine, "Updater runonce copy");
			strLine.Format("\"%s\" /ds %s", strRunOnceFilePath, strSource);
			regkey.SetValue(strLine, "Updater runonce delete");
			regkey.Close();
			retval = true;
		}
	}

	return retval;
}

CString CHttpFileDownload::GetRunOnceFilePath()
{
	CString rString("");
	LPTSTR	lpszSystemInfo;     // pointer to system information string 
	TCHAR	tchBuffer[MAX_PATH];   // buffer for expanded string 
	lpszSystemInfo = tchBuffer; 
	if(0==::GetSystemDirectory(lpszSystemInfo, MAX_PATH+1))
		return rString;
	else
		rString = (LPCTSTR)lpszSystemInfo;

	rString+= "\\RunOnce.exe";
	if(!IsFileExist(rString))
		rString = "";

	return rString;
}
#ifdef _URLDOWN
CUrlDownload::CUrlDownload(PUPDATE_TRANSNOTIFY pNotiInfo, BOOL* pCancelDownload)
: m_pNotiInfo(pNotiInfo)
, m_dwCurBytesRead(0)
, m_dwStartTicks(::GetTickCount())
, m_pbCancelDownload(pCancelDownload)
, m_strCacheFilePath(_T(""))
{
	
}

CUrlDownload::~CUrlDownload()
{

}

HRESULT CUrlDownload::OnStartBinding(DWORD dwReserved, IBinding *pib)
{
	return S_OK;
}

HRESULT CUrlDownload::GetPriority(LONG *pnPriority)
{
	return S_OK;
}

HRESULT CUrlDownload::OnLowResource(DWORD reserved)
{
	return S_OK;
}

HRESULT CUrlDownload::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
//	CString strDebug = szStatusText;
//	OutputDebugString(strDebug);

	if (m_pbCancelDownload && *m_pbCancelDownload)
		return E_ABORT;

	// 이전에 비해 얼마 정도의 용량을 다운로드 받았는지 체크
	m_dwCurBytesRead = ulProgress - m_dwCurBytesRead;

	if(m_pNotiInfo){
		m_pNotiInfo->nSendPos = m_dwCurBytesRead;
		m_pNotiInfo->nTotalPos += m_dwCurBytesRead;
		m_pNotiInfo->nRequiredTime = ::GetTickCount() - m_dwStartTicks;
		SendFileNotify(m_pNotiInfo);
	}

	// 다시 마지막 다운로드 받은 크기 저장
	m_dwCurBytesRead = ulProgress;
	if (ulStatusCode == BINDSTATUS_CACHEFILENAMEAVAILABLE)
		m_strCacheFilePath = szStatusText;

	return S_OK;
}

HRESULT CUrlDownload::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
	return S_OK;
}

HRESULT CUrlDownload::GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo)
{
	return S_OK;
}

HRESULT CUrlDownload::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
	return S_OK;
}

HRESULT CUrlDownload::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
	return S_OK;
}

HRESULT CUrlDownload::QueryInterface(REFIID riid, void ** ppvObject)
{
	return E_NOTIMPL;
}

ULONG CUrlDownload::AddRef(void)
{
	return 0;
}

ULONG CUrlDownload::Release(void)
{
	return 0;
}
#endif // _URLDOWN