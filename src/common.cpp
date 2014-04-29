#include "stdafx.h"

DWORD GetLongPathNameEx(CString strShortPath, CString& strLongPath)
{
	int iFound = strShortPath.ReverseFind('\\');
	if (iFound > 1)
	{
		// recurse to peel off components
		//
		if (GetLongPathNameEx(strShortPath.Left(iFound),
			strLongPath) > 0)
		{
			strLongPath += _T('\\');
			
			if (strShortPath.Right(1) != _T("\\"))
			{
				WIN32_FIND_DATA findData;
				
				// append the long component name to the path
				//
				if (INVALID_HANDLE_VALUE != ::FindFirstFile
					(strShortPath, &findData))
				{
					strLongPath += findData.cFileName;
				}
				else
				{
					// if FindFirstFile fails, return the error code
					//
					strLongPath.Empty();
					return 0;
				}
			}
		}
	}
	else
	{
		strLongPath = strShortPath;
	}
	
	return strLongPath.GetLength();
}