/**
 * @file magicfolderwiz.h
 * @brief Wizard to create magic folders
 * @author Simon Steele
 * @note Copyright (c) 2004 Simon Steele <s.steele@pnotepad.org>
 *
 * Programmers Notepad 2 : The license file (license.[txt|html]) describes 
 * the conditions under which this source may be modified / distributed.
 */
#ifndef magicfolderwiz_h__included
#define magicfolderwiz_h__included

class CBrowseTree;

class MagicFolderWizard1 : public CPropertyPageImpl<MagicFolderWizard1>
{
	typedef CPropertyPageImpl<MagicFolderWizard1> baseClass;
	friend class baseClass;
public:
	MagicFolderWizard1();
	~MagicFolderWizard1();

	enum { IDD = IDD_MAGICFOLDERWIZ1 };

	BEGIN_MSG_MAP(MagicFolderWizard1)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//NOTIFY_HANDLER(IDC_SHELLTREE, TVN_SELCHANGED, OnSelChanged)
        CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	//BOOL OnApply();
	int OnSetActive();

protected:
	CBrowseTree* shelltree;
};

#endif