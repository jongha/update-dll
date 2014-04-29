// HttpFileCheck.cpp: implementation of the CHttpFileCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UpdateDLL.h"
#include "HttpFileCheck.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHttpFileCheck::CHttpFileCheck(CString strFileURL, CString strLoginId, CString strLoginPassword)
{
	m_hInternetSession = NULL;
	m_hHttpConnection = NULL;
	m_hHttpFile = NULL;
	m_pNotiInfo = NULL;
	m_dwFileSize = 0;
	m_strFileURL = strFileURL;
	
	m_strLoginId = strLoginId;
	m_strLoginPassword = strLoginPassword;

}

CHttpFileCheck::CHttpFileCheck(CString strFileURL, PUPDATE_TRANSNOTIFY pNotiInfo, CString strLoginId, CString strLoginPassword)
{
	m_hInternetSession = NULL;
	m_hHttpConnection = NULL;
	m_hHttpFile = NULL;
	m_pNotiInfo = pNotiInfo;
	m_dwFileSize = 0;
	m_strFileURL = strFileURL;

	m_strLoginId = strLoginId;
	m_strLoginPassword = strLoginPassword;
}

CHttpFileCheck::~CHttpFileCheck()
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

}

BOOL CHttpFileCheck::ParseURL()
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


BOOL CHttpFileCheck::DownFileCheck()
{
	CString strFile;

	strFile = m_strObject;

	//Create the Internet session handle
	m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);    

	if (m_hInternetSession == NULL){
		SetError(UPDATE_HTTP_FAIL_INTERNET_SESSION);
		return FALSE;
	}

	//Make the connection to the HTTP server          
	if(m_strLoginId != "" && m_strLoginPassword != ""){
		m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_strServer, INTERNET_INVALID_PORT_NUMBER, m_strLoginId, 
											  m_strLoginPassword, INTERNET_SERVICE_HTTP, 0, (DWORD) this);
	}
	else{
		m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_strServer, INTERNET_INVALID_PORT_NUMBER, NULL, 
											  NULL, INTERNET_SERVICE_HTTP, 0, (DWORD) this);
	}


	if (m_hHttpConnection == NULL){
		SetError(UPDATE_HTTP_FAIL_INTERNET_CONNECTION);
		return FALSE;
	}

	//Set any CInternetSession options we  may need
	int ntimeOut = 30;
	::InternetSetOption(m_hInternetSession, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)(1000* ntimeOut),0);
	::InternetSetOption(m_hInternetSession, INTERNET_OPTION_CONNECT_BACKOFF,(void*)(1000), 0);
	::InternetSetOption(m_hInternetSession, INTERNET_OPTION_CONNECT_RETRIES,(void*)(1),0);


	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  
	ppszAcceptTypes[1] = NULL;
	DWORD dwFlags =INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION;

	m_hHttpFile = ::HttpOpenRequest(m_hHttpConnection, NULL, strFile, _T("HTTP/1.1"), NULL, ppszAcceptTypes, dwFlags, (DWORD) this);
	if (m_hHttpFile == NULL){
		SetError(UPDATE_HTTP_FAIL_INTERNET_OPEN_REQUEST);
		return FALSE;
	}

	BOOL btmpSend = ::HttpSendRequest(m_hHttpFile, NULL, 0, NULL, 0);
	if (!btmpSend){
		TRACE(_T("Failed in call to HttpSendRequest, Error:%d\n"), ::GetLastError());
		SetError(UPDATE_HTTP_FAIL_INTERNET_SEND_REQUEST);
		return FALSE;
	}

	//Check the HTTP status code
	TCHAR sztmpStatusCode[32];
	DWORD dwtmpInfoSize = 32;
	if (!::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_STATUS_CODE, sztmpStatusCode, &dwtmpInfoSize, NULL)){
		TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
		SetError(UPDATE_HTTP_FAIL_INTERNET_QUERY_STATUS_CODE);
		return FALSE;
	}
	else{
		//Get the HTTP status code
		long nStatusCode = _ttol(sztmpStatusCode);

		//Handle any authentication errors
		if (nStatusCode == HTTP_STATUS_OK){
			// Get the length of the file.            
			TCHAR szContentLength[32];
			dwtmpInfoSize = 32;
			if (::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwtmpInfoSize, NULL)){
				//Set the progress control range
				m_dwFileSize = (DWORD) _ttol(szContentLength);
			}
			else{
				SetError(UPDATE_HTTP_FAIL_INTERNET_QUERY_CONTENT_LENGTH);
				return FALSE;
			}
		}
		//Handle any errors
		else if (nStatusCode == HTTP_STATUS_DENIED){
			SetError(UPDATE_HTTP_FAIL_INTERNET_STATUS_DENIED);
			return FALSE;
		}
		else if (nStatusCode == HTTP_STATUS_NOT_FOUND){
			SetError(UPDATE_HTTP_FAIL_INTERNET_STATUS_NOT_FOUND);
			return FALSE;
		}
		else{
			SetError(UPDATE_HTTP_FAIL_INTERNET_OTHER_FAIL);
			return FALSE;
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
	
	return TRUE;
}

BOOL CHttpFileCheck::Start()
{
	if(ParseURL()){
		if(DownFileCheck()){
			if(m_pNotiInfo)
				m_pNotiInfo->nCurrentSize = m_dwFileSize;
			return TRUE;
		}
		else{
			// nErrono already check DownFileCheck()
			return FALSE;			
		}
	}
	else{
		SetError(UPDATE_HTTP_FAIL_PARSE_URL);
	}
	
	return FALSE;
}

void CHttpFileCheck::SetError(int nErr)
{
	if(m_pNotiInfo)
		m_pNotiInfo->nErrno = nErr;
}

DWORD CHttpFileCheck::GetFileSize()
{
	return m_dwFileSize;
}

void CHttpFileCheck::SetFileURL(CString strFileURL)
{
	m_strFileURL = strFileURL;
}