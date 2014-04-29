#ifndef UPDATE_DLLEXPORT  
#define UPDATE_DLLEXPORT extern "C" __declspec(dllexport) 
#endif

#define UPDATEDLL "UpdateDLL"

//success
#define UPDATE_SUCCESS							1
//error
#define UPDATE_ERROR_UNKNOWN					-1
#define UPDATE_ERROR_NONE						1
#define UPDATE_ERROR_INFORMATION				2
#define UPDATE_ERROR_FAIL_DOWNLOADXMLFILE		3
#define UPDATE_ERROR_FILE_NOT_FOUND				4

#define UPDATE_ERROR_CANT_MAKE_ITEMLIST_SOURCE	5
#define UPDATE_ERROR_CANT_MAKE_ITEMLIST_DEST	6

#define UPDATE_ERROR_CANT_READ_XML_SOURCE		7
#define UPDATE_ERROR_CANT_READ_XML_DEST			8

#define UPDATE_ERROR_CANT_OPEN_XML_FILE			9
#define UPDATE_ERROR_CANT_MAKE_XML_FILE			10

#define UPDATE_ERROR_CANT_MAKE_DOWNLOAD_THREAD	11

#define UPDATE_ERROR_NONE_NOT_EXECUTE			12
#define UPDATE_ERROR_NONE_NOT_EXECUTE_AND_REBOOT	13

#define UPDATE_SUCCESS_REGSVR_DLL				100
#define UPDATE_ERROR_DLL_LOAD_LIBRARY			101
#define UPDATE_ERROR_DLL_GET_PROC_ADDRESS		102

#define UPDATE_HTTP_FAIL_PARSE_URL 						301
#define UPDATE_HTTP_FAIL_INTERNET_SESSION				302
#define UPDATE_HTTP_FAIL_INTERNET_CONNECTION			303
#define UPDATE_HTTP_FAIL_INTERNET_OPEN_REQUEST			304
#define UPDATE_HTTP_FAIL_INTERNET_SEND_REQUEST 			305
#define UPDATE_HTTP_FAIL_INTERNET_QUERY_STATUS_CODE		306
#define UPDATE_HTTP_FAIL_INTERNET_QUERY_CONTENT_LENGTH 	307
#define UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE			308
#define UPDATE_HTTP_FAIL_INTERNET_READ_FILE				309
#define UPDATE_HTTP_FAIL_INTERNET_STATUS_DENIED			310
#define UPDATE_HTTP_FAIL_INTERNET_STATUS_NOT_FOUND		311
#define UPDATE_HTTP_FAIL_INTERNET_OTHER_FAIL			312
#define UPDATE_HTTP_FAIL_PARSE_DOWN_PATH 				313
#define UPDATE_HTTP_FAIL_CANCEL			 				314
#define UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE_HIDE		315
#define UPDATE_HTTP_FAIL_INTERNET_CREATE_FILE_REBOOT	316


#define UPDATE_FILECHECK_RESULT_EXIST			601
#define UPDATE_FILECHECK_RESULT_CREATE			602
#define UPDATE_FILECHECK_RESULT_ERROR			603

#define UPDATE_OPTION_FLAG_RENAME				1

#define UPDATE_RENAME_EXTENTION_NAME			".UpdateDLL_old"
#define UPDATE_REBOOT_EXTENTION_NAME			".UpdateDLL_new"

typedef struct _UPDATE_DATA
{
	char	Name[MAX_PATH];
	char	Type[MAX_PATH];
	char	Version[MAX_PATH];
	char	DownURL[MAX_PATH];
	char	DownPath[MAX_PATH];
	char	Description[MAX_PATH];
	char	Force[MAX_PATH];
	char	RegSrv[MAX_PATH];
	char	CheckSum[MAX_PATH];
	int		UpdateFlag;
	int		OptionFlag;
	unsigned long	Size;

} UPDATE_DATA, *PUPDATE_DATA;

typedef void (*UPDATE_TRANS_NOTIFYFUNC)	(ULONG);

typedef struct _UPDATE_TRANSINFO
{
	const char*		lpszSrcURL;
	const char*		lpszDestXmlFile;
//#ifdef _V3
	int				bNewCreateFile;
	const char*		lpszFileType;
//#endif // _V3
	HWND			hNotifyWnd;
	UINT			nNotifyMsg;
	UINT			nRebootAskMsg;
	int				nRebootAskResult;

	UPDATE_TRANS_NOTIFYFUNC	pNotifyFunc;
	int				nErrno;
	ULONG			lParam;

	const char*		lpszURLLoginId;
	const char*		lpszURLLoginPassword;	
} UPDATE_TRANSINFO, *PUPDATE_TRANSINFO;

