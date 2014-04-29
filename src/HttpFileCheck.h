// HttpFileCheck.h: interface for the CHttpFileCheck class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPFILECHECK_H__A5E83664_B26B_4F69_9B36_B19CA76B5CB7__INCLUDED_)
#define AFX_HTTPFILECHECK_H__A5E83664_B26B_4F69_9B36_B19CA76B5CB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment( lib, "Wininet.lib" )
#include <wininet.h>
#include <afxinet.h>
#include <direct.h>
#include "ItemList.h"

class CHttpFileCheck  
{
public:
	CHttpFileCheck(CString strFileURL, CString strLoginId = "", CString strLoginPassword = "");
	CHttpFileCheck(CString strFileURL, PUPDATE_TRANSNOTIFY pNotiInfo, CString strLoginId = "", CString strLoginPassword = "");
	virtual ~CHttpFileCheck();

public:
	void	SetFileURL(CString strFileURL);
	DWORD	GetFileSize();
	BOOL	Start();
	BOOL	DownFileCheck();

protected:
	BOOL	ParseURL();
	void	SetError(int nErr);

protected:
	PUPDATE_TRANSNOTIFY m_pNotiInfo;
	DWORD	m_dwFileSize;

	HINTERNET		m_hInternetSession;		// InternetSession Handle
	HINTERNET		m_hHttpConnection;		// HttpConnection Handle
	HINTERNET		m_hHttpFile;			// Http File Handle

	CString			m_strLoginId;
	CString			m_strLoginPassword;

	CString			m_strFileURL;	// File Address and Path
	CString			m_strServer;	// Server URL
	CString			m_strObject;	// Object URL
	CString			m_strFileName;	// File Name
	INTERNET_PORT	m_nPort;		// Port number
};

#endif // !defined(AFX_HTTPFILECHECK_H__A5E83664_B26B_4F69_9B36_B19CA76B5CB7__INCLUDED_)
