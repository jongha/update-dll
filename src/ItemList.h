// Item.h: interface for the CItemList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMLIST_H__069D836C_212F_4145_99BF_3D6CD9AEA555__INCLUDED_)
#define AFX_ITEMLIST_H__069D836C_212F_4145_99BF_3D6CD9AEA555__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UPDATE_KIND_SOURCE	1
#define UPDATE_KIND_DEST	2

class CItemList  
{
public:
	CItemList();
	CItemList(HWND pParentWnd, int nKind = UPDATE_KIND_SOURCE);
	virtual ~CItemList();

protected:
	CPtrList	m_ItemList;
	HWND		m_pParentWnd;
	int			m_nKind;

public:
	CPtrList* GetPtrList()					{ return &m_ItemList; };

	void	SetParentWnd(HWND pParentWnd)	{ m_pParentWnd = pParentWnd; };
	HWND	GetParentWnd() 					{ return m_pParentWnd; };
	int		GetKind()						{ return m_nKind; };
	int		SetKind(int nKind)				{ m_nKind = nKind; };
	
	int		GetItemCount();
	void*	GetNewItem();
	
	void	DeleteAll();
	bool	DeleteItem(void *pItemSet);
};

#endif // !defined(AFX_ITEMLIST_H__069D836C_212F_4145_99BF_3D6CD9AEA555__INCLUDED_)
