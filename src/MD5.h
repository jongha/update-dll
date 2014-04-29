// MD5.h: interface for the MD5 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MD5_H__441441AB_3B97_47C1_A774_AFF0DBA48049__INCLUDED_)
#define AFX_MD5_H__441441AB_3B97_47C1_A774_AFF0DBA48049__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

class MD5
{
public:
	MD5();
	MD5(FILE *file);
	MD5(CString sSource);
	CString Calculate(FILE *file);
	CString Calculate(CString sSource);
	CString	CalcFileMD5(CString szFileName);
	CString GetHash();
	DWORD   GetLastFileError();
private:
	CString	m_sHash;
	DWORD   m_dwErr;
};

#endif // !defined(AFX_MD5_H__441441AB_3B97_47C1_A774_AFF0DBA48049__INCLUDED_)
