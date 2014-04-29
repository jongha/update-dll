// UpdateUtil.h: interface for the CUpdateUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPDATEUTIL_H__F253E318_DD9E_490F_99FB_6B0B726437D0__INCLUDED_)
#define AFX_UPDATEUTIL_H__F253E318_DD9E_490F_99FB_6B0B726437D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "shlwapi.h"
#include "shlobj.h"
#include "ItemList.h"
#pragma comment( lib, "version.lib" )

class CUpdateUtil  
{
public:
	CUpdateUtil();
	CUpdateUtil(BOOL *pbCancel);
	virtual ~CUpdateUtil();

public:
	BOOL	DownloadFileSizeCheck(CString &strSrcURL, DWORD &dwFileSize);

	BOOL	DownloadFileCheck(PUPDATE_TRANSNOTIFY pNotiInfo, CString &strSrcURL);
	BOOL	DownloadFileCheckEx(PUPDATE_TRANSNOTIFY pNotiInfo, CString &strSrcURL, CString strLoginId, CString strLoginPassword);
	// notify function
	BOOL	Download(PUPDATE_TRANSNOTIFY pNotiInfo, CItemList* pItemListSoruce, CItemList* pItemListDest);
	BOOL	DownloadEx(PUPDATE_TRANSNOTIFY pNotiInfo, CItemList* pItemListSoruce, CItemList* pItemListDest, CString strLoginId, CString strLoginPassword);

	// general public function
	BOOL	DownloadXMLfile(CString& strSrcURL, CString& strXMLdoc, CString& strFilePath);
	BOOL	DownloadXMLfileEx(CString& strSrcURL, CString& strXMLdoc, CString& strFilePath, CString& strLoginId, CString& strLoginPassword);
	BOOL	OpenXML(CString& strSourcePath, CString& strXMLdoc);
	BOOL	ReadXML(CString& strXMLdoc, CItemList* pSourceItemList, int nKind = UPDATE_KIND_SOURCE);
	BOOL	CompareFileVersion(CItemList* pItemListSourceCmp, CItemList* pItemListDestCmp);
	int		CheckFile(CString strPath);
	BOOL	DeleteFile(CString& strFilePath);

protected:
	BOOL	CompareFileVersion2(CItemList* pItemListSourceCmp, CItemList* pItemListDestCmp);
	BOOL	LastCheckNewFile(CItemList* pItemListSourceCmp);
	// notify function
	BOOL	DownloadFile(PUPDATE_TRANSNOTIFY pNotiInfo, CString& strSrcURL, CString& strDownFolderPath, 
					CString strMD5);
	BOOL	DownloadFileEx(PUPDATE_TRANSNOTIFY pNotiInfo, CString& strSrcURL, CString& strDownFolderPath, 
					CString strLoginId, CString strLoginPassword, CString strMD5);
	// general function
	BOOL	IsFileExist(CString& strDestFile, unsigned long& ulClientSize);
	BOOL	MakeClientXMLfile(CItemList *pItemListSoruce, LPCSTR lpszDestFilePath);
	BOOL	CheckFolder(CString	strPath);

	// folder function
	int		GetFolder(CString strReplace);
	CString GetFolderPath(LPCSTR szPath);
	bool	GetSpecialDirectory(int nFolder, CString &strPath);
	bool	GetShSpecialDirectory(int nFolder, CString &strPath);
	bool	GetCurrentDirectory(CString& strPath);
	int		GetFolderSymbol(CString& strPath, CString& strReplace);
	int		CreateCheck(CString strPath);

	// regsvr function
	BOOL	RegSvr32(CString &strFilePath);
	bool	RegisterOLE(CString pszDllName, CString pszDllEntryPoint);
	bool	Error(HRESULT hRes);
	
	// version function
	BOOL	GetFileVersion(LPTSTR lpDestFile, LPTSTR VersionStr);
	void	StringTokenize(CString& strSource, CUIntArray &UIntDestArray);
	int		VersionCheck(LPCSTR lpszFileName, LPCSTR lpszClientVersion,  LPCSTR lpszServerVersion, LPCSTR lpszServerAddress,
						LPCSTR lpszServerMD5, unsigned long ulServerSize);
	int		CString2Integer(CString string);
	int		RealVersionCheck(CUIntArray& unarrayClient, CUIntArray& unarrayServer);
	// member variables
	BOOL	*m_pbCancel;

};

#endif // !defined(AFX_UPDATEUTIL_H__F253E318_DD9E_490F_99FB_6B0B726437D0__INCLUDED_)
