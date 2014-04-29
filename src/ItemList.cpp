// ItemAgent.cpp: implementation of the CItemList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UpdateDLL.h"
#include "ItemList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemList::CItemList()
{
	 m_pParentWnd = NULL;
}

CItemList::CItemList(HWND pParentWnd, int nKind)
{
	m_pParentWnd = pParentWnd;
	m_nKind = nKind;
}

CItemList::~CItemList()
{
	DeleteAll();
}

void CItemList::DeleteAll()
{
	POSITION pos = m_ItemList.GetHeadPosition();
	while(pos)
	{		
		UPDATE_DATA *psi = (UPDATE_DATA *)m_ItemList.GetNext(pos);
		DeleteItem((void*)psi);
	}
	m_ItemList.RemoveAll();
}

bool CItemList::DeleteItem(void *pItem)
{
	UPDATE_DATA *psi = (UPDATE_DATA*)pItem;
	if(!psi)
		return false;
	
	PTR_REMOVE(psi)
	
	return true;
}

void* CItemList::GetNewItem()
{
	UPDATE_DATA *pItem = new UPDATE_DATA;
	
	m_ItemList.AddTail((void*)pItem);

	return (void*)pItem;
}

int CItemList::GetItemCount()
{
	int retval = 0;
	retval = m_ItemList.GetCount();
	return retval;
}
