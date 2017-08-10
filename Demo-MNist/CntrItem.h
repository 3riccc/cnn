// CntrItem.h : interface of the CMNistCntrItem class
//

#if !defined(AFX_CNTRITEM_H__EA7C3AB5_7431_4923_8266_581BCE9A43AE__INCLUDED_)
#define AFX_CNTRITEM_H__EA7C3AB5_7431_4923_8266_581BCE9A43AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMNistDoc;
class CMNistView;

class CMNistCntrItem : public COleClientItem
{
	DECLARE_SERIAL(CMNistCntrItem)

// Constructors
public:
	CMNistCntrItem(CMNistDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CMNistDoc* GetDocument()
		{ return (CMNistDoc*)COleClientItem::GetDocument(); }
	CMNistView* GetActiveView()
		{ return (CMNistView*)COleClientItem::GetActiveView(); }

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMNistCntrItem)
	public:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	virtual void OnActivate();
	protected:
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
	//}}AFX_VIRTUAL

// Implementation
public:
	~CMNistCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CNTRITEM_H__EA7C3AB5_7431_4923_8266_581BCE9A43AE__INCLUDED_)
