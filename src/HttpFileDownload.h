// HttpFileDownload.h: interface for the CHttpFileDownload class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPFILEDOWNLOAD_H__456348D3_1158_4E1D_810A_0AD90232E1EB__INCLUDED_)
#define AFX_HTTPFILEDOWNLOAD_H__456348D3_1158_4E1D_810A_0AD90232E1EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma comment( lib, "Wininet.lib" )
#include <wininet.h>
#include <afxinet.h>
#include <direct.h>
#include "ItemList.h"

#define HTTP_DOWNLOAD_PATH_DEFAULT  100
#define HTTP_DOWNLOAD_PATH_EXECUTE	100
#define HTTP_DOWNLOAD_PATH_TEMP		200
#define HTTP_DOWNLOAD_PATH_USER		300

class CHttpFileDownload  
{
public:
	void	ResetInternet();
	CString GetDownFolder();
	CHttpFileDownload();
	CHttpFileDownload(CString strFileURL, int nDownMethod = HTTP_DOWNLOAD_PATH_DEFAULT, CString strLoginId = "", CString strLoginPassword = "");
	CHttpFileDownload(CString strFileURL, CString strDownPath, int nDownMethod = HTTP_DOWNLOAD_PATH_USER, BOOL *pbCancel = NULL, 
						PUPDATE_TRANSNOTIFY pNotiInfo = NULL, CString strLoginId = "", CString strLoginPassword = "");
	virtual ~CHttpFileDownload();

	BOOL Start();
	void GetFileURL(CString &strFileURL);
	BOOL SetFileURL(CString &strFileURL);
	void GetDownFilePath(CString &strDownFilePath);
	BOOL SetDownFilePath(CString &strDownFilePath);

public:
	void		SetError(int nErr);

//functions
protected:
	CString		GetDownPath();
	BOOL		SetDownPath(CString strDownPath);
	BOOL		ParseURL();
	short		Download();
	CString		GetTempPath();
	CString		GetExecutePath();
	BOOL		CheckFolder(CString	strPath);
	BOOL		CreateCheck(CString strPath);
	bool		IsFileExist(LPCSTR strFullPath);
	bool		SetWriteRunOnceAndReboot(CString strSource, CString strDest);
	CString		GetRunOnceFilePath();
	// variables	
#ifdef _URLDOWN
public:
	HANDLE			m_hDownThread;
#else // _URLDOWN
private:
#endif // _URLDOWN
	BOOL			*m_pbCancel;
	PUPDATE_TRANSNOTIFY m_pNotiInfo;

	CString			m_strFileURL;		// File Address and Path ex)
	CString			m_strDownFilePath;  
private:	
	int				m_nDownMethod;

	HINTERNET		m_hInternetSession;		// InternetSession Handle
	HINTERNET		m_hHttpConnection;		// HttpConnection Handle
	HINTERNET		m_hHttpFile;			// Http File Handle
	
	CString			m_strLoginId;
	CString			m_strLoginPassword;

	CString			m_strServer;	// Server URL
	CString			m_strObject;	// Object URL
	CString			m_strFileName;	// File Name
	INTERNET_PORT	m_nPort;		// Port number

	// Folder information
	CString			m_strExecutePath;		// Current Application execute folder
	CString			m_strDownPath;			// file folder
	CString			m_strTempPath;			// temporary directory
};

#ifdef _URLDOWN
//extern DWORD WINAPI DownThreadProc(LPVOID lpParam);
class CUrlDownload : public IBindStatusCallback  
{
public:
	CUrlDownload(PUPDATE_TRANSNOTIFY pNotiInfo, BOOL* pbCancelDownload);
	virtual ~CUrlDownload();
protected:
	PUPDATE_TRANSNOTIFY m_pNotiInfo;
	DWORD m_dwCurBytesRead;
	DWORD m_dwStartTicks;
	BOOL* m_pbCancelDownload;
public:
	CString m_strCacheFilePath;

/* IBindStatusCallback Interfaces */
public:
    HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding *pib);
    HRESULT STDMETHODCALLTYPE GetPriority(LONG *pnPriority);
    HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved);
    HRESULT STDMETHODCALLTYPE OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);
    HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError);
    HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo);
    HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed);
    HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown *punk);

//IUnknown Interfaces
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
	ULONG	STDMETHODCALLTYPE AddRef(void);
	ULONG	STDMETHODCALLTYPE Release(void);
};
#endif // _URLDOWN

#endif // !defined(AFX_HTTPFILEDOWNLOAD_H__456348D3_1158_4E1D_810A_0AD90232E1EB__INCLUDED_)