typedef struct _UPDATE_TRANSNOTIFY
{
	PUPDATE_TRANSINFO	pInfo;
	
	unsigned long		nTotalPos;
	unsigned long		nSendPos;
	unsigned long		nRequiredTime;
	int					nErrno;
	
	int					nTotalCount;
	int					nCurrentCount;

	unsigned long		nCurrentSize;
	unsigned long		nTotalSize;

	char*				lpszName;
	char*				lpszVersion;
	char*				lpszDownURL;
	char*				lpszDownPath;
	char*				lpszDescription;
	
	ULONG				lParam;

} UPDATE_TRANSNOTIFY, *PUPDATE_TRANSNOTIFY;

///*
static inline void SendFileNotify(PUPDATE_TRANSNOTIFY pNotiInfo)
{
#ifdef _DEBUG
	static UINT nSendCount = 0;
#endif
	if(pNotiInfo->pInfo)
	{
		if(pNotiInfo->pInfo->hNotifyWnd && pNotiInfo->pInfo->nNotifyMsg)
			SendMessage(pNotiInfo->pInfo->hNotifyWnd, pNotiInfo->pInfo->nNotifyMsg, 0, (LPARAM)pNotiInfo);
#ifdef _DEBUG
		TRACE("Send Message Count : %ld\n", nSendCount++);
#endif
		if(pNotiInfo->pInfo->pNotifyFunc)
			pNotiInfo->pInfo->pNotifyFunc((LPARAM)pNotiInfo);
	}
}

static inline void SendRebootAskNotify(PUPDATE_TRANSNOTIFY pNotiInfo)
{
#ifdef _DEBUG
	static UINT nSendCount = 0;
#endif
	if(pNotiInfo->pInfo)
	{
		if(pNotiInfo->pInfo->hNotifyWnd && pNotiInfo->pInfo->nRebootAskMsg)
			SendMessage(pNotiInfo->pInfo->hNotifyWnd, pNotiInfo->pInfo->nRebootAskMsg, 0, (LPARAM)pNotiInfo);
#ifdef _DEBUG
		TRACE("Send Message Count : %ld\n", nSendCount++);
#endif
	}
}

//*/
UPDATE_DLLEXPORT 
BOOL WINAPI StartUpdate(PUPDATE_TRANSINFO pInfo, BOOL *pbCancel);

static char _szDllRegSvr[] = "DllRegisterServer"; 
static char _szDllUnregSvr[] = "DllUnregisterServer";

#define PTR_ARRAY_REMOVE_ALL(CLASS_NAME,pPTR_ARRAY)					\
	while((pPTR_ARRAY)->GetSize()){									\
		CLASS_NAME* pXXXItem=(CLASS_NAME*)(pPTR_ARRAY)->GetAt(0);	\
		(pPTR_ARRAY)->RemoveAt(0);									\
		if (pXXXItem){												\
			delete pXXXItem;										\
			pXXXItem=NULL;											\
		}															\
	}

#define PTR_REMOVE(pPTR)	\
	if(pPTR){				\
		delete pPTR;		\
		pPTR = NULL;		\
	}

#define CANCEL()			\
	if(pbCancel){			\
		if((BOOL)(*pbCancel)){	\
			return FALSE;	\
		}					\
	}

#define CANCEL_VOID()		\
	if(pbCancel){			\
		if(*pbCancel){		\
			return;			\
		}					\
	}

#define CANCEL_B()			\
	if(pbCancel){			\
		if(*pbCancel){		\
			return false;	\
		}					\
	}

#define CString2pchar(str) ((LPSTR)(LPCSTR)(LPCTSTR)str)

#define DATA_INIT_AND_DEPOSIT()						\
	if(strData != ""){								\
		strData = "";								\
		strData = xml.GetChildData();				\
	}else{											\
		strData = xml.GetChildData();				\
	}

#define DATA_INIT_AND_DEPOSIT_NOCASE()				\
	if(strData != ""){								\
		strData = "";								\
		strData = xml.GetChildData();				\
		strData.MakeLower();						\
	}else{											\
		strData = xml.GetChildData();				\
		strData.MakeLower();						\
	}

#define STRCPY(data)								\
	strcpy(pFileData->data, (LPCSTR)strData);

#define SET_ITEM_DATA(srcch, destch, belse)			\
	if(xml.FindChildElem(_T(#srcch))){				\
		strData = "";								\
		strData = xml.GetChildData();				\
		pFileData->destch = CString2pchar(strData);	\
	}else{											\
		if(#belse)									\
			return FALSE;							\
	}


DWORD GetLongPathNameEx(CString strShortPath, CString& strLongPath);
